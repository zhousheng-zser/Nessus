#pragma once
#include "../protocol_register.hpp"
#include "../message_protocol_jsoncpp.hpp"
//
#include <smoke/detect_code.hpp>
#include <smoke/box_info.hpp>

namespace glasssix::exposing::nessus::Protocol {

	class P_Smoke : public Protocol
	{
		static Json::Value Smoke_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;

			try {
				int device = root["device"].asInt();
				std::string models_directory = root["models_directory"].asString();
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"device", box(device)},
							{u8"models_directory", box(std::string_view(models_directory))} });

				instance = unbox<guid>(plugin.execute(u8"smoke.new", param));
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

		static Json::Value Smoke_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				auto version = plugin.execute(u8"smoke.version", param);

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

		static Json::Value Smoke_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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
				auto posture_info_list = root["posture_info_list"];

				auto param_map_abi = exposing::make_param_hash_map<exposing::param_string, float>();

				for (auto& param_name : params.getMemberNames()) {
					param_map_abi.add_or_update(param_name.c_str(), params[param_name].asFloat());
				}
				auto postures = exposing::make_param_vector<posture::box_info>();
				for (auto p : posture_info_list)
				{
					auto posture = exposing::make_exported_interface<posture::box_info>();
					auto key_points = exposing::make_param_vector<float>();
					auto pts = p["key_points"];
					for(auto j : pts)
					{
						key_points.push_back(j["x"].asInt());
						key_points.push_back(j["x"].asInt());
						key_points.push_back(j["point_score"].asFloat());
					}
					posture.set_x1(p["location"]["x1"].asInt());
					posture.set_x2(p["location"]["x2"].asInt());
					posture.set_y1(p["location"]["y1"].asInt());
					posture.set_y2(p["location"]["y2"].asInt());
					posture.set_score(p["score"].asFloat());
					posture.set_key_points(key_points);
					postures.push_back(posture);
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
						{u8"params", param_map_abi},
						{u8"posture_info_list", postures}
					});

				auto result = plugin.execute(u8"smoke.detect", param).as<exposing::param_vector<smoke::box_info>>();

				int work_detected = 0;
				int lying_detected = 0;
				int desk_detected = 0;
				int standing_detected = 0;

				Json::Value jarray_box;

				Json::Value jarray_normal_detected(Json::arrayValue);
				Json::Value jarray_smoke_detected(Json::arrayValue);
				for (int i = 0; i < result.size(); i++)
				{
					int category = Json::Int(result[i].category());

					// Json::Value jarray_box;
					if (category == 1)
					{
						jarray_box["x1"] = Json::Int(result[i].x1());
						jarray_box["y1"] = Json::Int(result[i].y1());
						jarray_box["x2"] = Json::Int(result[i].x2());
						jarray_box["y2"] = Json::Int(result[i].y2());
						// jarray_box["label"] = Json::Int(result[i].category());
						jarray_box["score"] = Json::Value(result[i].confidence());
						jarray_normal_detected.append(jarray_box);
					}
					else if (category == 0)
					{

						jarray_box["x1"] = Json::Int(result[i].x1());
						jarray_box["y1"] = Json::Int(result[i].y1());
						jarray_box["x2"] = Json::Int(result[i].x2());
						jarray_box["y2"] = Json::Int(result[i].y2());
						// jarray_box["label"] = Json::Int(result[i].category());
						jarray_box["score"] = Json::Value(result[i].confidence());
						jarray_smoke_detected.append(jarray_box);
					}
				}

				Json::Value jarray_info;

				jarray_info["norm_list"] = jarray_normal_detected;
				jarray_info["smoke_list"] = jarray_smoke_detected;

				value["detect_info"] = jarray_info;
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

		static Json::Value Smoke_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				plugin.execute(u8"smoke.delete", param);

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
			protocol_map["smoke.new"] = &Smoke_new_json;
			protocol_map["smoke.delete"] = &Smoke_delete_json;
			protocol_map["smoke.detect"] = &Smoke_detect_json;
			protocol_map["smoke.version"] = &Smoke_version_json;

			return protocol_map;
		}
	};

	REGISTE_PROTOCOL(P_Smoke)

}