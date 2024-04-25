#pragma once
#include "../protocol_register.hpp"
#include "../message_protocol_jsoncpp.hpp"
#include <fighting/detect_code.hpp>

namespace glasssix::exposing::nessus::Protocol {

	class P_Fighting: public Protocol
	{
			static Json::Value Fighting_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try {
					int device = root["device"].asInt();
					int batch = root["batch"].asInt();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>({ 
						{u8"device", box(device)},
						{u8"batch", box(batch)},
						{u8"models_directory", box(std::string_view(models_directory))} 
					});

					instance = unbox<guid>(plugin.execute(u8"fighting.new", param));
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

			static Json::Value Fighting_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });
					
					auto version =plugin.execute(u8"fighting.version", param) ;

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

			static Json::Value Fighting_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();

					int roi_x = 0;
					int roi_y = 0;
					int roi_width = width;
					int roi_height = height;
					Json::Value rois = root.get("roi_x", Json::Value());
					bool inputRoiEmpty = rois.empty();
					if (!inputRoiEmpty) {
						roi_x = root["roi_x"].asInt();
						roi_y = root["roi_y"].asInt();
						roi_width = root["roi_width"].asInt();
						roi_height = root["roi_height"].asInt();
					}

					Json::Value params = root.get("params", Json::Value());

					auto param_map_abi = exposing::make_param_hash_map<exposing::param_string, float>();

					for (auto& param_name : params.getMemberNames()) {
						param_map_abi.add_or_update(param_name.c_str(), params[param_name].asFloat());
					}

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(data)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"object_id", box(instance)},

							{u8"roi_x", box(roi_x)},
							{u8"roi_y", box(roi_y)},

							{u8"roi_width",  box(roi_width)},
							{u8"roi_height", box(roi_height)},
							{u8"params", param_map_abi},
						});

					//auto result = plugin.execute(u8"fighting.detect", param);
					auto result = plugin.execute(u8"fighting.detect", param).as<exposing::param_vector<fighting::box_info>>();

					Json::Value jarray_box;
					Json::Value jarray_fight_detected(Json::arrayValue);
					Json::Value jarray_normal_detected(Json::arrayValue);

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
							jarray_box["category"] = Json::Value(result[i].category());
							jarray_fight_detected.append(jarray_box);
						}
						else
						{
							jarray_box["x1"] = Json::Int(result[i].x1());
							jarray_box["y1"] = Json::Int(result[i].y1());
							jarray_box["x2"] = Json::Int(result[i].x2());
							jarray_box["y2"] = Json::Int(result[i].y2());
							jarray_box["score"] = Json::Value(result[i].score());
							jarray_box["category"] = Json::Value(result[i].category());
							jarray_normal_detected.append(jarray_box);
						}
					}

					Json::Value jarray_info;

					jarray_info["fight_list"] = jarray_fight_detected;
					jarray_info["normal_list"] = jarray_normal_detected;

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
			
			static Json::Value Fighting_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"fighting.delete", param);

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
			protocol_map["fighting.new"] = &Fighting_new_json;
			protocol_map["fighting.delete"] = &Fighting_delete_json;
			protocol_map["fighting.detect"] = &Fighting_detect_json;
			protocol_map["fighting.version"] = &Fighting_version_json;

			return protocol_map;
		}
	};

	REGISTE_PROTOCOL(P_Fighting)

}