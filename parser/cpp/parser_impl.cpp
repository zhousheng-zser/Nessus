#include "parser_impl.hpp"
#include "simdjson.h"
#include "json.h"
#include "plugin_interface.hpp"
#include "vision_service.hpp"
#include "plugin_manager.hpp"

#include "message_protocol.hpp"

#include <filesystem.hpp>
#include <os_context.hpp>

#include <string_view>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <fstream>

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

	/// <summary>
	/// An implementation of the standard plugin manager.
	/// </summary>
	class parser_impl::impl
	{
	public:

		param_string parse(const param_string& protocol, const param_string& jsonstr)
		{
			Json::Value value;
			std::string_view protocol_view(protocol.data(), protocol.size());
			std::string_view jsonstr_view(jsonstr.data(), jsonstr.size());
			if (!ready)
			{
				value["status"] = Json::Value("parser init plugin exception");
				return to_param_string(writer.write(value));
			}

			simdjson::dom::element root;
			try
			{
				root = parser_.parse(jsonstr_view);
			}
			catch (const std::exception&)
			{
				value["status"] = Json::Value("parse json error");
				return to_param_string(writer.write(value));
			}

			std::vector<std::string> str_vec = split(protocol_view, ".");
			if (str_vec.size() != 2)
			{
				value["status"] = Json::Value("protocol illegal");
				return to_param_string(writer.write(value));
			}

			std::string protocol_str(protocol_view.begin(), protocol_view.end());
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
					value["status"] = Json::Value("Function of the protocol not register");
					return to_param_string(writer.write(value));
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
					return to_param_string(writer.write(value));
				}

				std::function<Json::Value(plugin_interface&, simdjson::dom::element&, guid&)> func;
				try
				{
					func = protocol_map.at(protocol_str);
				}
				catch (const std::exception&)
				{
					value["status"] = Json::Value("Function of the protocol not register");
					return to_param_string(writer.write(value));
				}

				guid instance(instance_guid);
				value = func(plugin, root, instance);
			}

			return to_param_string(writer.write(value));
		}

		param_string query_all_instance()
		{
			Json::Value value;
			auto instances = plugin.as<vision_service>().existing_instances();
			for (const auto& instance : instances)
			{
				auto key_array = to_char_array(instance.key());
				value[std::string(key_array.begin(), key_array.end())] = to_narrow_string(instance.value());
			}

			return to_param_string(writer.write(value));
		}

		param_string support_protocol()
		{
			Json::Value value;
			Json::Value protocol_array = Json::Value(Json::arrayValue);
			for (auto& protocol : protocol_map)
				protocol_array.append(protocol.first);
			value["protocol"] = protocol_array;

			return to_param_string(writer.write(value));
		}

		param_string init_plugin(const param_string& config_file_path)
		{
			static std::once_flag flag;

			std::string status = "{\"status\":\"Function 'init_plugin' has beed called and could be called one time\"}";
			std::call_once(flag, [&]
				{
					std::ifstream f_config{ std::string(config_file_path.begin(), config_file_path.end()) };
					std::string buffer(std::istreambuf_iterator<char>{ f_config }, std::istreambuf_iterator<char>{});

					try
					{
						simdjson::dom::parser parser_temp;
						simdjson::dom::element config = parser_temp.parse(buffer);
						//fs::path plugin_directory = os_context::expand_enviroment_variables(config["plugin_directory"].get<std::string_view>().value());

						for (auto lib_item : config["plugin_list"].get<simdjson::dom::array>().value())
						{
							//bool ret = get_component_loader().add_module(to_param_string((plugin_directory / lib_item.get<std::string_view>().value()).string()));
							bool ret = get_component_loader().add_module_by_name(to_param_string(lib_item.get<std::string_view>().value()));
							if (!ret)
							{
								ready = false;
								status = "{\"status\":\"load module '" + std::string(lib_item.get<std::string_view>().value()) + "' failed\"}";
								return;
							}
						}

						auto manager = exposing::make_exported_interface<plugin_manager>();

						manager.load_from_existing_libraries();
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

			return to_param_string(status);
		}

	private:
		static std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, simdjson::dom::element&, guid&)>> protocol_map;
		simdjson::dom::parser parser_;

		Json::FastWriter writer;
		static plugin_interface plugin;
		static bool ready;
	};

	std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, simdjson::dom::element&, guid&)>> parser_impl::impl::protocol_map = [] {
		std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, simdjson::dom::element&, guid&)>> protocol_map;
		protocol_map["Longinus.new"] = &Longinus_new_json;
		protocol_map["Longinus.delete"] = &Longinus_delete_json;
		protocol_map["Longinus.detect"] = &Longinus_detect_json;
		protocol_map["Romancia.new"] = &Romancia_new_json;
		protocol_map["Romancia.delete"] = &Romancia_delete_json;
		protocol_map["Romancia.alignFace"] = &Romancia_alignFace_json;
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
		return protocol_map;
	}();
	plugin_interface parser_impl::impl::plugin{nullptr};
	bool parser_impl::impl::ready = false;

	parser_impl::parser_impl() :impl_(new impl)
	{
	}
	parser_impl::~parser_impl()
	{
		if (impl_)
		{
			delete impl_;
			impl_ = nullptr;
		}
	}

	param_string parser_impl::parse(const param_string& protocol, const param_string& jsonstr)
	{
		return impl_->parse(protocol, jsonstr);
	}

	param_string parser_impl::query_all_instance()
	{
		return impl_->query_all_instance();
	}

	param_string parser_impl::support_protocol()
	{
		return impl_->support_protocol();
	}

	param_string parser_impl::init_plugin(const param_string& config_file_path)
	{
		return impl_->init_plugin(config_file_path);
	}
}
