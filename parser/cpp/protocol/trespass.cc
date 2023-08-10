#pragma once
#include "../protocol_register.hpp"
#include "../message_protocol_jsoncpp.hpp"
//
#include <trespass/detect_code.hpp>
#include <trespass/box_info.hpp>

namespace glasssix::exposing::nessus::Protocol {

	class P_Trespass : public Protocol
	{
		static Json::Value Trespass_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;

			try {
				std::string models_directory = root["models_directory"].asString();
				int device = root["device"].asInt();
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"device", box(device)}, {u8"models_directory", box(std::string_view(models_directory))} });

				instance = unbox<guid>(plugin.execute(u8"trespass.new", param));
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

		static Json::Value Trespass_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				int channels = root["channels"].asInt();
				int height = root["height"].asInt();
				int width = root["width"].asInt();

				int format = 1;
				auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
				param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

				auto param = make_param_hash_map<param_string, unknown_object>(
					{
						{u8"image", box(image_span)},
						{u8"channels", box(channels)},
						{u8"height", box(height)},
						{u8"width", box(width)},
						{u8"object_id", box(instance)},
					});

				auto result = plugin.execute(u8"trespass.detect", param).as<param_vector<trespass::box_info>>();
				Json::Value jarray_boxes = Json::Value(Json::arrayValue);
				for (auto box : result)
				{
					Json::Value jobj_box;
					// location
					Json::Value jarray_points;
					jarray_points["x1"] = Json::Int(box.x1());
					jarray_points["y1"] = Json::Int(box.y1());
					jarray_points["x2"] = Json::Int(box.x2());
					jarray_points["y2"] = Json::Int(box.y2());

					jobj_box["location"] = jarray_points;
					jobj_box["score"] = Json::Value(box.score());
					jobj_box["category"] = Json::Int(box.category());

					Json::Value jarray_key_points = Json::Value(Json::arrayValue);
					auto key_points = box.key_points();

					for (size_t i = 0; i < (int)key_points.size() / 3; i++) {
						Json::Value KPoint;
						KPoint["x"] = Json::Int(key_points[i * 3]);
						KPoint["y"] = Json::Int(key_points[i * 3 + 1]);
						KPoint["point_score"] = Json::Value(key_points[i * 3 + 2]);
						jarray_key_points.append(KPoint);
					}
					jobj_box["key_points"] = jarray_key_points;
					jarray_boxes.append(jobj_box);
				}

				value["info_list"] = jarray_boxes;

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

		static Json::Value Trespass_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				plugin.execute(u8"trespass.delete", param);

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
			protocol_map["trespass.new"] = &Trespass_new_json;
			protocol_map["trespass.delete"] = &Trespass_delete_json;
			protocol_map["trespass.detect"] = &Trespass_detect_json;

			return protocol_map;
		}
	};

	REGISTE_PROTOCOL(P_Trespass)

}