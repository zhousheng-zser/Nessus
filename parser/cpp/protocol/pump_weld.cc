#pragma once
#include "../protocol_register.hpp"
#include "../message_protocol_jsoncpp.hpp"
#include <pump_weld/detect_code.hpp>
#include <pump_weld/box_info.hpp>

#include <pedestrian/box_info.hpp>

namespace glasssix::exposing::nessus::Protocol {

	class P_Pump_weld : public Protocol
	{
		static Json::Value Pump_weld_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				auto version = plugin.execute(u8"pump_weld.version", param);

				value["version"] = Json::Value(glasssix::exposing::to_narrow_string(unbox<param_string>(version)));

				value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));

			}
			catch (const parser_exception& ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
			}
			catch (const Json::Exception& ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
			}
			catch (const std::exception& ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
			}
			catch (const abi_error& ex)
			{
				value["status"]["message"] = Json::Value(ex.what_to_narrow());
				value["status"]["code"] = Json::Int(ex.result());
			}

			return value;
		}

		static Json::Value Pump_weld_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;

			try {
				int device = root["device"].asInt();
				std::string models_directory = root["models_directory"].asString();
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"device", box(device)},
					{u8"models_directory", box(std::string_view(models_directory))} });

				instance = unbox<guid>(plugin.execute(u8"pump_weld.new", param));
				value["status"]["message"] = Json::Value("OK");
				value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
			}
			catch (const parser_exception& ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
			}
			catch (const Json::Exception& ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
			}
			catch (const std::exception& ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
			}
			catch (const abi_error& ex)
			{
				value["status"]["message"] = Json::Value(ex.what_to_narrow());
				value["status"]["code"] = Json::Int(ex.result());
			}

			return value;
		}

		static Json::Value Pump_weld_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				plugin.execute(u8"pump_weld.delete", param);

				value["status"]["message"] = Json::Value("OK");
				value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
			}
			catch (const parser_exception& ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
			}
			catch (const Json::Exception& ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
			}
			catch (const std::exception& ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
			}
			catch (const abi_error& ex)
			{
				value["status"]["message"] = Json::Value(ex.what_to_narrow());
				value["status"]["code"] = Json::Int(ex.result());
			}
			return value;
		}

		static Json::Value Pump_weld_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				int format = root["format"].asInt();
				int width = root["width"].asInt();
				int batch = root["batch"].asInt();
				int height = root["height"].asInt();

				Json::Value params = root.get("params", Json::Value());

				//dynamic params
				auto param_map_abi = exposing::make_param_hash_map<exposing::param_string, float>();
				for (auto& param_name : params.getMemberNames()) {
					param_map_abi.add_or_update(param_name.c_str(), params[param_name].asFloat());
				}

				auto param = make_param_hash_map<param_string, unknown_object>(
					{
					{u8"object_id", box(instance)},
					{u8"image", box(data)},
					{u8"batch", box(batch)},
					{u8"height", box(height)},
					{u8"width", box(width)},
					{u8"params", param_map_abi},
					});

				auto result = plugin.execute(u8"pump_weld.detect", param).as<exposing::param_vector<pump_weld::box_info>>();

				Json::Value jarray_persons_weld(Json::arrayValue);

				for (int i = 0; i < result.size(); i++)
				{
					Json::Value jarray_box;
					jarray_box["x1"] = Json::Int(result[i].can_x1()); // 临时将can_x1改回x1
					jarray_box["x2"] = Json::Int(result[i].can_x2());//  临时将can_x2改回x2
					jarray_box["y1"] = Json::Int(result[i].can_y1());//  临时将can_y1改回y1
					jarray_box["y2"] = Json::Int(result[i].can_y2());//  临时将can_y2改回y2
					jarray_box["weld_x1"] = Json::Int(result[i].weld_x1());
					jarray_box["weld_x2"] = Json::Int(result[i].weld_x2());
					jarray_box["weld_y1"] = Json::Int(result[i].weld_y1());
					jarray_box["weld_y2"] = Json::Int(result[i].weld_y2());
					auto category = result[i].category();
					auto score = result[i].score();
					jarray_box["category"] = Json::Int(category);
					//jarray_box["score"] = Json::Int(score);

					jarray_persons_weld.append(jarray_box);
				}

				value["detect_info"]["persons_weld"] = jarray_persons_weld;

				value["status"]["message"] = Json::Value("OK");
				value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
			}

			catch (const parser_exception& ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
			}
			catch (const Json::Exception& ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
			}
			catch (const std::exception& ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
			}
			catch (const abi_error& ex)
			{
				value["status"]["message"] = Json::Value(ex.what_to_narrow());
				value["status"]["code"] = Json::Int(ex.result());
			}

			return value;
		}

	public:
		virtual const std::unordered_map<std::string, protocol_function> parser_protocol_dump() const override {
			std::unordered_map<std::string, protocol_function> protocol_map;
			protocol_map["pump_weld.new"] = &Pump_weld_new_json;
			protocol_map["pump_weld.delete"] = &Pump_weld_delete_json;
			protocol_map["pump_weld.detect"] = &Pump_weld_detect_json;
			protocol_map["pump_weld.version"] = &Pump_weld_version_json;

			return protocol_map;
		}
	};

	REGISTE_PROTOCOL(P_Pump_weld)

}