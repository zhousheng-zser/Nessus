#pragma once
#include "../protocol_register.hpp"
#include "../message_protocol_jsoncpp.hpp"
#include <pumptop_helmet/pumptop_helmet_info.hpp>
#include <pumptop_helmet/pumptop_helmet_detector.hpp>

namespace glasssix::exposing::nessus::Protocol
{

	class P_PumptopHelmet : public Protocol
	{
		static Json::Value PumptopHelmet_new_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t> &external)
		{
			Json::Value value;

			try
			{

				std::string models_directory = root["models_directory"].asString();
				int device = root["device"].asInt();
				auto param = make_param_hash_map<param_string, unknown_object>(
					{{u8"device", box(device)}, {u8"models_directory", box(std::string_view(models_directory))}});

				instance = unbox<guid>(plugin.execute(u8"pumptop_helmet.new", param));
				value["status"]["message"] = Json::Value("OK");
				value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
			}
			catch (const parser_exception &ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
			}
			catch (const Json::Exception &ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
			}
			catch (const std::exception &ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
			}
			catch (const abi_error &ex)
			{
				value["status"]["message"] = Json::Value(ex.what_to_narrow());
				value["status"]["code"] = Json::Int(ex.result());
			}

			return value;
		}

		static Json::Value PumptopHelmet_detect_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t> &external)
		{
			Json::Value value;
			try
			{
				int format = root["format"].asInt();
				int height = root["height"].asInt();
				int width = root["width"].asInt();

				Json::Value params = root.get("params", Json::Value());
				auto param_map_abi = exposing::make_param_hash_map<exposing::param_string, float>();

				for (auto &param_name : params.getMemberNames())
				{
					param_map_abi.add_or_update(param_name.c_str(), params[param_name].asFloat());
				}

				int channels = 3;

				auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width , height);
				param_span<std::uint8_t> image_span(const_cast<std::uint8_t *>(frame->data_), frame->size_);

				auto param = make_param_hash_map<param_string, unknown_object>(
					{
						{u8"image", box(image_span)},
						{u8"height", box(height)},
						{u8"width", box(width)},
						{u8"channels", box(channels)},
						{u8"object_id", box(instance)},
						{u8"params", param_map_abi},
					});

				auto result = plugin.execute(u8"pumptop_helmet.detect", param).as<exposing::param_vector<pumptop_helmet::pumptop_helmet_info>>();

				Json::Value jarray_normal_detected(Json::arrayValue);
				Json::Value jarray_steal_detected(Json::arrayValue);

				Json::Value jarray_box =  Json::Value(Json::arrayValue);
				// jarray_box["category"] = Json::Int(0);
				for (int i = 0; i < result.size(); i++)
				{
					Json::Value val;
					val["x1"] = Json::Value(result[i].x1());
					val["y1"] = Json::Value(result[i].y1());
					val["x2"] = Json::Value(result[i].x2());
					val["y2"] = Json::Value(result[i].y2());
					val["category"] = Json::Value(result[i].category());
					val["score"] = Json::Value(result[i].score());
					val["helmet_score"] = Json::Value(result[i].helmet_score());
					jarray_box.append(val);
					// Json::Value jarray_box;
					// if (category == 1)
					// {
					// 	jarray_box["category"] = Json::Int(1);
					// }
				}
				Json::Value jarray_info;
				jarray_info["person_list"] = jarray_box;

				value["detect_info"] = jarray_info;
				value["status"]["message"] = Json::Value("OK");
				value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
			}
			catch (const parser_exception &ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
			}
			catch (const Json::Exception &ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
			}
			catch (const std::exception &ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
			}
			catch (const abi_error &ex)
			{
				value["status"]["message"] = Json::Value(ex.what_to_narrow());
				value["status"]["code"] = Json::Int(ex.result());
			}

			return value;
		}

		static Json::Value PumptopHelmet_delete_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t> &external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{{u8"object_id", box(instance)}});

				plugin.execute(u8"pumptop_helmet.delete", param);

				value["status"]["message"] = Json::Value("OK");
				value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
			}
			catch (const parser_exception &ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
			}
			catch (const Json::Exception &ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
			}
			catch (const std::exception &ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
			}
			catch (const abi_error &ex)
			{
				value["status"]["message"] = Json::Value(ex.what_to_narrow());
				value["status"]["code"] = Json::Int(ex.result());
			}
			return value;
		}

		static Json::Value PumptopHelmet_version_json(plugin_interface &plugin, Json::Value &root, param_span<std::uint8_t> &data, guid &instance, param_span<std::uint8_t> &external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{{u8"object_id", box(instance)}});

				auto version = plugin.execute(u8"pumptop_helmet.version", param);

				value["version"] = Json::Value(glasssix::exposing::to_narrow_string(unbox<param_string>(version)));

				// Json::Value(ex.what_to_narrow())

				value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
			}
			catch (const parser_exception &ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(ex.what_code()));
			}
			catch (const Json::Exception &ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::JSON_EXCEPTION));
			}
			catch (const std::exception &ex)
			{
				value["status"]["message"] = Json::Value(ex.what());
				value["status"]["code"] = Json::Int(static_cast<int>(parser_exception::parser_exception_code::UNKNOWN_EXCEPTION));
			}
			catch (const abi_error &ex)
			{
				value["status"]["message"] = Json::Value(ex.what_to_narrow());
				value["status"]["code"] = Json::Int(ex.result());
			}

			return value;
		}

	public:
		virtual const std::unordered_map<std::string, protocol_function> parser_protocol_dump() const override
		{
			std::unordered_map<std::string, protocol_function> protocol_map;
			protocol_map["pumptop_helmet.new"] = &PumptopHelmet_new_json;
			protocol_map["pumptop_helmet.delete"] = &PumptopHelmet_delete_json;
			protocol_map["pumptop_helmet.detect"] = &PumptopHelmet_detect_json;
			protocol_map["pumptop_helmet.version"] = &PumptopHelmet_version_json;

			return protocol_map;
		}
	};

	REGISTE_PROTOCOL(P_PumptopHelmet)

}