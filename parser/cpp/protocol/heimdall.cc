#pragma once
#include "../protocol_register.hpp"
#include "../message_protocol_jsoncpp.hpp"
#include <heimdall/material_code.hpp>
#include <heimdall/box_info.hpp>

namespace glasssix::exposing::nessus::Protocol {

	class P_Heimdall : public Protocol
	{
		static Json::Value Heimdall_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				auto version = plugin.execute(u8"heimdall.version", param);

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

		static Json::Value Heimdall_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				int device = root["device"].asInt();
				int factory_type = root["factory_type"].asInt();
				std::string models_directory = root["models_directory"].asString();

				Json::Value params = root.get("params", Json::Value());
				auto param_map_abi = exposing::make_param_hash_map<exposing::param_string, float>();
				for (auto& param_name : params.getMemberNames()) {
					param_map_abi.add_or_update(param_name.c_str(), params[param_name].asFloat());
				}

				auto param = make_param_hash_map<param_string, unknown_object>(
					{
					{u8"device", box(device)},
					{u8"factory_type", box(factory_type)},
					{u8"models_directory", box(std::string_view(models_directory))},
					{u8"params", param_map_abi}
					});
				instance = unbox<guid>(plugin.execute(u8"heimdall.new", param));
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

		static Json::Value Heimdall_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				plugin.execute(u8"heimdall.delete", param);

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

		static Json::Value Heimdall_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				int format = root["format"].asInt();
				int height = root["height"].asInt();
				int width = root["width"].asInt();
				int top_five = root["top_five"].asInt();

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
					{u8"top_five", box(top_five)},
					{u8"order", box(static_cast<int>(frame->format_))},
					{u8"object_id", box(instance)},
					{u8"x", box(x)},
					{u8"y", box(y)},
					{u8"roi_width", box(roi_width)},
					{u8"roi_height", box(roi_height)},
					});

				auto result = plugin.execute(u8"heimdall.detect", param).as<param_vector<heimdall::box_info>>();
				Json::Value jarray_boxes = Json::Value(Json::arrayValue);
				for (auto box : result)
				{
					Json::Value jobj_box;
					Json::Value jarray_points = Json::Value(Json::arrayValue);
					// location
					auto location = box.location();
					for (size_t i = 0; i < location.size() / 2; i++)
					{
						Json::Value point;
						point["x"] = Json::Int(location[i * 2]);
						point["y"] = Json::Int(location[i * 2 + 1]);

						jarray_points.append(point);
					}
					jobj_box["location"] = jarray_points;
					// strinfos
					Json::Value jarray_strinfo = Json::Value(Json::arrayValue);
					for (auto strinfo : box.strinfos())
					{
						jarray_strinfo.append(Json::Value(exposing::to_narrow_string(strinfo)));
					}
					jobj_box["strinfo"] = jarray_strinfo;
					// messages
					if (!box.messages().empty()) {
						Json::Value jarray_message = Json::Value(Json::arrayValue);
						for (auto message : box.messages())
						{
							jarray_message.append(Json::Value(exposing::to_narrow_string(message)));
						}
						jobj_box["messages"] = jarray_message;
					}
					// angle
					jobj_box["angle"] = Json::Value(box.angle());

					//jobj_box["cut_roi"] = Json::Value(Json::arrayValue);
					//
					//auto cut_roi = box.cut_roi();
					//std::vector<std::uint8_t> temp(TB64ENCLEN(cut_roi.size()), 0);
					//std::uint8_t* ptr = temp.data();
					//std::vector<std::uint8_t> buffer(cut_roi.size());
					//cut_roi.copy_to(0, buffer);

					//size_t base64_len = tb64xenc(buffer.data(), buffer.size(), ptr);

					//jobj_box["cut_roi"].append(Json::Value(reinterpret_cast<char*>(ptr), reinterpret_cast<char*>(ptr) + base64_len));

					//jobj_box["cut_roi_width"] = Json::Value(box.cut_roi_width());
					//jobj_box["cut_roi_height"] = Json::Value(box.cut_roi_height());

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
			protocol_map["heimdall.version"] = &Heimdall_version_json;
			protocol_map["heimdall.new"] = &Heimdall_new_json;
			protocol_map["heimdall.delete"] = &Heimdall_delete_json;
			protocol_map["heimdall.detect"] = &Heimdall_detect_json;

			return protocol_map;
		}
	};

	REGISTE_PROTOCOL(P_Heimdall)

}