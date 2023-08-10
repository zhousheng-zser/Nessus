#pragma once
#include "../protocol_register.hpp"
#include "../message_protocol_jsoncpp.hpp"
#include <mjollner/box_info.hpp>

namespace glasssix::exposing::nessus::Protocol {

	class P_Mjollner : public Protocol
	{

		static Json::Value Mjollner_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				int device = root["device"].asInt();
				std::string models_directory = root["models_directory"].asString();
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"device", box(device)},
					{u8"models_directory", box(std::string_view(models_directory))} });
				instance = unbox<guid>(plugin.execute(u8"mjollner.new", param));
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

		static Json::Value Mjollner_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				plugin.execute(u8"mjollner.delete", param);

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

		static Json::Value Mjollner_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				int format = root["format"].asInt();
				int height = root["height"].asInt();
				int width = root["width"].asInt();

				Json::Value roi = root.get("roi", Json::Value());
				bool flag = roi.empty();
				int x = flag ? 0 : roi["x"].asInt();
				int y = flag ? 0 : roi["y"].asInt();
				int roi_width = flag ? width : roi["width"].asInt();
				int roi_height = flag ? height : roi["height"].asInt();

				auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
				param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

				auto param = make_param_hash_map<param_string, unknown_object>(
					{
					{u8"image", box(image_span)},
					{u8"height", box(height)},
					{u8"width", box(width)},
					{u8"order", box(static_cast<int>(frame->format_))},
					{u8"object_id", box(instance)},
					{u8"x", box(x)},
					{u8"y", box(y)},
					{u8"roi_width", box(roi_width)},
					{u8"roi_height", box(roi_height)},
					});

				auto result = plugin.execute(u8"mjollner.detect", param).as<param_vector<mjollner::box_info>>();
				Json::Value jarray_boxes = Json::Value(Json::arrayValue);
				for (auto box : result)
				{
					Json::Value jobj_box;
					Json::Value jarray_points = Json::Value(Json::arrayValue);
					auto location = box.location();
					for (size_t i = 0; i < 4; i++)
					{
						Json::Value point;
						point["x"] = Json::Int(location[i * 2]);
						point["y"] = Json::Int(location[i * 2 + 1]);

						jarray_points.append(point);
					}
					jobj_box["location"] = jarray_points;
					jobj_box["strinfo"] = Json::Value(exposing::to_narrow_string(box.strinfo()));
					jobj_box["angle"] = Json::Value(box.angle());

					jarray_boxes.append(jobj_box);
				}

				value["strinfo_list"] = jarray_boxes;

				// value["strinfo_list"] = "hello world";
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
			protocol_map["mjollner.new"] = &Mjollner_new_json;
			protocol_map["mjollner.delete"] = &Mjollner_delete_json;
			protocol_map["mjollner.detect"] = &Mjollner_detect_json;

			return protocol_map;
		}
	};

	REGISTE_PROTOCOL(P_Mjollner)

}