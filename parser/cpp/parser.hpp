#ifndef _PARSER_HPP_
#define _PARSER_HPP_
#include <string>
#include <unordered_map>
#include <utility>
#include <mutex>
#include <fstream>
#include <filesystem.hpp>
#include "singleton.hpp"
#include "simdjson.h"

#include "plugin_manager.hpp"
#include "plugin_interface.hpp"

#include "message_protocol.hpp"

using std::string;
using std::unordered_map;
using std::pair;

namespace glasssix
{
	namespace exposing
	{
		namespace nessus
		{
			class parser : public singleton<parser>
			{
			public:
				friend singleton<parser>;

				string parse(string& topic, string &jsonstr);
				string query_all_instance()
				{
					Json::Value value;
					std::lock_guard<std::mutex> lck_instance_map(mut_instance_map);
					for (auto &instance : instance_map)
						value[instance.first] = std::get<0>(instance.second);

					return writer.write(value);
				}

				string support_protocol()
				{
					Json::Value value;
					Json::Value protocol_array = Json::Value(Json::arrayValue);
					for (auto &protocol : protocol_map)
						protocol_array.append(protocol.first);
					value["protocol"] = protocol_array;

					return writer.write(value);
				}

				string init_plugin(string config_file_path)
				{
					static std::once_flag flag;

					string status = "{\"status\":\"Function 'init_plugin' has beed called and could be called one time\"}";
					std::call_once(flag, [&]
					{
						std::ifstream f_config(config_file_path);
						std::string buffer(std::istreambuf_iterator<char>{ f_config }, std::istreambuf_iterator<char>{});

						try
						{
							simdjson::dom::element config = parser_.parse(buffer);
							fs::path plugin_directory = (config["plugin_directory"].get<std::string_view>().value());
							string pluginManager_lib = string(config["pluginManager_lib"].get<std::string_view>().value());

							auto factory = component_loader::instance().add_module_with_factory(to_param_string((plugin_directory / pluginManager_lib).string()));
							if(!factory)
							{
								ready=false;
								status = "{\"status\":\"Get a nullptr 'class_factory' instance\"}";
								return;
							}
							auto manager = factory.create_instance(u8"glasssix.nessus.pluginManager").as<plugin_manager>();
							if(!manager)
							{
								ready=false;
								status = "{\"status\":\"Get a nullptr 'plugin_manager' instance\"}";
								return;
							}
							//manager.load_from_directory(to_param_string(plugin_directory));
							for (auto plugin_item : config["plugin_list"].get<simdjson::dom::array>().value())
							{
								manager.load_from_file(to_param_string((plugin_directory / plugin_item.get<std::string_view>().value()).string()));
							}

							plugin = manager.lookup(u8"Glasssix Vision Service");
							if(!plugin)
							{
								ready=false;
								status = "{\"status\":\"Get a nullptr 'plugin_interface' instance\"}";
								return;
							}

							ready = true;
							status = "{\"status\":\"OK\"}";
						}
						catch (const std::exception& ex)
						{
							ready = false;
							status = string("{\"status\":\"") + ex.what() + string("\"}");
						}
						catch (const abi_error& ex)
						{
							ready = false;
							status = string("{\"status\":\"") + ex.what_to_narrow() + string("\"}");
						}
					});

					return status;
				}

			private:
				parser()
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

				unordered_map<string, std::tuple<string, uint64_t, std::shared_ptr<std::mutex>>> instance_map;
				unordered_map<string, std::function<Json::Value(plugin_interface&, simdjson::dom::element&, uint64_t&)>> protocol_map;
				simdjson::dom::parser parser_;

				Json::FastWriter writer;
				plugin_interface plugin;

				std::mutex mut_parse;
				std::mutex mut_instance_map;
				bool ready;
			};
		}
	}
}

#endif

