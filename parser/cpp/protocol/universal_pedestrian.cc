#pragma once
#include "../protocol_register.hpp"
#include "../message_protocol_jsoncpp.hpp"
#include <universal_pedestrian/detect_code.hpp>
#include <universal_pedestrian/box_info.hpp>
//

namespace glasssix::exposing::nessus::Protocol {

	class S_Universal_pedestrian : public Protocol
	{
		static Json::Value Universal_pedestrian_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;

			try {

				std::string models_directory = root["models_directory"].asString();
				int device = root["device"].asInt();
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"device", box(device)}, {u8"models_directory", box(std::string_view(models_directory))} });


				instance = unbox<guid>(plugin.execute(u8"universal_pedestrian.new", param));
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

		// static Json::Value Universal_pedestrian_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		// {
		// 	Json::Value value;
		// 	try
		// 	{
		// 		auto param = make_param_hash_map<param_string, unknown_object>(
		// 			{ {u8"object_id", box(instance)} });

		// 		auto version = plugin.execute(u8"universal_pedestrian.version", param);

		// 		value["version"] = Json::Value(glasssix::exposing::to_narrow_string(unbox<param_string>(version)));

		// 		value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));

		// 	}
		// 	catch (const parser_exception& ex)
		// 	{
		// 		value["status"]["message"] = Json::Value(ex.what());
		// 		value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
		// 	}
		// 	catch (const Json::Exception& ex)
		// 	{
		// 		value["status"]["message"] = Json::Value(ex.what());
		// 		value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
		// 	}
		// 	catch (const std::exception& ex)
		// 	{
		// 		value["status"]["message"] = Json::Value(ex.what());
		// 		value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
		// 	}
		// 	catch (const abi_error& ex)
		// 	{
		// 		value["status"]["message"] = Json::Value(ex.what_to_narrow());
		// 		value["status"]["code"] = Json::Int(ex.result());
		// 	}

		// 	return value;
		// }

		static Json::Value Universal_pedestrian_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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
						{u8"object_id", box(instance)},
						{u8"params", param_map_abi},
					});

				auto result = plugin.execute(u8"universal_pedestrian.detect", param).as<exposing::param_vector<universal_pedestrian::box_info>>();

				Json::Value jarray_box;
				Json::Value jarray_universal_pedestrian_detected(Json::arrayValue);			


				for (int i = 0; i < result.size(); i++)
				{

                    jarray_box["x1"] = Json::Int(result[i].x1());
                    jarray_box["y1"] = Json::Int(result[i].y1());
                    jarray_box["x2"] = Json::Int(result[i].x2());
                    jarray_box["y2"] = Json::Int(result[i].y2());
                    jarray_box["score"]= Json::Value(result[i].score());
                    jarray_universal_pedestrian_detected.append(jarray_box);



				}

				Json::Value jarray_info;

				jarray_info["with_universal_pedestrian_list"] = jarray_universal_pedestrian_detected;



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

		static Json::Value Universal_pedestrian_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				plugin.execute(u8"universal_pedestrian.delete", param);

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
			protocol_map["universal_pedestrian.new"] = &Universal_pedestrian_new_json;
			protocol_map["universal_pedestrian.delete"] = &Universal_pedestrian_delete_json;
			protocol_map["universal_pedestrian.detect"] = &Universal_pedestrian_detect_json;
			// protocol_map["universal_pedestrian.version"] = &Universal_pedestrian_version_json;

			return protocol_map;
		}
	};

	REGISTE_PROTOCOL(S_Universal_pedestrian)

}