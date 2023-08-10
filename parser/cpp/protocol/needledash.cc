//#pragma once
//#include "../protocol_register.hpp"
//#include "../message_protocol_jsoncpp.hpp"
//#include <needledash/ocr_code.hpp>
//#include <needledash/box_info.hpp>
//
//namespace glasssix::exposing::nessus::Protocol {
//
//	class P_Needledash : public Protocol
//	{
//		static Json::Value Needledash_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
//		{
//			Json::Value value;
//
//			try {
//				int device = root["device"].asInt();
//				std::string models_directory = root["models_directory"].asString();
//				auto param = make_param_hash_map<param_string, unknown_object>(
//					{ {u8"device", box(device)},
//					{u8"models_directory", box(std::string_view(models_directory))} });
//
//				instance = unbox<guid>(plugin.execute(u8"needledash.new", param));
//				value["status"]["message"] = Json::Value("OK");
//				value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
//			}
//			catch (const parser_exception& ex)
//			{
//				value["status"]["message"] = Json::Value(ex.what());
//				value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
//			}
//			catch (const Json::Exception& ex)
//			{
//				value["status"]["message"] = Json::Value(ex.what());
//				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
//			}
//			catch (const std::exception& ex)
//			{
//				value["status"]["message"] = Json::Value(ex.what());
//				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
//			}
//			catch (const abi_error& ex)
//			{
//				value["status"]["message"] = Json::Value(ex.what_to_narrow());
//				value["status"]["code"] = Json::Int(ex.result());
//			}
//
//			return value;
//		}
//
//		static Json::Value Needledash_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
//		{
//			Json::Value value;
//			try
//			{
//				auto param = make_param_hash_map<param_string, unknown_object>(
//					{ {u8"object_id", box(instance)} });
//
//				plugin.execute(u8"needledash.delete", param);
//
//				value["status"]["message"] = Json::Value("OK");
//				value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
//			}
//			catch (const parser_exception& ex)
//			{
//				value["status"]["message"] = Json::Value(ex.what());
//				value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
//			}
//			catch (const Json::Exception& ex)
//			{
//				value["status"]["message"] = Json::Value(ex.what());
//				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
//			}
//			catch (const std::exception& ex)
//			{
//				value["status"]["message"] = Json::Value(ex.what());
//				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
//			}
//			catch (const abi_error& ex)
//			{
//				value["status"]["message"] = Json::Value(ex.what_to_narrow());
//				value["status"]["code"] = Json::Int(ex.result());
//			}
//			return value;
//		}
//
//		static Json::Value Needledash_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
//		{
//			Json::Value value;
//			try
//			{
//				int format = 1;
//				int height = root["height"].asInt();
//				int width = root["width"].asInt();
//				int type = root["type"].asInt();
//
//				int x = root["roi"]["roi_x"].asInt();
//				int y = root["roi"]["roi_y"].asInt();
//				int roi_width = root["roi"]["roi_width"].asInt();
//				int roi_height = root["roi"]["roi_height"].asInt();
//
//				Json::Value params = root.get("params", Json::Value());
//
//				auto param_map_abi = exposing::make_param_hash_map<exposing::param_string, float>();
//
//				for (auto& param_name : params.getMemberNames()) {
//					param_map_abi.add_or_update(param_name.c_str(), params[param_name].asFloat());
//				}
//
//				auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
//				param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);
//
//				auto param = make_param_hash_map<param_string, unknown_object>(
//					{
//					{u8"image", box(image_span)},
//					{u8"height", box(height)},
//					{u8"width", box(width)},
//					{u8"type", box(type)},
//					{u8"object_id", box(instance)},
//					{u8"roi_x", box(x)},
//					{u8"roi_y", box(y)},
//					{u8"roi_width", box(roi_width)},
//					{u8"roi_height", box(roi_height)},
//					{u8"params", param_map_abi},
//					});
//
//				auto result = plugin.execute(u8"needledash.detect", param).as<exposing::param_vector<needledash::box_info>>();
//
//				Json::Value jarray_box;
//				Json::Value jarray_dash_detected;
//
//				for (int i = 0; i < result.size(); i++)
//				{
//					Json::Value jarray_location;
//
//					jarray_location["x1"] = Json::Int(result[i].x1());
//					jarray_location["y1"] = Json::Int(result[i].y1());
//					jarray_location["x2"] = Json::Int(result[i].x2());
//					jarray_location["y2"] = Json::Int(result[i].y2());
//					jarray_box["location"] = jarray_location;
//
//					jarray_box["title"] = Json::Value("needledash");
//					jarray_box["status"] = Json::Value(glasssix::exposing::to_narrow_string(result[0].strinfo()));
//					jarray_dash_detected.append(jarray_box);
//				}
//
//				value["strinfo_list"] = jarray_dash_detected;
//
//				value["status"]["message"] = Json::Value("OK");
//				value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
//
//
//			}
//			catch (const parser_exception& ex)
//			{
//				value["status"]["message"] = Json::Value(ex.what());
//				value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
//			}
//			catch (const Json::Exception& ex)
//			{
//				value["status"]["message"] = Json::Value(ex.what());
//				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
//			}
//			catch (const std::exception& ex)
//			{
//				value["status"]["message"] = Json::Value(ex.what());
//				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
//			}
//			catch (const abi_error& ex)
//			{
//				value["status"]["message"] = Json::Value(ex.what_to_narrow());
//				value["status"]["code"] = Json::Int(ex.result());
//			}
//
//			return value;
//		}
//
//	public:
//		virtual const std::unordered_map<std::string, protocol_function> parser_protocol_dump() const override {
//			std::unordered_map<std::string, protocol_function> protocol_map;
//			protocol_map["needledash.new"] = &Needledash_new_json;
//			protocol_map["needledash.delete"] = &Needledash_delete_json;
//			protocol_map["needledash.detect"] = &Needledash_detect_json;
//
//			return protocol_map;
//		}
//	};
//
//	REGISTE_PROTOCOL(P_Needledash)
//
//}