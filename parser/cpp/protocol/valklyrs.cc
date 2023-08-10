#pragma once
#include "../protocol_register.hpp"
#include "../message_protocol_jsoncpp.hpp"
#include <valklyrs/result_info.hpp>
#include <valklyrs/vp_info.hpp>

namespace glasssix::exposing::nessus::Protocol {

	class P_Valklyrs : public Protocol
	{
		static Json::Value Valklyrs_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				int device = root["device"].asInt();
				std::string models_directory = root["models_directory"].asString();
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"device", box(device)},
					{u8"models_directory", box(std::string_view(models_directory))} });
				instance = unbox<guid>(plugin.execute(u8"valklyrs.new", param));
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

		static Json::Value Valklyrs_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				plugin.execute(u8"valklyrs.delete", param);

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

		static Json::Value Valklyrs_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				int format = root["format"].asInt();
				int height = root["height"].asInt();
				int width = root["width"].asInt();

				auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
				param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"image", box(image_span)},
					{u8"height", box(height)},
					{u8"width", box(width)},
					{u8"order", box(static_cast<int>(frame->format_))},
					{u8"object_id", box(instance)} });

				auto result = plugin.execute(u8"valklyrs.detect", param).as<param_vector<valklyrs::result_info>>();

				Json::Value jbox;
				Json::Value jvehicle_list = Json::Value(Json::arrayValue);
				Json::Value jperson_list = Json::Value(Json::arrayValue);

				auto vehicle_list = result[0].vehicle_list();
				auto person_list = result[0].person_list();
				for (auto&& vp_info : vehicle_list)
				{
					Json::Value vp_obj;
					Json::Value coordinate_obj;
					Json::Value attribute_obj;
					auto coordinates = vp_info.coordinates();
					auto attributes = vp_info.attributes();
					coordinate_obj["x"] = Json::Int(coordinates[0]);
					coordinate_obj["y"] = Json::Int(coordinates[1]);
					coordinate_obj["width"] = Json::Int(coordinates[2]);
					coordinate_obj["height"] = Json::Int(coordinates[3]);
					attribute_obj["color"] = Json::Value(exposing::to_narrow_string(attributes[0]));
					attribute_obj["orientation"] = Json::Value(exposing::to_narrow_string(attributes[1]));
					attribute_obj["car_type"] = Json::Value(exposing::to_narrow_string(attributes[2]));
					vp_obj["coordinates"] = coordinate_obj;
					vp_obj["attributes"] = attribute_obj;
					jvehicle_list.append(vp_obj);
				}

				for (auto&& vp_info : person_list)
				{
					Json::Value vp_obj;
					Json::Value coordinate_obj;
					Json::Value attribute_obj;
					auto coordinates = vp_info.coordinates();
					auto attributes = vp_info.attributes();
					coordinate_obj["x"] = Json::Int(coordinates[0]);
					coordinate_obj["y"] = Json::Int(coordinates[1]);
					coordinate_obj["width"] = Json::Int(coordinates[2]);
					coordinate_obj["height"] = Json::Int(coordinates[3]);
					attribute_obj["gender"] = Json::Value(exposing::to_narrow_string(attributes[0]));
					attribute_obj["age"] = Json::Value(exposing::to_narrow_string(attributes[1]));
					attribute_obj["ori"] = Json::Value(exposing::to_narrow_string(attributes[2]));
					attribute_obj["hat"] = Json::Value(exposing::to_narrow_string(attributes[3]));
					attribute_obj["glass"] = Json::Value(exposing::to_narrow_string(attributes[4]));
					attribute_obj["handbag"] = Json::Value(exposing::to_narrow_string(attributes[5]));
					attribute_obj["shoulderbag"] = Json::Value(exposing::to_narrow_string(attributes[6]));
					attribute_obj["backpack"] = Json::Value(exposing::to_narrow_string(attributes[7]));
					attribute_obj["sleeve"] = Json::Value(exposing::to_narrow_string(attributes[8]));
					attribute_obj["texture"] = Json::Value(exposing::to_narrow_string(attributes[9]));
					attribute_obj["lower_type"] = Json::Value(exposing::to_narrow_string(attributes[10]));
					vp_obj["coordinates"] = coordinate_obj;
					vp_obj["attributes"] = attribute_obj;
					jperson_list.append(vp_obj);
				}

				jbox["vehicle_list"] = jvehicle_list;
				jbox["person_list"] = jperson_list;
				value["results"] = jbox;
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
			protocol_map["valklyrs.new"] = &Valklyrs_new_json;
			protocol_map["valklyrs.delete"] = &Valklyrs_delete_json;
			protocol_map["valklyrs.detect"] = &Valklyrs_detect_json;

			return protocol_map;
		}
	};

	REGISTE_PROTOCOL(P_Valklyrs)

}