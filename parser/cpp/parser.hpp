#ifndef _PARSER_HPP_
#define _PARSER_HPP_
#include <string>
#include <unordered_map>
#include <utility>
#include <mutex>
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

				void init_plugin(string config_file_path)
				{
					static std::once_flag flag;

					std::call_once(flag, [&]
					{
						std::ifstream f_config(config_file_path);
						std::string buffer(std::istreambuf_iterator<char>{ f_config }, std::istreambuf_iterator<char>{});

						simdjson::dom::element config = parser_.parse(buffer);
						try
						{
							string plugin_directory = string(config["plugin_directory"].get<std::string_view>().value());
							string pluginManager_lib = string(config["pluginManager_lib"].get<std::string_view>().value());
							auto factory = component_loader::instance().add_module_with_factory(to_param_string(plugin_directory + "/" + pluginManager_lib));
							auto manager = factory.create_instance(u8"glasssix.nessus.pluginManager").as<plugin_manager>();
							manager.load_from_directory(to_param_string(plugin_directory));

							plugin = manager.lookup(u8"glasssix.nessus.visionService");

							ready = true;
						}
						catch (const std::exception& ex)
						{
							ready = false;
						}
						catch (const abi_error& ex)
						{
							ready = false;
						}
					});
				}

			private:
				parser()
				{
					protocol_map["Logninus.new"] = &Longinus_new_json;
					protocol_map["Logninus.release"] = &Longinus_delete_json;
					protocol_map["Logninus.detectEx"] = &Longinus_detectEx_json;
					protocol_map["Logninus.detectRetina"] = &Longinus_detectRetina_json;
					protocol_map["Logninus.alignFace"] = &Longinus_alignFace_json;
					protocol_map["Gaius.new"] = &Gaius_new_json;
					protocol_map["Gaius.release"] = &Gaius_delete_json;
					protocol_map["Gaius.Forward"] = &Gaius_Forward_json;
					protocol_map["Cassius.new"] = &Cassius_new_json;
					protocol_map["Cassius.release"] = &Cassius_delete_json;
					protocol_map["Cassius.Forward"] = &Cassius_Forward_json;
					protocol_map["Irisviel.new"] = &Irisviel_new_json;
					protocol_map["Irisviel.release"] = &Irisviel_delete_json;
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
				static unordered_map<string, std::function<Json::Value(plugin_interface&, simdjson::dom::element&, uint64_t&)>> protocol_map;
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

