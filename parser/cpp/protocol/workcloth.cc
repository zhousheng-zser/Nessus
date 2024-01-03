#pragma once
#include "../protocol_register.hpp"
#include "../message_protocol_jsoncpp.hpp"
//
#include <workcloth/classify_code.hpp>
#include <workcloth/box_info.hpp>

namespace glasssix::exposing::nessus::Protocol {

	class P_Workcloth : public Protocol
	{
		static Json::Value Workcloth_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				auto version = plugin.execute(u8"workcloth.version", param);

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

		static Json::Value Workcloth_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;

			try {
				int device = root["device"].asInt();
				std::string models_directory = root["models_directory"].asString();
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"device", box(device)},
					{u8"models_directory", box(std::string_view(models_directory))} });

				instance = unbox<guid>(plugin.execute(u8"workcloth.new", param));
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

		static Json::Value Workcloth_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				plugin.execute(u8"workcloth.delete", param);

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

		static Json::Value Workcloth_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

				Json::Value params = root.get("params", Json::Value());
				Json::Value color_hsv_cfg = root.get("color_hsv_cfg", Json::Value());
				auto posture_info_list = root["posture_info_list"];
				auto postures = exposing::make_param_vector<posture::box_info>();
				for(auto p : posture_info_list)
				{
					auto posture = exposing::make_exported_interface<posture::box_info>();
					auto key_points = exposing::make_param_vector<float>();
					auto pts = p["key_points"];
					for(auto j : pts)
					{
						key_points.push_back(j["x"].asInt());
						key_points.push_back(j["y"].asInt());
						key_points.push_back(j["point_score"].asInt());
					}
					posture.set_x1(p["location"]["x1"].asInt());
					posture.set_y1(p["location"]["y1"].asInt());
					posture.set_x2(p["location"]["x2"].asInt());
					posture.set_y2(p["location"]["y2"].asInt());
					posture.set_score(p["score"].asFloat());
					posture.set_key_points(key_points);
					postures.push_back(posture);

				}

				auto param_map_abi = exposing::make_param_hash_map<exposing::param_string, float>();
				auto color_hsv_cfg_abi = exposing::make_param_hash_map<exposing::param_string, exposing::param_vector<int>>();

				for (auto& param_name : params.getMemberNames()) {
					param_map_abi.add_or_update(param_name.c_str(), params[param_name].asFloat());
				}
				for (auto& param_name : color_hsv_cfg.getMemberNames()) {
					param_vector<int> color_array = make_param_vector<int>();
					auto iarray_data = color_hsv_cfg[param_name.c_str()];
					for (auto j : iarray_data)
						color_array.push_back(j.asInt());
					color_hsv_cfg_abi.add_or_update(param_name.c_str(), color_array);
				}
				auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
				param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

				auto param = make_param_hash_map<param_string, unknown_object>(
					{
					{u8"image", box(image_span)},
					{u8"height", box(height)},
					{u8"width", box(width)},
					{u8"object_id", box(instance)},
					{u8"roi_x", box(roi_x)},
					{u8"roi_y", box(roi_y)},
					{u8"roi_width",  box(roi_width)},
					{u8"roi_height", box(roi_height)},
					{u8"posture_info_list", postures},
					{u8"params", param_map_abi},
					{u8"color_hsv_cfg", color_hsv_cfg_abi},
					});

				auto result = plugin.execute(u8"workcloth.detect", param).as<exposing::param_vector<workcloth::box_info>>();

				Json::Value jarray_box;

				Json::Value jarray_workcloth_detected(Json::arrayValue);

				for (int i = 0; i < result.size(); i++)
				{
					jarray_box["x1"] = Json::Int(result[i].x1());
					jarray_box["y1"] = Json::Int(result[i].y1());
					jarray_box["x2"] = Json::Int(result[i].x2());
					jarray_box["y2"] = Json::Int(result[i].y2());

					jarray_box["is_sleeve"] = result[i].is_sleeve();
					// color ratio : black = 0, grey, white, red, orange, yellow, green, cyan, blue, purple
					auto color_ratios = result[i].color_ratios();
					int color_ratios_size = color_ratios.size();
					if (color_ratios_size != 10 && color_ratios_size != 30 && color_ratios_size != 40)continue;

					Json::Value jarray_color_ratios = Json::Value(Json::arrayValue);
					for (size_t i = 0; i < color_ratios_size; i++)
					{				
						jarray_color_ratios.append(Json::Value(color_ratios[i]));
					}
					jarray_box["color_ratios"] = jarray_color_ratios;
					jarray_workcloth_detected.append(jarray_box);
				}


				value["detect_info"]["cloth_list"] = jarray_workcloth_detected;

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
			protocol_map["workcloth.new"] = &Workcloth_new_json;
			protocol_map["workcloth.delete"] = &Workcloth_delete_json;
			protocol_map["workcloth.detect"] = &Workcloth_detect_json;
			protocol_map["workcloth.version"] = &Workcloth_version_json;

			return protocol_map;
		}
	};

	REGISTE_PROTOCOL(P_Workcloth)

}