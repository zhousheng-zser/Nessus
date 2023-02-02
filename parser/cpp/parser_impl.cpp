#include "parser_impl.hpp"
#include "plugin_interface.hpp"
#include "vision_service.hpp"
#include "plugin_manager.hpp"

#if (defined(__aarch64__) || defined(__x86_64__) || defined(_M_X64)) && defined(USE_SIMDJSON)
#include "message_protocol.hpp"
#else
#include "message_protocol_jsoncpp.hpp"
#endif

#include <filesystem.hpp>
#include <os_context.hpp>

#include <string_view>
#include <vector>
#include <mutex>
#include <unordered_map>
#include <fstream>
#include <algorithm>

#ifdef __GNUC__
#include <malloc.h>
#endif

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
#if (defined(__aarch64__) || defined(__x86_64__) || defined(_M_X64)) && defined(USE_SIMDJSON)
		param_string parse(const param_string& topic, const param_string& jstr_param, param_span<std::uint8_t> data)
		{
			Json::Value value;
			std::string topic_str(topic.data(), topic.size());
			std::transform(topic_str.begin(), topic_str.end(), topic_str.begin(), ::tolower);
			std::string_view jstr_param_view(jstr_param.data(), jstr_param.size());
			if (!ready)
			{
				value["status"]["message"] = Json::Value("parser hasn't been inited plugin");
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::INVALID_OPERATION));
				return to_param_string(writer.write(value));
			}

			simdjson::dom::element root;
			try
			{
				root = parser_.parse(jstr_param_view);
			}
			catch (const std::exception& ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
				return to_param_string(writer.write(value));
			}

			std::vector<std::string> str_vec = split(topic_str, ".");
			if ((str_vec[0] == "fusion" && str_vec.size() != 5))
			{
				value["status"]["message"] = Json::Value("topic illegal");
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::INVALID_ARGUMENT));
				return to_param_string(writer.write(value));
			}
			else if ((str_vec[0] != "fusion" && str_vec.size() != 2))
			{
				value["status"] = Json::Value("topic illegal");
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::INVALID_ARGUMENT));
				return to_param_string(writer.write(value));
			}

			if (str_vec[0] == "fusion")
			{
				std::vector<guid> guids;

				try
				{
					guids.push_back(guid(std::string(root["romancia_instance_guid"].get<std::string_view>().value())));
					guids.push_back(guid(std::string(root["gaius_instance_guid"].get<std::string_view>().value())));
				}
				catch (const simdjson::simdjson_error& ex)
				{
					value["status"]["message"] = Json::Value(ex.what());
					value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
					return to_param_string(writer.write(value));
				}

				std::function<Json::Value(plugin_interface&, simdjson::dom::element&, param_span<std::uint8_t>&, std::vector<guid>&)> func;
				try
				{
					func = fusion_protocol_map.at(topic_str);
				}
				catch (const std::exception&)
				{
					value["status"]["message"] = Json::Value("Topic \"" + topic_str+ "\" not registed.");
					value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::INVALID_ARGUMENT));
					return to_param_string(writer.write(value));
				}

				value = func(plugin, root, data, guids);
			}
			else
			{
				std::string instance_type = str_vec[0];
				std::string method = str_vec[1];

				if (method == "new")
				{
					guid instance;
					std::function<Json::Value(plugin_interface&, simdjson::dom::element&, param_span<std::uint8_t>&, guid&)> func;
					try
					{
						func = basic_protocol_map.at(topic_str);
					}
					catch (const std::exception&)
					{
						value["status"]["message"] = Json::Value("Function of the topic not registed");
						value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::INVALID_ARGUMENT));
						return to_param_string(writer.write(value));
					}

					value = func(plugin, root, data, instance);

					if (value["status"]["code"].asInt() == 0)
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
					catch (const simdjson::simdjson_error& ex)
					{
						value["status"]["message"] = Json::Value(ex.what());
						value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
						return to_param_string(writer.write(value));
					}

					std::function<Json::Value(plugin_interface&, simdjson::dom::element&, param_span<std::uint8_t>&, guid&)> func;
					try
					{
						func = basic_protocol_map.at(topic_str);
					}
					catch (const std::exception&)
					{
						value["status"]["message"] = Json::Value("Function of the topic not registed");
						value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::INVALID_ARGUMENT));
						return to_param_string(writer.write(value));
					}

					guid instance(instance_guid);
					value = func(plugin, root, data, instance);
#ifdef __GNUC__
					if (method == "delete")
						::malloc_trim(0);
#endif
				}
			}

			return to_param_string(writer.write(value));
		}
#else
		param_string parse(const param_string& topic, const param_string& str_param, param_span<std::uint8_t> data, param_span<std::uint8_t> external)
		{
			Json::Value value;
			std::string topic_str(topic.data(), topic.size());
			std::transform(topic_str.begin(), topic_str.end(), topic_str.begin(), ::tolower);
			std::string_view str_param_view(str_param.data(), str_param.size());
			if (!ready)
			{
				value["status"]["message"] = Json::Value("parser hasn't been inited plugin");
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::INVALID_OPERATION));
				return to_param_string(writer.write(value));
			}

			Json::Value root;
			try
			{
				if (str_param_view.size() != 0)
				{
					if(!parser_.parse(std::string(str_param_view), root))
						throw parser_exception(parser_exception::parser_exception_code::JSON_EXCEPTION, "parse json failed");
				}
			}
			catch (const std::exception& ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
				return to_param_string(writer.write(value));
			}

			std::vector<std::string> str_vec = split(topic_str, ".");
			if (str_vec[0] == "fusion" && str_vec.size() != 5)
			{
				value["status"]["message"] = Json::Value("topic illegal");
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::INVALID_ARGUMENT));
				return to_param_string(writer.write(value));
			}
			else if (str_vec[0] == "new" && (str_vec.size() != 2 && str_vec.size() != 3))
			{
				value["status"] = Json::Value("topic illegal");
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::INVALID_ARGUMENT));
				return to_param_string(writer.write(value));
			}

			if (str_vec[0] == "fusion")
			{
				std::vector<guid> guids;
				try
				{
					guids.push_back(guid(root[str_vec[1] + "_instance_guid"].asString()));
					guids.push_back(guid(root[str_vec[3] + "_instance_guid"].asString()));
				}
				catch (const Json::Exception& ex)
				{
					value["status"]["message"] = Json::Value(ex.what());
					value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
					return to_param_string(writer.write(value));
				}

				std::function<Json::Value(plugin_interface&, Json::Value&, param_span<std::uint8_t>&, std::vector<guid>&, param_span<std::uint8_t>&)> func;
				try
				{
					func = fusion_protocol_map.at(topic_str);
				}
				catch (const std::exception&)
				{
					value["status"]["message"] = Json::Value("Topic \"" + topic_str + "\" not registed.");
					value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::INVALID_ARGUMENT));
					return to_param_string(writer.write(value));
				}

				value = func(plugin, root, data, guids, external);
			}
			else
			{
				std::string instance_type = str_vec[0];
				std::string method = str_vec[1];

				if (method == "new")
				{
					guid instance;
					std::function<Json::Value(plugin_interface&, Json::Value&, param_span<std::uint8_t>&, guid&, param_span<std::uint8_t>&)> func;
					try
					{
						func = basic_protocol_map.at(topic_str);
					}
					catch (const std::exception&)
					{
						value["status"]["message"] = Json::Value("Function of the topic not registed");
						value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::INVALID_ARGUMENT));
						return to_param_string(writer.write(value));
					}

					value = func(plugin, root, data, instance, external);

					if (value["status"]["code"].asInt() == 0)
					{
						auto array = to_char_array(instance);
						value["instance_guid"] = Json::Value(std::string(array.begin(), array.end()));
					}
				}
				else if(str_vec.size() == 2)
				{
					std::string instance_guid = "";
					try
					{
						instance_guid = root["instance_guid"].asString();
					}
					catch (const Json::Exception& ex)
					{
						value["status"]["message"] = Json::Value(ex.what());
						value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
						return to_param_string(writer.write(value));
					}

					std::function<Json::Value(plugin_interface&, Json::Value&, param_span<std::uint8_t>&, guid&, param_span<std::uint8_t>&)> func;
					try
					{
						func = basic_protocol_map.at(topic_str);
					}
					catch (const std::exception&)
					{
						value["status"]["message"] = Json::Value("Function of the topic not registed");
						value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::INVALID_ARGUMENT));
						return to_param_string(writer.write(value));
					}

					guid instance(instance_guid);
					value = func(plugin, root, data, instance, external);

#if defined(__GNUC__) && !defined(ANDROID)
					if (method == "delete")
						::malloc_trim(0);
#endif
				}
				else
				{
					value["status"] = Json::Value("topic illegal");
					value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::INVALID_ARGUMENT));
					return to_param_string(writer.write(value));
				}
			}

			return to_param_string(writer.write(value));
		}
#endif

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
			for (auto& protocol : basic_protocol_map)
				protocol_array.append(protocol.first);
			value["protocol"] = protocol_array;

			return to_param_string(writer.write(value));
		}

#if (defined(__aarch64__) || defined(__x86_64__) || defined(_M_X64)) && defined(USE_SIMDJSON)
		param_string init_plugin(const param_string& config_file_path)
		{
			static std::once_flag flag;

			std::string status = "{\"status\":{\"message\":\"Function 'init_plugin' has beed called and could be called one time\",\"code\":-7}}";
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
								status = "{\"status\":{\"message\":\"load module '" + std::string(lib_item.get<std::string_view>().value()) + "' failed\",\"code\":-1}}";
								return;
							}
						}

						auto manager = exposing::make_exported_interface<plugin_manager>();

						manager.load_from_existing_libraries();
						plugin = manager.lookup(u8"Glasssix Vision Service");
						if (!plugin)
						{
							ready = false;
							status = "{\"status\":{\"message\":\"Get a nullptr 'plugin_interface' instance\",\"code\":-1}}";
							return;
						}

						ready = true;
						status = "{\"status\":{\"message\":\"OK\",\"code\":0}}";
					}
					catch (const std::exception& ex)
					{
						ready = false;
						status = std::string("{\"status\":{\"message\":\"") + ex.what() + std::string("\",\"code\":-99}}");
					}
					catch (const abi_error& ex)
					{
						ready = false;
						status = std::string("{\"status\":{\"message\":\"") + ex.what_to_narrow() + std::string("\",\"code\":") + std::to_string(ex.result()) + "}}";
					}
				});

			return to_param_string(status);
		}
#else
		param_string init_plugin(const param_string& config_file_path)
		{
			static std::once_flag flag;

			std::string status = "{\"status\":{\"message\":\"Function 'init_plugin' has beed called and could be called one time\",\"code\":-7}}";
			std::call_once(flag, [&]
				{
					std::ifstream f_config{ std::string(config_file_path.begin(), config_file_path.end()) };
					std::string buffer(std::istreambuf_iterator<char>{ f_config }, std::istreambuf_iterator<char>{});

					try
					{
						Json::Reader reader_temp(Json::Features::strictMode());
						Json::Value config;
						if (!reader_temp.parse(buffer, config))
							throw parser_exception(parser_exception::parser_exception_code::JSON_EXCEPTION, "parse json failed");

						for (auto lib_item : config["plugin_list"])
						{
							bool ret = get_component_loader().add_module_by_name(to_param_string(std::string_view(lib_item.asString())));
							if (!ret)
							{
								ready = false;
								status = "{\"status\":{\"message\":\"load module '" + lib_item.asString() + "' failed\",\"code\":-1}}";
								return;
							}
						}

						auto manager = exposing::make_exported_interface<plugin_manager>();

						manager.load_from_existing_libraries();
						plugin = manager.lookup(u8"Glasssix Vision Service");
						if (!plugin)
						{
							ready = false;
							status = "{\"status\":{\"message\":\"Get a nullptr 'plugin_interface' instance\",\"code\":-1}}";
							return;
						}

						ready = true;
						status = "{\"status\":{\"message\":\"OK\",\"code\":0}}";
					}
					catch (const Json::Exception& ex)
					{
						ready = false;
						status = std::string("{\"status\":{\"message\":\"") + ex.what() + std::string("\",\"code\":-98}}");
					}
					catch (const std::exception& ex)
					{
						ready = false;
						status = std::string("{\"status\":{\"message\":\"") + ex.what() + std::string("\",\"code\":-99}}");
					}
				});

			return to_param_string(status);
		}
#endif

	private:
#if (defined(__aarch64__) || defined(__x86_64__) || defined(_M_X64)) && defined(USE_SIMDJSON)
		static std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, simdjson::dom::element&, param_span<std::uint8_t>&, guid&)>> basic_protocol_map;
		static std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, simdjson::dom::element&, param_span<std::uint8_t>&, std::vector<guid>&)>> fusion_protocol_map;
		simdjson::dom::parser parser_;
#else
		static std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, Json::Value&, param_span<std::uint8_t>&, guid&, param_span<std::uint8_t>&)>> basic_protocol_map;
		static std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, Json::Value&, param_span<std::uint8_t>&, std::vector<guid>&, param_span<std::uint8_t>&)>> fusion_protocol_map;
		Json::Reader parser_;
	public:
		impl() :parser_(Json::Features::strictMode()) {}

	private:
#endif

		Json::FastWriter writer;
		static plugin_interface plugin;
		static bool ready;
	};

#if (defined(__aarch64__) || defined(__x86_64__) || defined(_M_X64)) && defined(USE_SIMDJSON)
	std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, simdjson::dom::element&, param_span<std::uint8_t>&, guid&)>> parser_impl::impl::basic_protocol_map = [] {
		std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, simdjson::dom::element&, param_span<std::uint8_t>&, guid&)>> protocol_map;
#else
	std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, Json::Value&, param_span<std::uint8_t>&, guid&, param_span<std::uint8_t>&)>> parser_impl::impl::basic_protocol_map = [] {
		std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, Json::Value&, param_span<std::uint8_t>&, guid&, param_span<std::uint8_t>&)>> protocol_map;
#endif
		protocol_map["firesmoke.new"] = &Firesmoke_new_json;
		protocol_map["firesmoke.delete"] = &Firesmoke_delete_json;
		protocol_map["firesmoke.detect"] = &Firesmoke_detect_json;
		protocol_map["refvest.new"] = &Refvest_new_json;
		protocol_map["refvest.delete"] = &Refvest_delete_json;
		protocol_map["refvest.detect"] = &Refvest_detect_json;
		protocol_map["rail.new"] = &Rail_new_json;
		protocol_map["rail.delete"] = &Rail_delete_json;
		protocol_map["rail.detect"] = &Rail_detect_json;
		protocol_map["plate.new"] = &Plate_new_json;
		protocol_map["plate.delete"] = &Plate_delete_json;
		protocol_map["plate.detect"] = &Plate_detect_json;
		protocol_map["plate.trace_init"] = &Plate_trace_init_json;
		protocol_map["plate.trace_update"] = &Plate_trace_update_json;
		protocol_map["plate.recognize"] = &Plate_recognize_json;
		protocol_map["ring.new"] = &Ring_new_json;
		protocol_map["ring.delete"] = &Ring_delete_json;
		protocol_map["ring.detect"] = &Ring_detect_json;
		protocol_map["longinus.new"] = &Longinus_new_json;
		protocol_map["longinus.delete"] = &Longinus_delete_json;
		protocol_map["longinus.detect"] = &Longinus_detect_json;
		protocol_map["longinus.trace"] = &Longinus_trace_json;
		protocol_map["longinus.center_scale_alignface"] = &Longinus_center_scale_alignFace_json;
		protocol_map["romancia.new"] = &Romancia_new_json;
		protocol_map["romancia.delete"] = &Romancia_delete_json;
		protocol_map["romancia.alignface128"] = &Romancia_alignFace_128_json;
		protocol_map["romancia.alignface"] = &Romancia_alignFace_json;
		protocol_map["romancia.blur_detect"] = &Romancia_blur_detect_json;
		protocol_map["romancia.mask_detect"] = &Romancia_mask_detect_json;
		protocol_map["romancia.rotate"] = &Romancia_rotate_json;
		protocol_map["gaius.new"] = &Gaius_new_json;
		protocol_map["gaius.delete"] = &Gaius_delete_json;
		protocol_map["gaius.forward"] = &Gaius_forward_json;
		protocol_map["gaius.make_mask_forward"] = &Gaius_make_mask_forward_json;
		protocol_map["cassius.new"] = &Cassius_new_json;
		protocol_map["cassius.delete"] = &Cassius_delete_json;
		protocol_map["cassius.forward"] = &Cassius_forward_json;
		protocol_map["selene.new"] = &Selene_new_json;
		protocol_map["selene.new.test"] = &Selene_new_test_json;
		protocol_map["selene.delete"] = &Selene_delete_json;
		protocol_map["selene.forward"] = &Selene_forward_json;
		protocol_map["selene.make_mask_forward"] = &Selene_make_mask_forward_json;
		protocol_map["damocles.new"] = &Damocles_new_json;
		protocol_map["damocles.delete"] = &Damocles_delete_json;
		protocol_map["damocles.spoofing_detect"] = &Damocles_spoofing_detect_json;
		protocol_map["damocles.presentation_attack_detect"] = &Damocles_presentation_attack_detect_json;
		protocol_map["irisviel.new"] = &Irisviel_new_json;
		protocol_map["irisviel.delete"] = &Irisviel_delete_json;
		protocol_map["irisviel.search"] = &Irisviel_search_json;
		protocol_map["irisviel.search_nf"] = &Irisviel_search_nf_json;
		protocol_map["irisviel.clear"] = &Irisviel_clear_json;
		protocol_map["irisviel.record_count"] = &Irisviel_record_count_json;
		protocol_map["irisviel.contains_key"] = &Irisviel_contains_key_json;
		protocol_map["irisviel.try_get_record"] = &Irisviel_try_get_record_json;
		protocol_map["irisviel.remove_all"] = &Irisviel_remove_all_json;
		protocol_map["irisviel.load_databases"] = &Irisviel_load_databases_json;
		protocol_map["irisviel.remove_records"] = &Irisviel_remove_records_json;
		protocol_map["irisviel.add_records"] = &Irisviel_add_records_json;
		protocol_map["irisviel.update_records"] = &Irisviel_update_records_json;
        protocol_map["gungnir.new"] = &Gungnir_new_json;
		protocol_map["gungnir.delete"] = &Gungnir_delete_json;
		protocol_map["gungnir.detect"] = &Gungnir_detect_json;
        protocol_map["mjollner.new"] = &Mjollner_new_json;
		protocol_map["mjollner.delete"] = &Mjollner_delete_json;
		protocol_map["mjollner.detect"] = &Mjollner_detect_json;
        protocol_map["valklyrs.new"] = &Valklyrs_new_json;
		protocol_map["valklyrs.delete"] = &Valklyrs_delete_json;
		protocol_map["valklyrs.detect"] = &Valklyrs_detect_json;
        protocol_map["heimdall.new"] = &Heimdall_new_json;
		protocol_map["heimdall.delete"] = &Heimdall_delete_json;
		protocol_map["heimdall.detect"] = &Heimdall_detect_json;
        protocol_map["banshee.new"] = &Banshee_new_json;
		protocol_map["banshee.delete"] = &Banshee_delete_json;
		protocol_map["banshee.init"] = &Banshee_init_json;
		protocol_map["banshee.update"] = &Banshee_update_json;
		return protocol_map;
	}();

#if (defined(__aarch64__) || defined(__x86_64__) || defined(_M_X64)) && defined(USE_SIMDJSON)
	std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, simdjson::dom::element&, param_span<std::uint8_t>&, std::vector<guid>&)>> parser_impl::impl::fusion_protocol_map = [] {
		std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, simdjson::dom::element&, param_span<std::uint8_t>&, std::vector<guid>&)>> protocol_map;
#else
	std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, Json::Value&, param_span<std::uint8_t>&, std::vector<guid>&, param_span<std::uint8_t>&)>> parser_impl::impl::fusion_protocol_map = [] {
		std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, Json::Value&, param_span<std::uint8_t>&, std::vector<guid>&, param_span<std::uint8_t>&)>> protocol_map;
#endif
		protocol_map["fusion.romancia.alignface128.gaius.forward"] = &Fusion_Romancia_alignFace128_Gaius_forward_json;
		protocol_map["fusion.romancia.alignface.cassius.forward"] = &Fusion_Romancia_alignFace_Cassius_forward_json;
		protocol_map["fusion.romancia.alignface.selene.forward"] = &Fusion_Romancia_alignFace_Selene_forward_json;
		protocol_map["fusion.romancia.rotate.longinus.detect"] = &Fusion_Romancia_rotate_Longinus_detect_json;
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

	param_string parser_impl::parse(const param_string& topic, const param_string& jstr_param, param_span<std::uint8_t> data, param_span<std::uint8_t> external)
	{
		return impl_->parse(topic, jstr_param, data, external);
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
