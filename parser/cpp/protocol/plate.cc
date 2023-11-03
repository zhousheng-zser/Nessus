#pragma once
#include "../protocol_register.hpp"
#include "../message_protocol_jsoncpp.hpp"
#include <plate/ocr_code.hpp>
#include <plate/box_info.hpp>

namespace glasssix::exposing::nessus::Protocol {

	class P_Plate : public Protocol
	{
		static Json::Value Plate_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;

			try {
				int device = root["device"].asInt();
				std::string models_directory = root["models_directory"].asString();
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"device", box(device)},
					{u8"models_directory", box(std::string_view(models_directory))} });

				instance = unbox<guid>(plugin.execute(u8"plate.new", param));
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

		static constexpr int plate_aligned_base64_buffer_len = TB64ENCLEN(3 * 140 * 440);
		static Json::Value Plate_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				int format = root["format"].asInt();
				int height = root["height"].asInt();
				int width = root["width"].asInt();

				int x = root["roi"]["x"].asInt();
				int y = root["roi"]["y"].asInt();
				int roi_width = root["roi"]["w"].asInt();
				int roi_height = root["roi"]["h"].asInt();

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
					{u8"order", box(static_cast<int>(frame->format_))},
					{u8"object_id", box(instance)},
					{u8"x", box(x)},
					{u8"y", box(y)},
					{u8"roi_width", box(roi_width)},
					{u8"roi_height", box(roi_height)},
					{u8"params", param_map_abi},
					});

				auto result = plugin.execute(u8"plate.detect", param).as<exposing::param_vector<plate::box_info>>();

				Json::Value jarray_boxes;

				// location
				Json::Value jarray_points;

				jarray_points["x"] = Json::Int(result[0].x());
				jarray_points["y"] = Json::Int(result[0].y());
				jarray_points["h"] = Json::Int(result[0].height());
				jarray_points["w"] = Json::Int(result[0].width());

				jarray_boxes["location"] = jarray_points;

				// aligned_images
				Json::Value jarray_aligned_images = Json::Value(Json::arrayValue);

				auto aligned_images = result[0].aligned_images();

				std::vector<std::uint8_t> temp(plate_aligned_base64_buffer_len, 0);
				std::uint8_t* ptr = temp.data();
				std::vector<std::uint8_t> buffer(aligned_images.size());
				aligned_images.copy_to(0, buffer);
				tb64xenc(buffer.data(), buffer.size(), ptr);
				jarray_aligned_images.append(Json::Value(reinterpret_cast<char*>(ptr), reinterpret_cast<char*>(ptr) + plate_aligned_base64_buffer_len));
				jarray_boxes["aligned_images"] = jarray_aligned_images;

				value["detect_info"] = jarray_boxes;

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

		static Json::Value Plate_trace_init_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				int format = root["format"].asInt();
				int height = root["height"].asInt();
				int width = root["width"].asInt();

				int roi_x = root["location"]["x"].asFloat();
				int roi_y = root["location"]["y"].asFloat();
				int roi_w = root["location"]["w"].asFloat();
				int roi_h = root["location"]["h"].asFloat();

				auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
				param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"image", box(image_span)},
					{u8"height", box(height)},
					{u8"width", box(width)},
					{u8"roi_x", box(roi_x)},
					{u8"roi_y", box(roi_y)},
					{u8"roi_width",  box(roi_w)},
					{u8"roi_height", box(roi_h)},
					{u8"order", box(static_cast<int>(frame->format_))},
					{u8"object_id", box(instance)} });

				plugin.execute(u8"plate.trace_init", param);

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

		static Json::Value Plate_trace_update_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

				auto result = plugin.execute(u8"plate.trace_update", param).as<exposing::param_vector<plate::box_info>>();

				Json::Value jobj_trace_plate;

				if (result[0].x() != 0)
				{
					value["trace_success"] = Json::Value(true);

					Json::Value jarray_boxes;
					Json::Value jarray_points;

					jarray_points["x"] = Json::Int(result[0].x());
					jarray_points["y"] = Json::Int(result[0].y());
					jarray_points["h"] = Json::Int(result[0].height());
					jarray_points["w"] = Json::Int(result[0].width());

					jarray_boxes["location"] = jarray_points;

					jobj_trace_plate = jarray_boxes;
				}
				else
				{
					value["trace_success"] = Json::Value(false);
					jobj_trace_plate = Json::Value("LOST TRACE!");
				}

				value["trace_info"] = jobj_trace_plate;
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

		static Json::Value Plate_recognize_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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
					{u8"object_id", box(instance)}
					});

				auto result = plugin.execute(u8"plate.recognize", param).as<exposing::param_vector<plate::box_info>>();

				Json::Value jobj_reco;

				if (!result[0].strinfos().empty())
				{
					value["recognize_success"] = Json::Value(true);

					// strinfos
					Json::Value jarray_strinfos;

					jarray_strinfos = Json::Value(glasssix::exposing::to_narrow_string(result[0].strinfos()));

					jobj_reco = jarray_strinfos;
				}
				else
				{
					value["recognize_success"] = Json::Value(false);
					jobj_reco = Json::Value("Plate is blocked!");
				}

				value["recognize_info"] = jobj_reco;
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

		static Json::Value Plate_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
		{
			Json::Value value;
			try
			{
				auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });

				plugin.execute(u8"plate.delete", param);

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
			protocol_map["plate.new"] = &Plate_new_json;
			protocol_map["plate.delete"] = &Plate_delete_json;
			protocol_map["plate.detect"] = &Plate_detect_json;
			protocol_map["plate.trace_init"] = &Plate_trace_init_json;
			protocol_map["plate.trace_update"] = &Plate_trace_update_json;
			protocol_map["plate.recognize"] = &Plate_recognize_json;

			return protocol_map;
		}
	};

	REGISTE_PROTOCOL(P_Plate)

}