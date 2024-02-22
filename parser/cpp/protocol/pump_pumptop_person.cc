#pragma once
#include "../protocol_register.hpp"
#include "../message_protocol_jsoncpp.hpp"
#include <pump_pumptop_person/detect_code.hpp>
#include <pump_pumptop_person/box_info.hpp>

#include <pedestrian/box_info.hpp>

namespace glasssix::exposing::nessus::Protocol {

	class P_Pump_pumptop_person : public Protocol
	{
		static Json::Value Pump_pumptop_person_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				auto version = plugin.execute(u8"pump_pumptop_person.version", param);

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

		static Json::Value Pump_pumptop_person_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;

			try {
				int device = root["device"].asInt();
				std::string models_directory = root["models_directory"].asString();
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"device", box(device)},
					{u8"models_directory", box(std::string_view(models_directory))} });

				instance = unbox<guid>(plugin.execute(u8"pump_pumptop_person.new", param));
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

		static Json::Value Pump_pumptop_person_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				plugin.execute(u8"pump_pumptop_person.delete", param);

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

		static Json::Value Pump_pumptop_person_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				int format = root["format"].asInt();
				int height = root["height"].asInt();
				int width = root["width"].asInt();

				Json::Value params = root.get("params", Json::Value());

				auto param_map_abi = exposing::make_param_hash_map<exposing::param_string, float>();

				for (auto& param_name : params.getMemberNames()) {
					param_map_abi.add_or_update(param_name.c_str(), params[param_name].asFloat());
				}

				Json::Value pedestrain_info(Json::arrayValue);
				pedestrain_info = root["person_list"];

                auto pedestrain_info_abi = exposing::make_param_vector<pedestrian::box_info>();
                for (int i = 0; i < pedestrain_info.size(); i++)
                {
					auto temp = exposing::make_exported_interface<pedestrian::box_info>();
                    temp.set_x1(pedestrain_info[i]["x1"].asInt());
                    temp.set_y2(pedestrain_info[i]["y2"].asInt());
                    temp.set_x2(pedestrain_info[i]["x2"].asInt());
                    temp.set_y1(pedestrain_info[i]["y1"].asInt());
                    temp.set_score(pedestrain_info[i]["score"].asFloat());
                    pedestrain_info_abi.push_back(temp);
                }

				auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
				param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

				auto param = make_param_hash_map<param_string, unknown_object>(
					{
					{u8"object_id", box(instance)},
					{u8"image", box(image_span)},
					{u8"height", box(height)},
					{u8"width", box(width)},
					{u8"person_list", pedestrain_info_abi},
					{u8"params", param_map_abi},
					});

				auto result = plugin.execute(u8"pump_pumptop_person.detect", param).as<exposing::param_vector<pump_pumptop_person::box_info>>();

				Json::Value jarray_box;

				Json::Value jarray_persons_in_pumptop(Json::arrayValue);//在泵内
				Json::Value jarray_persons_out_pumptop(Json::arrayValue);//不在泵内

				for (int i = 0; i < result.size(); i++)
				{
					jarray_box["x1"] = Json::Int(result[i].x1());
					jarray_box["y1"] = Json::Int(result[i].y1());
					jarray_box["x2"] = Json::Int(result[i].x2());
					jarray_box["y2"] = Json::Int(result[i].y2());
					auto category = result[i].category();
					jarray_box["category"] = Json::Int(category);

					auto pump_loaction = result[i].pump();
					if (pump_loaction.size() == 8) {
						Json::Value jarray_pump_location_points(Json::arrayValue);

						for (int ptIdx = 0; ptIdx < 4; ptIdx++) {
							Json::Value jarray_point;
							jarray_point["x"] = pump_loaction[2 * ptIdx];
							jarray_point["y"] = pump_loaction[2 * ptIdx + 1];
							jarray_pump_location_points.append(jarray_point);
						}

						jarray_box["pump"] = jarray_pump_location_points;
					}

					if (category == 1) {
						jarray_persons_in_pumptop.append(jarray_box);
					}
					else {
						jarray_persons_out_pumptop.append(jarray_box);
					}
				}

				value["detect_info"]["persons_in_pumptop"] = jarray_persons_in_pumptop;
				value["detect_info"]["persons_out_pumptop"] = jarray_persons_out_pumptop;

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
			protocol_map["pump_pumptop_person.new"] = &Pump_pumptop_person_new_json;
			protocol_map["pump_pumptop_person.delete"] = &Pump_pumptop_person_delete_json;
			protocol_map["pump_pumptop_person.detect"] = &Pump_pumptop_person_detect_json;
			protocol_map["pump_pumptop_person.version"] = &Pump_pumptop_person_version_json;

			return protocol_map;
		}
	};

	REGISTE_PROTOCOL(P_Pump_pumptop_person)

}