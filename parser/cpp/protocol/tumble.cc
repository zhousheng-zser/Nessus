#pragma once
#include "../protocol_register.hpp"
#include "../message_protocol_jsoncpp.hpp"

#include <tumble/detect_code.hpp>
#include <tumble/box_info.hpp>

namespace glasssix::exposing::nessus::Protocol {

	class P_Tumble : public Protocol
	{
		static Json::Value Tumble_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;

			try {
				std::string models_directory = root["models_directory"].asString();
				int device = root["device"].asInt();
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"device", box(device)}, {u8"models_directory", box(std::string_view(models_directory))} });

				instance = unbox<guid>(plugin.execute(u8"tumble.new", param));
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

		static Json::Value Tumble_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				int format = 1;
				int height = root["height"].asInt();
				int width = root["width"].asInt();
				int type = root["type"].asInt();

				int x = root["roi"]["roi_x"].asInt();
				int y = root["roi"]["roi_y"].asInt();
				int roi_width = root["roi"]["roi_width"].asInt();
				int roi_height = root["roi"]["roi_height"].asInt();

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
						{u8"type", box(type)},
						{u8"object_id", box(instance)},
						{u8"roi_x", box(x)},
						{u8"roi_y", box(y)},
						{u8"roi_width", box(roi_width)},
						{u8"roi_height", box(roi_height)},
						{u8"params", param_map_abi},
					});

				auto result = plugin.execute(u8"tumble.detect", param).as<exposing::param_vector<tumble::box_info>>();

				Json::Value jarrary_tumble(Json::arrayValue);
				Json::Value jarrary_normal(Json::arrayValue);

				for (int i = 0; i < result.size(); i++)
				{
					Json::Value jarray_location;
				
					int type = Json::Int(result[i].category());
					
					jarray_location["x1"] = Json::Int(result[i].x1());
					jarray_location["y1"] = Json::Int(result[i].y1());
					jarray_location["x2"] = Json::Int(result[i].x2());
					jarray_location["y2"] = Json::Int(result[i].y2());
					jarray_location["score"] = Json::Int(result[i].score());

					if (type = 0)
					{
						jarrary_normal.append(jarray_location);
					}
					else if (type = 1)
					{
						jarrary_tumble.append(jarray_location);
					}
				}

				value["detect_list"]["normal"] = jarrary_normal;
				value["detect_list"]["tumble"] = jarrary_tumble;
				value["status"]["message"] = Json::Value("ok");
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
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
			}
			catch (const abi_error& ex)
			{
				value["status"]["message"] = Json::Value(ex.what_to_narrow());
				value["status"]["code"] = Json::Int(ex.result());
			}

			return value;
		}

		static Json::Value Tumble_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				plugin.execute(u8"tumble.delete", param);

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
			protocol_map["tumble.new"] = &Tumble_new_json;
			protocol_map["tumble.delete"] = &Tumble_detect_json;
			protocol_map["tumble.detect"] = &Tumble_delete_json;

			return protocol_map;
		}
	};

	REGISTE_PROTOCOL(P_Tumble)

}