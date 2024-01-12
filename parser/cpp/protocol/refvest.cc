#pragma once
#include "../protocol_register.hpp"
#include "../message_protocol_jsoncpp.hpp"
//
#include <refvest/classify_code.hpp>
#include <refvest/box_info.hpp>

namespace glasssix::exposing::nessus::Protocol {

	class P_Refvest : public Protocol
	{
		static Json::Value Refvest_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;

			try {
				std::string models_directory = root["models_directory"].asString();
				int device = root["device"].asInt();
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"device", box(device)}, {u8"models_directory", box(std::string_view(models_directory))} });

				instance = unbox<guid>(plugin.execute(u8"refvest.new", param));
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

		static Json::Value Refvest_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				auto version = plugin.execute(u8"refvest.version", param);

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

		static Json::Value Refvest_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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
				auto posture_info_list = root["posture_info_list"];
				int channels = 3;
				auto postures = exposing::make_param_vector<posture::box_info>();
				for( auto p : posture_info_list)
				{
					auto key_points = exposing::make_param_vector<float>();
					auto posture = exposing::make_exported_interface<posture::box_info>();
					posture.set_x1(p["location"]["x1"].asInt());
					posture.set_y1(p["location"]["y1"].asInt());
					posture.set_x2(p["location"]["x2"].asInt());
					posture.set_y2(p["location"]["y2"].asInt());
					posture.set_score(p["score"].asFloat());
					auto pts = p["key_points"];
					for(auto j : pts)
					{
						key_points.push_back(j["x"].asInt());
						key_points.push_back(j["y"].asInt());
						key_points.push_back(j["point_score"].asFloat());
					}
					posture.set_key_points(key_points);
					postures.push_back(posture);
				}

				Json::Value params = root.get("params", Json::Value());

				auto param_map_abi = exposing::make_param_hash_map<exposing::param_string, float>();

				for (auto& param_name : params.getMemberNames()) {
					param_map_abi.add_or_update(param_name.c_str(), params[param_name].asFloat());
				}

				auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
				param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

				auto param = make_param_hash_map<param_string, unknown_object>(
					{
						{u8"image", box(image_span)},
						{u8"height", box(height)},
						{u8"width", box(width)},
						{u8"roi_x", box(roi_x)},
						{u8"roi_y", box(roi_y)},
						{u8"roi_width", box(roi_width)},
						{u8"roi_height", box(roi_height)},
						{u8"channels", box(channels)},
						{u8"object_id", box(instance)},
						{u8"posture_info_list", postures},
						{u8"params", param_map_abi},

					});

				auto result = plugin.execute(u8"refvest.detect", param).as<param_vector<refvest::box_info>>();
				Json::Value jarray_box;
				Json::Value jarray_offvest_detected(Json::arrayValue);
				Json::Value jarray_withvest_detected(Json::arrayValue);

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
						jarray_box["score"] = Json::Value(result[i].score());
						jarray_withvest_detected.append(jarray_box);
					}
					else if (category == 0)
					{
						jarray_box["x1"] = Json::Int(result[i].x1());
						jarray_box["y1"] = Json::Int(result[i].y1());
						jarray_box["x2"] = Json::Int(result[i].x2());
						jarray_box["y2"] = Json::Int(result[i].y2());
						jarray_box["score"] = Json::Value(result[i].score());
						jarray_offvest_detected.append(jarray_box);
					}
				}

				Json::Value jarray_info;

				jarray_info["with_refvest_list"] = jarray_withvest_detected;
				jarray_info["without_refvest_list"] = jarray_offvest_detected;

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

		static Json::Value Refvest_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				plugin.execute(u8"refvest.delete", param);

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
			protocol_map["refvest.new"] = &Refvest_new_json;
			protocol_map["refvest.delete"] = &Refvest_delete_json;
			protocol_map["refvest.detect"] = &Refvest_detect_json;
			protocol_map["refvest.version"] = &Refvest_version_json;

			return protocol_map;
		}
	};

	REGISTE_PROTOCOL(P_Refvest)

}