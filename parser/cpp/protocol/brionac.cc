#pragma once
#include "../protocol_register.hpp"
#include "../message_protocol_jsoncpp.hpp"
#include <brionac/classify_code.hpp>
#include <brionac/box_info.hpp>

namespace glasssix::exposing::nessus::Protocol {

	class P_Brionac : public Protocol
	{
		static Json::Value Brionac_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;

			try {

				std::string models_directory = root["models_directory"].asString();
				int device = root["device"].asInt();
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"device", box(device)}, {u8"models_directory", box(std::string_view(models_directory))} });


				instance = unbox<guid>(plugin.execute(u8"brionac.new", param));
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

		static Json::Value Brionac_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				int format = root["format"].asInt();
				int height = root["height"].asInt();
				int width = root["width"].asInt();

				int roi_x = root["roi"]["roi_x"].asInt();
				int roi_y = root["roi"]["roi_y"].asInt();
				int roi_width = root["roi"]["roi_width"].asInt();
				int roi_height = root["roi"]["roi_height"].asInt();

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
					{u8"roi_x", box(roi_x)},
					{u8"roi_y", box(roi_y)},
					{u8"roi_width", box(roi_width)},
					{u8"roi_height", box(roi_height)},
					{u8"channels", box(channels)},
					{u8"object_id", box(instance)},
					{u8"params", param_map_abi},
					});

				auto result = plugin.execute(u8"brionac.detect", param).as<exposing::param_vector<brionac::box_info>>();

				Json::Value jarray_box;
				Json::Value jarray_location;
				Json::Value jarray_dash_detected;
				for (int i = 0; i < result.size(); i++)
				{
					jarray_box["title"] = Json::Value("brionac");
					jarray_box["status"] = Json::Value(glasssix::exposing::to_narrow_string(result[0].strinfos()));

					jarray_location["x1"] = Json::Int(result[i].x1());
					jarray_location["y1"] = Json::Int(result[i].y1());
					jarray_location["x2"] = Json::Int(result[i].x2());
					jarray_location["y2"] = Json::Int(result[i].y2());

					jarray_box["location"] = jarray_location;
					jarray_dash_detected.append(jarray_box);
				}

				value["strinfo_list"] = jarray_dash_detected;

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

		static Json::Value Brionac_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				plugin.execute(u8"brionac.delete", param);

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

		static Json::Value Brionac_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				auto version = plugin.execute(u8"brionac.version", param);

				value["version"] = Json::Value(glasssix::exposing::to_narrow_string(unbox<param_string>(version)));

				// Json::Value(ex.what_to_narrow())

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
			protocol_map["brionac.new"] = &Brionac_new_json;
			protocol_map["brionac.delete"] = &Brionac_delete_json;
			protocol_map["brionac.detect"] = &Brionac_detect_json;
			protocol_map["brionac.version"] = &Brionac_version_json;

			return protocol_map;
		}
	};

	REGISTE_PROTOCOL(P_Brionac)

}