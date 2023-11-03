#pragma once
#include "../protocol_register.hpp"
#include "../message_protocol_jsoncpp.hpp"
//
#include <valve/detect_code.hpp>
#include <valve/box_info.hpp>

namespace glasssix::exposing::nessus::Protocol {

	class P_valve : public Protocol
	{

		static Json::Value Valve_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				int device = root["device"].asInt();
				int factory_type = root["factory_type"].asInt();
				std::string models_directory = root["models_directory"].asString();

				Json::Value params = root.get("params", Json::Value());
				auto param_map_abi = exposing::make_param_hash_map<exposing::param_string, float>();
				for (auto& param_name : params.getMemberNames()) {
					param_map_abi.add_or_update(param_name.c_str(), params[param_name].asFloat());
				}

				auto param = make_param_hash_map<param_string, unknown_object>(
					{
					{u8"device", box(device)},
					{u8"factory_type", box(factory_type)},
					{u8"models_directory", box(std::string_view(models_directory))},
					{u8"params", param_map_abi}
					});
				instance = unbox<guid>(plugin.execute(u8"valve.new", param));
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

		static Json::Value Valve_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				plugin.execute(u8"valve.delete", param);

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

		static Json::Value Valve_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				int format = root["format"].asInt();
				int height = root["height"].asInt();
				int width = root["width"].asInt();

				Json::Value rois = root.get("rois", Json::Value());
				bool emptyFlag = rois.empty();
				exposing::param_vector roi_list = exposing::make_param_vector<int>();
				if (emptyFlag) {
					roi_list.push_back(-1);
					roi_list.push_back(0);
					roi_list.push_back(0);
					roi_list.push_back(width);
					roi_list.push_back(height);
				}
				else {
					for (auto roi : rois) {
						roi_list.push_back(roi["type"].asInt());
						roi_list.push_back(roi["x"].asInt());
						roi_list.push_back(roi["y"].asInt());
						roi_list.push_back(roi["width"].asInt());
						roi_list.push_back(roi["height"].asInt());
					}
				}

				auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
				param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

				Json::Value params = root.get("params", Json::Value());
				auto param_map_abi = exposing::make_param_hash_map<exposing::param_string, float>();
				for (auto& param_name : params.getMemberNames()) {
					param_map_abi.add_or_update(param_name.c_str(), params[param_name].asFloat());
				}

				auto param = make_param_hash_map<param_string, unknown_object>(
					{
					{u8"object_id", box(instance)},
					{u8"image", box(image_span)},
					{u8"height", box(height)},
					{u8"width", box(width)},
					{u8"order", box(static_cast<int>(frame->format_))},
					{u8"rois", roi_list},
					{u8"params", param_map_abi}
					});

				auto result = plugin.execute(u8"valve.detect", param).as<param_vector<valve::box_info>>();
				Json::Value jarray_boxes = Json::Value(Json::arrayValue);

				std::map<int, std::string> tasks_map{
				{0,"helmet"},
				{1,"live"},
				{2,"valve"},
				{3,"planes"},
				};

				for (auto box : result)
				{
					Json::Value jobj_box;

					int task = box.task();
					int status = box.status();
					float rst_value = box.score();

					jobj_box["title"] = Json::Value(tasks_map[task]);

					auto rst_value_str = std::to_string(rst_value);

					jobj_box["status"] = Json::Value(std::to_string(rst_value));
					auto roi_location = box.location();
					if (roi_location.size() == 4) {
						jobj_box["x"] = Json::Int(roi_location[0]);
						jobj_box["y"] = Json::Int(roi_location[1]);
						jobj_box["width"] = Json::Int(roi_location[2]);
						jobj_box["height"] = Json::Int(roi_location[3]);
					}
					jarray_boxes.append(jobj_box);
				}

				value["strinfo_list"] = jarray_boxes;
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
			protocol_map["valve.new"] = &Valve_new_json;
			protocol_map["valve.delete"] = &Valve_delete_json;
			protocol_map["valve.detect"] = &Valve_detect_json;

			return protocol_map;
		}
	};

	REGISTE_PROTOCOL(P_valve)

}