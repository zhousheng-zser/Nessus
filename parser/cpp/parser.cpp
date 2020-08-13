#include "parser.hpp"
#include "abi/guid.hpp"
#include "simdjson.h"
#include "singleton.hpp"
#include "plugin_manager.hpp"
#include "plugin_interface.hpp"
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

	class parser::impl
	{
	public:
		impl()
		{
			protocol_map["Longinus.new"] = &Longinus_new_json;
			protocol_map["Longinus.delete"] = &Longinus_delete_json;
			protocol_map["Longinus.detectEx"] = &Longinus_detectEx_json;
			protocol_map["Longinus.detectRetina"] = &Longinus_detectRetina_json;
			protocol_map["Longinus.alignFace"] = &Longinus_alignFace_json;
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
				uint64_t instance = 0;
				std::function<Json::Value(plugin_interface&, simdjson::dom::element&, uint64_t&)> func;
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
					//根据instance生成guid
					auto guid_array = to_char_array(create_guid_from_bytes(meta::to_array(instance)));
					std::string instance_guid(guid_array.data(), guid_array.size());

					std::lock_guard<std::mutex> lck(mut_instance_map);

					instance_map[instance_guid] = std::tuple<std::string, uint64_t, std::shared_ptr<std::mutex>>(instance_type, instance, std::shared_ptr<std::mutex>(new std::mutex));
					value["instance_guid"] = Json::Value(instance_guid);
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

				std::shared_ptr<std::mutex> mut_instance;
				try
				{
					std::lock_guard<std::mutex> lck(mut_instance_map);
					mut_instance = std::get<2>(instance_map.at(instance_guid));
				}
				catch (const std::exception&)
				{
					value["status"] = Json::Value(instance_guid + " instance not found");
					return writer.write(value);
				}

				if (method == "release")
				{
					std::lock_guard<std::mutex> lck_instance_map(mut_instance_map);
					std::lock_guard<std::mutex> lck_instance(*mut_instance);
					try
					{
						auto& instance_tuple = instance_map.at(instance_guid);
					}
					catch (const std::exception&)
					{
						value["status"] = Json::Value(instance_guid + " instance not found");
						return writer.write(value);
					}

					std::function<Json::Value(plugin_interface&, simdjson::dom::element&, uint64_t&)> func;
					try
					{
						func = protocol_map.at(protocol_str);
					}
					catch (const std::exception&)
					{
						value["status"] = Json::Value("Function of the topic not register");
						return writer.write(value);
					}

					value = func(plugin, root, std::get<1>(instance_map[instance_guid]));
					instance_map.erase(instance_guid);
				}
				else
				{
					std::lock_guard<std::mutex> lck_instance(*mut_instance);
					try
					{
						std::lock_guard<std::mutex> lck(mut_instance_map);
						auto& instance_tuple = instance_map.at(instance_guid);
					}
					catch (const std::exception&)
					{
						value["status"] = Json::Value(instance_guid + " instance not found");
						return writer.write(value);
					}

					std::function<Json::Value(plugin_interface&, simdjson::dom::element&, uint64_t&)> func;
					try
					{
						func = protocol_map.at(protocol_str);
					}
					catch (const std::exception&)
					{
						value["status"] = Json::Value("Function of the topic not register");
						return writer.write(value);
					}

					value = func(plugin, root, std::get<1>(instance_map[instance_guid]));
				}
			}

			return writer.write(value);
		}

		std::string query_all_instance()
		{
			Json::Value value;
			std::lock_guard<std::mutex> lck_instance_map(mut_instance_map);
			for (auto& instance : instance_map)
				value[instance.first] = std::get<0>(instance.second);

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
						std::string pluginManager_lib = std::string(config["pluginManager_lib"].get<std::string_view>().value());

						auto factory = component_loader::instance().add_module_with_factory(to_param_string((plugin_directory / pluginManager_lib).string()));
						if (!factory)
						{
							ready = false;
							status = "{\"status\":\"Get a nullptr 'class_factory' instance\"}";
							return;
						}
						auto manager = factory.create_instance(u8"glasssix.nessus.pluginManager").as<plugin_manager>();
						if (!manager)
						{
							ready = false;
							status = "{\"status\":\"Get a nullptr 'plugin_manager' instance\"}";
							return;
						}
						//manager.load_from_directory(to_param_string(plugin_directory));
						for (auto plugin_item : config["plugin_list"].get<simdjson::dom::array>().value())
						{
							manager.load_from_file(to_param_string((plugin_directory / plugin_item.get<std::string_view>().value()).string()));
						}

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
		std::unordered_map<std::string, std::tuple<std::string, uint64_t, std::shared_ptr<std::mutex>>> instance_map;
		std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, simdjson::dom::element&, uint64_t&)>> protocol_map;
		simdjson::dom::parser parser_;

		Json::FastWriter writer;
		plugin_interface plugin;

		std::mutex mut_parse;
		std::mutex mut_instance_map;
		bool ready;
	};

	parser::parser() : impl_{ new impl }
	{
	}

	parser::~parser()
	{
		if (impl_)
		{
			delete impl_;
			impl_ = nullptr;
		}
	}

	std::string parser::parse(std::string_view topic, std::string_view jsonstr)
	{
		return impl_->parse(topic, jsonstr);
	}

	std::string parser::query_all_instance()
	{
		return impl_->query_all_instance();
	}

	std::string parser::support_protocol()
	{
		return impl_->support_protocol();
	}

	std::string parser::init_plugin(std::string_view config_file_path)
	{
		return impl_->init_plugin(config_file_path);
	}
}
