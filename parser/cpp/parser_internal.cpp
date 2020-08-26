#include "parser_internal.hpp"
#include "abi/guid.hpp"
#include "singleton.hpp"
#include "plugin_manager.hpp"
#include "plugin_interface.hpp"
#include "vision_service.hpp"
#include "message_protocol.hpp"

#include <mutex>
#include <utility>
#include <fstream>
#include <unordered_map>

#include <filesystem.hpp>
#include <os_context.hpp>

namespace glasssix::exposing::nessus
{
	namespace
	{
		std::vector<std::string> split(std::string_view s, std::string_view seperator)
		{
			std::vector<std::string> result;
			typedef std::string::size_type string_size;
			string_size i = 0;

			while (i != s.size())
			{
				int flag = 0;
				while (i != s.size() && flag == 0)
				{
					flag = 1;
					for (string_size x = 0; x < seperator.size(); ++x)
						if (s[i] == seperator[x])
						{
							++i;
							flag = 0;
							break;
						}
				}

				flag = 0;
				string_size j = i;
				while (j != s.size() && flag == 0)
				{
					for (string_size x = 0; x < seperator.size(); ++x)
						if (s[j] == seperator[x])
						{
							flag = 1;
							break;
						}
					if (flag == 0)
						++j;
				}
				if (i != j)
				{
					result.push_back(std::string(s.substr(i, j - i)));
					i = j;
				}
			}
			return result;
		}
	}

	class parser_internal::impl
	{
	public:
		impl()
		{
			protocol_map["Longinus.new"] = &Longinus_new_json;
			protocol_map["Longinus.delete"] = &Longinus_delete_json;
			protocol_map["Longinus.detect"] = &Longinus_detect_json;
			protocol_map["Longinus.alignFace"] = &Romancia_alignFace_json;
			protocol_map["Gaius.new"] = &Gaius_new_json;
			protocol_map["Gaius.delete"] = &Gaius_delete_json;
			protocol_map["Gaius.Forward"] = &Gaius_Forward_json;
			protocol_map["Cassius.new"] = &Cassius_new_json;
			protocol_map["Cassius.delete"] = &Cassius_delete_json;
			protocol_map["Cassius.Forward"] = &Cassius_Forward_json;
			protocol_map["Irisviel.new"] = &Irisviel_new_json;
			protocol_map["Irisviel.delete"] = &Irisviel_delete_json;
			protocol_map["Irisviel.search"] = &Irisviel_search_json;
			protocol_map["Irisviel.clear"] = &Irisviel_clear_json;
			protocol_map["Irisviel.remove_all"] = &Irisviel_remove_all_json;
			protocol_map["Irisviel.load_databases"] = &Irisviel_load_databases_json;
			protocol_map["Irisviel.remove_records"] = &Irisviel_remove_records_json;
			protocol_map["Irisviel.remove_record"] = &Irisviel_remove_record_json;
			protocol_map["Irisviel.add_record"] = &Irisviel_add_record_json;
			protocol_map["Irisviel.add_records"] = &Irisviel_add_records_json;
			protocol_map["Irisviel.update_record"] = &Irisviel_update_record_json;
			protocol_map["Irisviel.update_records"] = &Irisviel_update_records_json;

			ready = false;
		}

		std::string parse(std::string_view protocol, std::string_view jsonstr)
		{
			Json::Value value;

			if (!ready)
			{
				value["status"] = Json::Value("parser init plugin exception");
				return writer.write(value);
			}

			simdjson::dom::element root;
			try
			{
				std::lock_guard<std::mutex> lck(mut_parse);
				root = parser_.parse(jsonstr);
			}
			catch (const std::exception&)
			{
				value["status"] = Json::Value("parse json error");
				return writer.write(value);
			}

			std::vector<std::string> str_vec = split(protocol, ".");
			if (str_vec.size() != 2)
			{
				value["status"] = Json::Value("topic illegal");
				return writer.write(value);
			}

			std::string protocol_str{ protocol };
			std::string instance_type = str_vec[0];
			std::string method = str_vec[1];

			if (method == "new")
			{
				guid instance;
				std::function<Json::Value(plugin_interface&, simdjson::dom::element&, guid&)> func;
				try
				{
					func = protocol_map.at(protocol_str);
				}
				catch (const std::exception&)
				{
					value["status"] = Json::Value("Function of the topic not register");
					return writer.write(value);
				}

				value = func(plugin, root, instance);

				if (value["status"] == "OK")
				{
					auto array = to_char_array(instance);
					value["instance_guid"] = Json::Value(std::string(array.begin(), array.end()));
				}
			}
			else
			{
				std::string instance_guid = "";
				try
				{
					instance_guid = std::string(root["instance_guid"].get<std::string_view>().value());
				}
				catch (const std::exception&)
				{
					value["status"] = Json::Value("Lack of \"instance_guid\"");
					return writer.write(value);
				}

				std::function<Json::Value(plugin_interface&, simdjson::dom::element&, guid&)> func;
				try
				{
					func = protocol_map.at(protocol_str);
				}
				catch (const std::exception&)
				{
					value["status"] = Json::Value("Function of the topic not register");
					return writer.write(value);
				}

				guid instance(instance_guid);
				value = func(plugin, root, instance);
			}

			return writer.write(value);
		}

		std::string query_all_instance()
		{
			
			Json::Value value;
			auto instances = plugin.as<vision_service>().existing_instances();
			for (const auto& instance : instances)
			{
				auto key_array = to_char_array(instance.key());
				value[std::string(key_array.begin(), key_array.end())] = to_narrow_string(instance.value());
			}

			return writer.write(value);
		}

		std::string support_protocol()
		{
			Json::Value value;
			Json::Value protocol_array = Json::Value(Json::arrayValue);
			for (auto& protocol : protocol_map)
				protocol_array.append(protocol.first);
			value["protocol"] = protocol_array;

			return writer.write(value);
		}

		std::string init_plugin(std::string_view config_file_path)
		{
			static std::once_flag flag;

			std::string status = "{\"status\":\"Function 'init_plugin' has beed called and could be called one time\"}";
			std::call_once(flag, [&]
				{
					std::ifstream f_config{ std::string(config_file_path) };
					std::string buffer(std::istreambuf_iterator<char>{ f_config }, std::istreambuf_iterator<char>{});

					try
					{
						simdjson::dom::element config = parser_.parse(buffer);
						fs::path plugin_directory = os_context::expand_enviroment_variables(config["plugin_directory"].get<std::string_view>().value());

						for (auto lib_item : config["plugin_list"].get<simdjson::dom::array>().value())
						{
							bool ret = get_component_loader().add_module(to_param_string((plugin_directory / lib_item.get<std::string_view>().value()).string()));
							if (!ret)
							{
								ready = false;
								status = "{\"status\":\"load module '" + std::string(lib_item.get<std::string_view>().value()) + "' failed\"}";
								return;
							}
						}

						auto manager = exposing::make_exported_interface<plugin_manager>();

						manager.load_from_file((plugin_directory / "libvision_serviced.dll").u8string().c_str());
						plugin = manager.lookup(u8"Glasssix Vision Service");
						if (!plugin)
						{
							ready = false;
							status = "{\"status\":\"Get a nullptr 'plugin_interface' instance\"}";
							return;
						}

						ready = true;
						status = "{\"status\":\"OK\"}";
					}
					catch (const std::exception& ex)
					{
						ready = false;
						status = std::string("{\"status\":\"") + ex.what() + std::string("\"}");
					}
					catch (const abi_error& ex)
					{
						ready = false;
						status = std::string("{\"status\":\"") + ex.what_to_narrow() + std::string("\"}");
					}
				});

			return status;
		}
	private:
		std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, simdjson::dom::element&, guid&)>> protocol_map;
		simdjson::dom::parser parser_;

		Json::FastWriter writer;
		plugin_interface plugin;

		std::mutex mut_parse;
		bool ready;
	};

	parser_internal::parser_internal() : impl_{ new impl }
	{
	}

	parser_internal::~parser_internal()
	{
		if (impl_)
		{
			delete impl_;
			impl_ = nullptr;
		}
	}

	std::string parser_internal::parse(std::string_view topic, std::string_view jsonstr)
	{
		return impl_->parse(topic, jsonstr);
	}

	std::string parser_internal::query_all_instance()
	{
		return impl_->query_all_instance();
	}

	std::string parser_internal::support_protocol()
	{
		return impl_->support_protocol();
	}

	std::string parser_internal::init_plugin(std::string_view config_file_path)
	{
		return impl_->init_plugin(config_file_path);
	}
}
