#pragma once
#include "../protocol_register.hpp"
#include "../message_protocol_jsoncpp.hpp"
//
#include <pump_gate_status/gate_status.hpp>

namespace glasssix::exposing::nessus::Protocol {

	class P_pump_gate_status : public Protocol
	{
		static Json::Value pump_gate_status_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
				try
				{
				int device =-1;
					auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"device", box(device)} });

					instance = unbox<guid>(plugin.execute(u8"pump_gate_status.new", param));
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

		static Json::Value pump_gate_status_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				auto version = plugin.execute(u8"pump_gate_status.version", param);

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

		static Json::Value pump_gate_status_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
				try
				{

				int format = root["format"].asInt();
				int height = root["height"].asInt();
				int width = root["width"].asInt();

				int roi_x = root["roi_x"].asInt();
				int roi_y = root["roi_y"].asInt();

				int roi_width = root["roi_width"].asInt();
				int roi_height = root["roi_height"].asInt();
				

				int yellow_hsv_lower = root["yellow_hsv_lower"].asInt();
				int yellow_hsv_upper = root["yellow_hsv_upper"].asInt();
				int gray_hsv_lower = root["gray_hsv_lower"].asInt();
				int gray_hsv_upper = root["gray_hsv_upper"].asInt();

				auto rois = root["rois"];

				auto door_floor_rois = exposing::make_param_vector<int>();

				door_floor_rois.push_back( rois["door"]["x1"].asInt() );
				door_floor_rois.push_back( rois["door"]["y1"].asInt() );
				door_floor_rois.push_back( rois["door"]["x2"].asInt() );
				door_floor_rois.push_back( rois["door"]["y2"].asInt() );
				door_floor_rois.push_back( rois["floor"]["x1"].asInt() );
				door_floor_rois.push_back( rois["floor"]["y1"].asInt() );
				door_floor_rois.push_back( rois["floor"]["x2"].asInt() );
				door_floor_rois.push_back( rois["floor"]["y2"].asInt() );

				Json::Value params = root.get("params", Json::Value());

				auto param_map_abi = exposing::make_param_hash_map<exposing::param_string, float>();

				for (auto& param_name : params.getMemberNames()) {
					param_map_abi.add_or_update(param_name.c_str(), params[param_name].asFloat());
				}

				int channels = 3;
				auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
				param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

				auto param = make_param_hash_map<param_string, unknown_object>(
					{
					{u8"image", box(image_span)},
					{u8"height", box(height)},
					{u8"width", box(width)},
					{u8"yellow_hsv_lower", box(yellow_hsv_lower)},
					{u8"yellow_hsv_upper", box(yellow_hsv_upper)},
					{u8"gray_hsv_lower", box(gray_hsv_lower)},
					{u8"gray_hsv_upper", box(gray_hsv_upper)},
					{u8"rois", door_floor_rois},
					{u8"channels", box(channels)},
					{u8"object_id", box(instance)},
					{u8"params", param_map_abi},
					});

				auto result = unbox<int>(plugin.execute(u8"pump_gate_status.detect", param));

				if(result)
					value["security status"] = Json::Value("dangerous");
				else
					value["security status"] = Json::Value("secure");

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

		static Json::Value pump_gate_status_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				plugin.execute(u8"pump_gate_status.delete", param);

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
			protocol_map["pump_gate_status.new"] = &pump_gate_status_new_json;
			protocol_map["pump_gate_status.delete"] = &pump_gate_status_delete_json;
			protocol_map["pump_gate_status.detect"] = &pump_gate_status_detect_json;
			protocol_map["pump_gate_status.version"] = &pump_gate_status_version_json;

			return protocol_map;
		}
	};

	REGISTE_PROTOCOL(P_pump_gate_status)

}