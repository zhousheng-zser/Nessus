#ifndef _MESSAGEPROTOCOL_JSONCPP_HPP_
#define _MESSAGEPROTOCOL_JSONCPP_HPP_

#include <iostream>
#include <libyuv.h>
#include "json.h"
#include "base64_x.hpp"
#include "plugin_interface.hpp"
#include "parser_exception.hpp"
#include "../../common/include/gungnir/hat_info.hpp"
#include "../../common/include/gungnir/yolo_net.hpp"
#include "../../common/include/leavepost/box_info.hpp"
#include "../../common/include/leavepost/yolo_net.hpp"
#include "../../common/include/mjollner/box_info.hpp"
#include "../../common/include/heimdall/box_info.hpp"
#include "../../common/include/valklyrs/result_info.hpp"
#include "../../common/include/valklyrs/vp_info.hpp"
#include "../../common/include/banshee/track_info.hpp"
#include "../../common/include/longinus/face_info.hpp"
#include "../../common/include/irisviel/search_result.hpp"
#include "../../common/include/irisviel/record.hpp"
#include "../../common/include/ring/material_code.hpp"
#include "../../common/include/ring/box_info.hpp"
#include "../../common/include/plate/ocr_code.hpp"
#include "../../common/include/plate/box_info.hpp"
#include "../../common/include/rail/classify_code.hpp"
#include "../../common/include/rail/box_info.hpp"
#include "../../common/include/refvest/classify_code.hpp"
#include "../../common/include/refvest/box_info.hpp"
#include "../../common/include/flame/detect_code.hpp"
#include "../../common/include/flame/box_info.hpp"
#include "../../common/include/sleep/detect_code.hpp"
#include "../../common/include/sleep/box_info.hpp"
#include "../../common/include/smoke/detect_code.hpp"
#include "../../common/include/smoke/box_info.hpp"
#include "../../common/include/onphone/detect_code.hpp"
#include "../../common/include/onphone/box_info.hpp"
#include "../../common/include/trespass/detect_code.hpp"
#include "../../common/include/trespass/box_info.hpp"
#include "../../common/include/helmet/detect_code.hpp"
#include "../../common/include/helmet/box_info.hpp"
#include "../../common/include/eledash/box_info.hpp"
#include "../../common/include/eledash/classify_code.hpp"
#include "../../common/include/ebike/detect_code.hpp"
#include "../../common/include/ebike/box_info.hpp"
#include "../../common/include/callsmoke/detect_code.hpp"
#include "../../common/include/callsmoke/box_info.hpp"
#include "../../common/include/genocr/txt_code.hpp"
#include "../../common/include/genocr/box_info.hpp"
#include "../../common/include/startorus/detect_code.hpp"
#include "../../common/include/startorus/box_info.hpp"
#include "../../common/include/valve/detect_code.hpp"
#include "../../common/include/valve/box_info.hpp"
#include "../../common/include/needledash/ocr_code.hpp"
#include "../../common/include/needledash/box_info.hpp"
#include "../../common/include/playphone/detect_code.hpp"
#include "../../common/include/playphone/box_info.hpp"
#include "../../common/include/workcloth/classify_code.hpp"
#include "../../common/include/workcloth/box_info.hpp"


#include <string>
#include <memory>
#include <unordered_map>

namespace glasssix
{
	namespace exposing
	{
		namespace nessus
		{
			//supported image format
			enum class PROTOCOL_IMAGE_FORMAT
			{
				PROTOCOL_IMAGE_BGR_NCHW = 0,
				PROTOCOL_IMAGE_BGR_NHWC = 1,
				PROTOCOL_IMAGE_NV21 = 2,
				PROTOCOL_IMAGE_UNKNOW = 3
			};

			struct data_handler
			{
				data_handler() : data_{ nullptr }, size_{ 0 }, format_{ PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_UNKNOW }, is_heap_allocated_{ false } {}
				data_handler(std::uint8_t* data, size_t size, PROTOCOL_IMAGE_FORMAT format, bool is_heap_allocated) : data_{ data }, size_{ size }, format_{ format }, is_heap_allocated_{ is_heap_allocated } {}
				~data_handler()
				{
					if (is_heap_allocated_)
						if (data_)
							delete[] data_;
				}
				const std::uint8_t* data_;
				size_t size_;
				PROTOCOL_IMAGE_FORMAT format_;
				bool is_heap_allocated_;
			};

			namespace
			{
                inline void parse_aud_operation_result(Json::Value& value, exposing::param_vector<bool> result, const std::string& failure_message)
                {
                    value["status"]["message"] = Json::Value("OK");
                    value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
					
					auto&& result_array = (value["result"] = Json::Value{Json::arrayValue});

					for (auto&& item : result)
					{
						Json::Value json_item;
						
						json_item["success"] = item;
						json_item["reason"] = item ? u8"OK" : failure_message;
						result_array.append(json_item);
					}
                }
			}

			inline void convert_to_bgr(std::shared_ptr<data_handler>& src, std::shared_ptr<data_handler>& dst, int width, int height)
			{
				switch (src->format_)
				{
				case PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_BGR_NCHW:
				{
					if (width * height * 3 != src->size_)
						throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "BGR_NCHW, width * height * 3 != src->size_");
					dst = src;
				}
				case PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_BGR_NHWC:
				{
					int step = 0;
					if (src->size_ != width * height * 3)
					{
						step = ((width * 3 + 3) >> 2) << 2;
						if (src->size_ != step * height)
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "BGR_NHWC, src->size_ != width * height * 3 || src->size_ != (((width * 3 + 3) >> 2) << 2) * height");
					}
					else
						step = width * 3;

					if (step == width * 3)
						dst = src;
					else if (step > width * 3)
					{
						size_t size = 3 * height * width;
						std::uint8_t* dst_ptr = new std::uint8_t[size];
						const std::uint8_t* src_ptr = src->data_;
						for (size_t i = 0; i < height; i++)
							std::copy(src_ptr + i * step, src_ptr + i * step + width * 3, dst_ptr + i * width * 3);

						dst = std::make_shared<data_handler>(dst_ptr, size, PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_BGR_NHWC, true);
					}
					else
					{
						throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "step < width * 3");
					}

					break;
				}
				case PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_NV21:
				{
					size_t size = width * height * 3;
					if (src->size_ != (size >> 1))
						throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "convert_to_bgr: src->size_ != (width * height * 3 >> 1)");

					std::uint8_t* dst_ptr = new std::uint8_t[size];
					int aligned_src_width = (width + 1) & ~1;
					const uint8_t* y = src->data_;
					const uint8_t* uv = src->data_ + aligned_src_width * height;
					if (libyuv::NV21ToRGB24(y, width, uv, aligned_src_width, dst_ptr, width * 3, width, height))
						throw parser_exception(parser_exception::parser_exception_code::INTERNAL_FUNCTION_FAILURE, "NV21ToRGB24 failed.");

					dst = std::make_shared<data_handler>(dst_ptr, size, PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_BGR_NHWC, true);
					break;
				}
				default:
					throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Unsupported image format.");
					break;
				}
			}

			inline std::shared_ptr<data_handler> decode_and_convert(param_span<std::uint8_t> src, bool is_base64, PROTOCOL_IMAGE_FORMAT format, int width, int height)
			{
				if (height <= 0 || width <= 0)
					throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Invalid argument: height <= 0 || width <= 0");

				if (src.size() <= 0)
					throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Invalid argument: src.size() <= 0");

				std::shared_ptr<data_handler> temp;
				if (is_base64)
				{
					int current_image_str_len = TB64DECLEN(src.size());

					std::uint8_t* decoded_data = new std::uint8_t[current_image_str_len];
					tb64xdec(reinterpret_cast<const std::uint8_t*>(src.data()), src.size(), decoded_data);
					temp = std::make_shared<data_handler>(decoded_data, current_image_str_len, format, true);
				}
				else
				{
					temp = std::make_shared<data_handler>(src.data(), src.size(), format, false);
				}

				std::shared_ptr<data_handler> dst;
				convert_to_bgr(temp, dst, width, height);
				return dst;
			}

			inline Json::Value Workcloth_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });
					
					auto version =plugin.execute(u8"workcloth.version", param) ;

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

			inline Json::Value Workcloth_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try {
					int device = root["device"].asInt();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)},
						{u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"workcloth.new", param));
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

			inline Json::Value Workcloth_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"workcloth.delete", param);

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

			inline Json::Value Workcloth_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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
						{u8"object_id", box(instance)},

						{u8"roi_x", box(roi_x)},
						{u8"roi_y", box(roi_y)},

						{u8"roi_width",  box(roi_width)},
						{u8"roi_height", box(roi_height)},
						{u8"params", param_map_abi},
						});

					auto result = plugin.execute(u8"workcloth.detect", param).as<exposing::param_vector<workcloth::box_info>>();

					Json::Value jarray_box;
					Json::Value jarray_workcloth_detected(Json::arrayValue);

					for (int i = 0; i < result.size(); i++)
					{
						jarray_box["x1"] = Json::Int(result[i].x1());
						jarray_box["y1"] = Json::Int(result[i].y1());
						jarray_box["x2"] = Json::Int(result[i].x2());
						jarray_box["y2"] = Json::Int(result[i].y2());

						// rgb
						auto up_rgb = result[i].up_rgb();
						auto lw_rgb = result[i].lw_rgb();
						jarray_box["up_rgb"]["r"] = Json::Int(up_rgb[0]);
						jarray_box["up_rgb"]["g"] = Json::Int(up_rgb[1]);
						jarray_box["up_rgb"]["b"] = Json::Int(up_rgb[2]);
						jarray_box["lw_rgb"]["r"] = Json::Int(lw_rgb[0]);
						jarray_box["lw_rgb"]["g"] = Json::Int(lw_rgb[1]);
						jarray_box["lw_rgb"]["b"] = Json::Int(lw_rgb[2]);;

						jarray_box["score"] = Json::Value(result[i].score());
						jarray_workcloth_detected.append(jarray_box);
					}

					value["detect_info"] = jarray_workcloth_detected;

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

			inline Json::Value Playphone_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try {
					int device = root["device"].asInt();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)},
								{u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"playphone.new", param));
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

			inline Json::Value Playphone_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });
					
					auto version =plugin.execute(u8"playphone.version", param) ;

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

			inline Json::Value Playphone_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"playphone.delete", param);

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

			inline Json::Value Playphone_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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
							{u8"object_id", box(instance)},

							{u8"roi_x", box(roi_x)},
							{u8"roi_y", box(roi_y)},

							{u8"roi_width",  box(roi_width)},
							{u8"roi_height", box(roi_height)},
							{u8"params", param_map_abi},
						});

					auto result = plugin.execute(u8"playphone.detect", param).as<exposing::param_vector<playphone::box_info>>();

					Json::Value jarray_box;
					Json::Value jarray_phone_detected(Json::arrayValue);
					Json::Value jarray_no_phone_detected(Json::arrayValue);

					for (int i = 0; i < result.size(); i++)
					{
						int category = Json::Int(result[i].category());

						if (category == 0)
						{
							jarray_box["x1"] = Json::Int(result[i].x1());
							jarray_box["y1"] = Json::Int(result[i].y1());
							jarray_box["x2"] = Json::Int(result[i].x2());
							jarray_box["y2"] = Json::Int(result[i].y2());
							jarray_box["score"] = Json::Value(result[i].score());
							jarray_phone_detected.append(jarray_box);
						}
						else if (category == 1)
						{
							jarray_box["x1"] = Json::Int(result[i].x1());
							jarray_box["y1"] = Json::Int(result[i].y1());
							jarray_box["x2"] = Json::Int(result[i].x2());
							jarray_box["y2"] = Json::Int(result[i].y2());
							jarray_box["score"] = Json::Value(result[i].score());
							jarray_no_phone_detected.append(jarray_box);
						}
					}

					Json::Value jarray_info;

					jarray_info["phone_list"] = jarray_phone_detected;
					jarray_info["no_phone_list"] = jarray_no_phone_detected;

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

			inline Json::Value Needledash_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try {
					int device = root["device"].asInt();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)},
								{u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"needledash.new", param));
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

			inline Json::Value Needledash_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"needledash.delete", param);

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

			inline Json::Value Needledash_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int format = 1;
					int height = root["height"].asInt();
					int width = root["width"].asInt();
					int type = root["type"].asInt();

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
							{u8"type", box(type)},
							{u8"object_id", box(instance)},
							{u8"x", box(x)},
							{u8"y", box(y)},
							{u8"roi_width", box(roi_width)},
							{u8"roi_height", box(roi_height)},
							{u8"params", param_map_abi},
						});

					auto result = plugin.execute(u8"needledash.detect", param).as<exposing::param_vector<needledash::box_info>>();


					Json::Value jobj_reco;

					// jobj_reco["title"] = "needledash";

					// jobj_reco["status"] = Json::Value(glasssix::exposing::to_narrow_string(result[0].strinfo()));

					// value["recognize_info"] = jobj_reco;

					// value["status"]["message"] = Json::Value("OK");
					// value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
					Json::Value jarray_box;
					Json::Value jarray_dash_detected;
					for (int i = 0; i < result.size(); i++)
					{
						jarray_box["title"] = Json::Value("needledash");
						jarray_box["status"] = Json::Value(glasssix::exposing::to_narrow_string(result[0].strinfo()));
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

			inline Json::Value Startorus_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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
					instance = unbox<guid>(plugin.execute(u8"startorus.new", param));
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

			inline Json::Value Startorus_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"startorus.delete", param);

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

			inline Json::Value Startorus_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();

					Json::Value rois = root.get("rois", Json::Value());
					bool emptyFlag = rois.empty();
					exposing::param_vector roi_list = exposing::make_param_vector<int>(); //will be never used, helmet & live det whole image(3840*2160)
					if (emptyFlag) {
						roi_list.push_back(0);
						roi_list.push_back(0);
						roi_list.push_back(width);
						roi_list.push_back(height);
					}
					else{
						for (auto roi : rois) {
							roi_list.push_back(roi["type"].asInt());
							roi_list.push_back(roi["x"].asInt());
							roi_list.push_back(roi["y"].asInt());
							roi_list.push_back(roi["width"].asInt());
							roi_list.push_back(roi["height"].asInt());
						}
					}

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"object_id", box(instance)},
							{u8"image", box(image_span)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"order", box(static_cast<int>(frame->format_))},
							{u8"rois", roi_list}
						});

					auto result = plugin.execute(u8"startorus.detect", param).as<param_vector<startorus::box_info>>();
					Json::Value jarray_boxes = Json::Value(Json::arrayValue);
					
					std::array<std::string, 3> tasks_str{ "helmet","live","planes" };
					for (auto box : result)
					{
						Json::Value jobj_box;

						int task = box.task();
						int status = box.status();

						jobj_box["title"] = Json::Value(tasks_str[task]);
						jobj_box["status"] = Json::Value(status ? "1" : "0");
						auto roi_location = box.location();
						if (roi_location.size() == 4) {
							jobj_box["x"] = Json::Int(roi_location[0]);
							jobj_box["y"] = Json::Int(roi_location[1]);
							jobj_box["width"] = Json::Int(roi_location[2]);
							jobj_box["height"] = Json::Int(roi_location[3]);
						}
						jarray_boxes.append(jobj_box);
					}

					value["strinfo_list"] = jarray_boxes;
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


			inline Json::Value Valve_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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
					instance = unbox<guid>(plugin.execute(u8"valve.new", param));
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

			inline Json::Value Valve_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"valve.delete", param);

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

			inline Json::Value Valve_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();

					Json::Value rois = root.get("rois", Json::Value());
					bool emptyFlag = rois.empty();
					exposing::param_vector roi_list = exposing::make_param_vector<int>();
					if (emptyFlag) {
						roi_list.push_back(-1);
						roi_list.push_back(0);
						roi_list.push_back(0);
						roi_list.push_back(width);
						roi_list.push_back(height);
					}
					else {
						for (auto roi : rois) {
							roi_list.push_back(roi["type"].asInt());
							roi_list.push_back(roi["x"].asInt());
							roi_list.push_back(roi["y"].asInt());
							roi_list.push_back(roi["width"].asInt());
							roi_list.push_back(roi["height"].asInt());
						}
					}

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					Json::Value params = root.get("params", Json::Value());
					auto param_map_abi = exposing::make_param_hash_map<exposing::param_string, float>();
					for (auto& param_name : params.getMemberNames()) {
						param_map_abi.add_or_update(param_name.c_str(), params[param_name].asFloat());
					}

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
						{u8"object_id", box(instance)},
						{u8"image", box(image_span)},
						{u8"height", box(height)},
						{u8"width", box(width)},
						{u8"order", box(static_cast<int>(frame->format_))},
						{u8"rois", roi_list},
						{u8"params", param_map_abi}
						});

					auto result = plugin.execute(u8"valve.detect", param).as<param_vector<valve::box_info>>();
					Json::Value jarray_boxes = Json::Value(Json::arrayValue);

					std::map<int, std::string> tasks_map{
						{0,"helmet"},
						{1,"live"},
						{2,"valve"},
						{3,"planes"},
					};

					for (auto box : result)
					{
						Json::Value jobj_box;

						int task = box.task();
						int status = box.status();
						float rst_value = box.score();
						
						jobj_box["title"] = Json::Value(tasks_map[task]);

						auto rst_value_str = std::to_string(rst_value);

						jobj_box["status"] = Json::Value(std::to_string(rst_value));
						auto roi_location = box.location();
						if (roi_location.size() == 4) {
							jobj_box["x"] = Json::Int(roi_location[0]);
							jobj_box["y"] = Json::Int(roi_location[1]);
							jobj_box["width"] = Json::Int(roi_location[2]);
							jobj_box["height"] = Json::Int(roi_location[3]);
						}
						jarray_boxes.append(jobj_box);
					}

					value["strinfo_list"] = jarray_boxes;
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






			inline Json::Value Genocr_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int device = root["device"].asInt();
					int factory_type = root["factory_type"].asInt();
					std::string models_directory = root["models_directory"].asString();
					std::string chardic_directory = root["chardic_directory"].asString();

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
							{u8"chardic_directory", box(std::string_view(chardic_directory))},
							{u8"params", param_map_abi}
						});
					instance = unbox<guid>(plugin.execute(u8"genocr.new", param));
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

			inline Json::Value Genocr_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"genocr.delete", param);

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

			inline Json::Value Genocr_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

					auto result = plugin.execute(u8"genocr.detect", param).as<param_vector<genocr::box_info>>();
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
						// angle
						jobj_box["angle"] = Json::Value(box.angle());
						jarray_boxes.append(jobj_box);
					}

					value["strinfo_list"] = jarray_boxes;
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

			inline Json::Value Callsmoke_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try {
					std::string models_directory = root["models_directory"].asString();
					int device = root["device"].asInt();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)}, {u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"callsmoke.new", param));
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

			inline Json::Value Callsmoke_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(image_span)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"object_id", box(instance)},
							{u8"params", param_map_abi},
						});

					auto result = plugin.execute(u8"callsmoke.detect", param).as<exposing::param_vector<callsmoke::box_info>>();

					Json::Value jarray_box;
					Json::Value jarray_call_detected(Json::arrayValue);
					Json::Value jarray_smoke_detected(Json::arrayValue);

					for (int i = 0; i < result.size(); i++)
					{
						int category = Json::Int(result[i].category());

						if (category == 0)
						{
							jarray_box["x1"] = Json::Int(result[i].x1());
							jarray_box["y1"] = Json::Int(result[i].y1());
							jarray_box["x2"] = Json::Int(result[i].x2());
							jarray_box["y2"] = Json::Int(result[i].y2());
							jarray_call_detected.append(jarray_box);
						}
						else if (category == 1)
						{
							jarray_box["x1"] = Json::Int(result[i].x1());
							jarray_box["y1"] = Json::Int(result[i].y1());
							jarray_box["x2"] = Json::Int(result[i].x2());
							jarray_box["y2"] = Json::Int(result[i].y2());
							jarray_smoke_detected.append(jarray_box);
						}
					}

					Json::Value jarray_detected;
					Json::Value jarray_cant_detected;

					jarray_detected["call_list"] = jarray_call_detected;
					jarray_cant_detected["smoke_list"] = jarray_smoke_detected;

					Json::Value jarray_info;
					jarray_info.append(jarray_detected);
					jarray_info.append(jarray_cant_detected);

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

			inline Json::Value Callsmoke_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"callsmoke.delete", param);

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

			inline Json::Value Ebike_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try {
					std::string models_directory = root["models_directory"].asString();
					int device = root["device"].asInt();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)}, {u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"ebike.new", param));
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

			inline Json::Value Ebike_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(image_span)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"object_id", box(instance)},
							{u8"params", param_map_abi},
						});

					auto result = plugin.execute(u8"ebike.detect", param).as<exposing::param_vector<ebike::box_info>>();

					Json::Value jarray_detected;

					for (size_t i = 0; i < result.size(); i++)
					{
						Json::Value jarray_box;
						jarray_box["x1"] = Json::Int(result[i].x1());
						jarray_box["y1"] = Json::Int(result[i].y1());
						jarray_box["x2"] = Json::Int(result[i].x2());
						jarray_box["y2"] = Json::Int(result[i].y2());
						jarray_detected.append(jarray_box);
					}

					Json::Value jarray_info;
					jarray_info["box_list"]= jarray_detected;

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

			inline Json::Value Ebike_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"ebike.delete", param);

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

			inline Json::Value Helmet_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try {

					std::string models_directory = root["models_directory"].asString();
					int device = root["device"].asInt();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)}, {u8"models_directory", box(std::string_view(models_directory))} });

	
					instance = unbox<guid>(plugin.execute(u8"helmet.new", param));
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

			inline Json::Value Helmet_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });
					
					auto version =plugin.execute(u8"helmet.version", param) ;

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

			inline Json::Value Helmet_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

					auto result = plugin.execute(u8"helmet.detect", param).as<exposing::param_vector<helmet::box_info>>();

					Json::Value jarray_box;
					Json::Value jarray_helmet_detected(Json::arrayValue);
					Json::Value jarray_helmet_cant_detected(Json::arrayValue);

					for (int i = 0; i < result.size(); i++)
					{
						int category = Json::Int(result[i].category());

						if (category == 0)
						{
							jarray_box["x1"] = Json::Int(result[i].x1());
							jarray_box["y1"] = Json::Int(result[i].y1());
							jarray_box["x2"] = Json::Int(result[i].x2());
							jarray_box["y2"] = Json::Int(result[i].y2());
							jarray_box["score"] = Json::Value(result[i].score());
							jarray_helmet_detected.append(jarray_box);
						}
						else if (category == 1)
						{
							jarray_box["x1"] = Json::Int(result[i].x1());
							jarray_box["y1"] = Json::Int(result[i].y1());
							jarray_box["x2"] = Json::Int(result[i].x2());
							jarray_box["y2"] = Json::Int(result[i].y2());
							jarray_box["score"] = Json::Value(result[i].score());
							jarray_helmet_cant_detected.append(jarray_box);
						}
					}

					Json::Value jarray_info;

					jarray_info["with_helmet_list"] = jarray_helmet_detected;
					jarray_info["without_helmet_list"] = jarray_helmet_cant_detected;

			
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

			inline Json::Value Helmet_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"helmet.delete", param);

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

			inline Json::Value Eledash_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try {

					std::string models_directory = root["models_directory"].asString();
					int device = root["device"].asInt();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)}, {u8"models_directory", box(std::string_view(models_directory))} });

	
					instance = unbox<guid>(plugin.execute(u8"eledash.new", param));
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

			inline Json::Value Eledash_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(image_span)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"object_id", box(instance)},
							{u8"params", param_map_abi},
						});

					auto result = plugin.execute(u8"eledash.detect", param).as<exposing::param_vector<eledash::box_info>>();

					Json::Value jarray_box;
					Json::Value jarray_dash_detected;
					for (int i = 0; i < result.size(); i++)
					{
						jarray_box["title"] = Json::Value("eledash");
						jarray_box["status"] = Json::Value(glasssix::exposing::to_narrow_string(result[0].strinfos()));
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

			inline Json::Value Eledash_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"eledash.delete", param);

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

			inline Json::Value Trespass_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try {
					std::string models_directory = root["models_directory"].asString();
					int device = root["device"].asInt();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)}, {u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"trespass.new", param));
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

			inline Json::Value Trespass_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int channels = root["channels"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();

					int format = 1;
					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(image_span)},
							{u8"channels", box(channels)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"object_id", box(instance)},
						});

					auto result = plugin.execute(u8"trespass.detect", param).as<param_vector<trespass::box_info>>();
					Json::Value jarray_boxes = Json::Value(Json::arrayValue);
					for (auto box : result)
					{
						Json::Value jobj_box;
						// location
						Json::Value jarray_points;
						jarray_points["x1"] = Json::Int(box.x1());
						jarray_points["y1"] = Json::Int(box.y1());
						jarray_points["x2"] = Json::Int(box.x2());
						jarray_points["y2"] = Json::Int(box.y2());

						jobj_box["location"] = jarray_points;
						jobj_box["score"] = Json::Value(box.score());
						jobj_box["category"] = Json::Int(box.category());

						Json::Value jarray_key_points = Json::Value(Json::arrayValue);
						auto key_points = box.key_points();

						for (size_t i = 0; i < (int)key_points.size() / 3; i++) {
							Json::Value KPoint;
							KPoint["x"] = Json::Int(key_points[i * 3]);
							KPoint["y"] = Json::Int(key_points[i * 3 + 1]);
							KPoint["point_score"] = Json::Value(key_points[i * 3 + 2]);
							jarray_key_points.append(KPoint);
						}
						jobj_box["key_points"] = jarray_key_points;
						jarray_boxes.append(jobj_box);
					}

					value["info_list"] = jarray_boxes;

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

			inline Json::Value Trespass_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"trespass.delete", param);

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

			inline Json::Value Flame_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try {
					int device = root["device"].asInt();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)},
								{u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"flame.new", param));
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

			inline Json::Value Flame_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });
					
					auto version =plugin.execute(u8"flame.version", param) ;

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

			inline Json::Value Flame_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();

					int roi_x = root["roi_x"].asInt();
					int roi_y = root["roi_y"].asInt();

					int roi_width  = root["roi_width"].asInt();
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
							{u8"object_id", box(instance)},

							{u8"roi_x", box(roi_x)},
							{u8"roi_y", box(roi_y)},

							{u8"roi_width",  box(roi_width)},
							{u8"roi_height", box(roi_height)},
							{u8"params", param_map_abi},
						});

					auto result = plugin.execute(u8"flame.detect", param).as<exposing::param_vector<flame::box_info>>();

					Json::Value jarray_box;
					Json::Value jarray_fire_detected(Json::arrayValue);
					Json::Value jarray_smoke_detected(Json::arrayValue);

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
                            jarray_fire_detected.append(jarray_box);
						}
						else if (category == 0)
						{
							jarray_box["x1"] = Json::Int(result[i].x1());
							jarray_box["y1"] = Json::Int(result[i].y1());
							jarray_box["x2"] = Json::Int(result[i].x2());
							jarray_box["y2"] = Json::Int(result[i].y2());
							jarray_box["score"] = Json::Value(result[i].score());
                            jarray_smoke_detected.append(jarray_box);
						}
					}

					Json::Value jarray_info;

					jarray_info["fire_list"] = jarray_fire_detected;
					jarray_info["smoke_list"] = jarray_smoke_detected;

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

			
			inline Json::Value Flame_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"flame.delete", param);

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

			inline Json::Value Sleep_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try {
					int device = root["device"].asInt();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)},
								{u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"sleep.new", param));
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

			inline Json::Value Sleep_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();
					int roi_x = root["roi_x"].asInt();
					int roi_y = root["roi_y"].asInt();
					int roi_width  = root["roi_width"].asInt();
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
							{u8"object_id", box(instance)},

							{u8"roi_x", box(roi_x)},
							{u8"roi_y", box(roi_y)},
							{u8"roi_width",  box(roi_width)},
							{u8"roi_height", box(roi_height)},
							{u8"params", param_map_abi},
						});

					auto result = plugin.execute(u8"sleep.detect", param).as<exposing::param_vector<sleep::box_info>>();

					int work_detected = 0;
					int lying_detected = 0;
					int desk_detected = 0;
					int standing_detected = 0;

					Json::Value jarray_box;
					Json::Value jarray_work_detected(Json::arrayValue);
					Json::Value jarray_lying_detected(Json::arrayValue);
					Json::Value jarray_desk_detected(Json::arrayValue);
					Json::Value jarray_standing_detected(Json::arrayValue);

					for (int i = 0; i < result.size(); i++)
					{
						int category = Json::Int(result[i].category());
						
						// Json::Value jarray_box;
						if (category == 0)
						{
							jarray_box["x1"] = Json::Int(result[i].x1());
							jarray_box["y1"] = Json::Int(result[i].y1());
							jarray_box["x2"] = Json::Int(result[i].x2());
							jarray_box["y2"] = Json::Int(result[i].y2());
							jarray_box["score"] = Json::Value(result[i].confidence());
                            jarray_work_detected.append(jarray_box);
						}
						else if (category == 1)
						{
							jarray_box["x1"] = Json::Int(result[i].x1());
							jarray_box["y1"] = Json::Int(result[i].y1());
							jarray_box["x2"] = Json::Int(result[i].x2());
							jarray_box["y2"] = Json::Int(result[i].y2());
							jarray_box["score"] = Json::Value(result[i].confidence());
                            jarray_lying_detected.append(jarray_box);
						}
						else if (category == 2)
						{
							jarray_box["x1"] = Json::Int(result[i].x1());
							jarray_box["y1"] = Json::Int(result[i].y1());
							jarray_box["x2"] = Json::Int(result[i].x2());
							jarray_box["y2"] = Json::Int(result[i].y2());
							jarray_box["score"] = Json::Value(result[i].confidence());
                            jarray_desk_detected.append(jarray_box);
						}
						else if (category == 3)
						{
							jarray_box["x1"] = Json::Int(result[i].x1());
							jarray_box["y1"] = Json::Int(result[i].y1());
							jarray_box["x2"] = Json::Int(result[i].x2());
							jarray_box["y2"] = Json::Int(result[i].y2());
							jarray_box["score"] = Json::Value(result[i].confidence());
                            jarray_standing_detected.append(jarray_box);
						}
					}

					Json::Value jarray_info;


					jarray_info["work_list"] = jarray_work_detected;

					jarray_info["lying_list"] = jarray_lying_detected;

					jarray_info["desk_list"] = jarray_desk_detected;
					
					jarray_info["standing_list"] = jarray_standing_detected;

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
			
			inline Json::Value Sleep_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"sleep.delete", param);

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

			inline Json::Value Smoke_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try {
					int device = root["device"].asInt();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)},
								{u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"smoke.new", param));
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


			inline Json::Value Smoke_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });
					
					auto version =plugin.execute(u8"smoke.version", param) ;

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


			inline Json::Value Smoke_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();

					int roi_x = root["roi_x"].asInt();
					int roi_y = root["roi_y"].asInt();

					int roi_width  = root["roi_width"].asInt();
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
							{u8"object_id", box(instance)},

							{u8"roi_x", box(roi_x)},
							{u8"roi_y", box(roi_y)},

							{u8"roi_width",  box(roi_width)},
							{u8"roi_height", box(roi_height)},
							{u8"params", param_map_abi},
						});

					auto result = plugin.execute(u8"smoke.detect", param).as<exposing::param_vector<smoke::box_info>>();

					int work_detected = 0;
					int lying_detected = 0;
					int desk_detected = 0;
					int standing_detected = 0;

					Json::Value jarray_box;

					Json::Value jarray_normal_detected(Json::arrayValue);
					Json::Value jarray_smoke_detected(Json::arrayValue);
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
							// jarray_box["label"] = Json::Int(result[i].category());
							jarray_box["score"] = Json::Value(result[i].confidence());
                            jarray_normal_detected.append(jarray_box);
						}
						else if (category == 0)
						{

							jarray_box["x1"] = Json::Int(result[i].x1());
							jarray_box["y1"] = Json::Int(result[i].y1());
							jarray_box["x2"] = Json::Int(result[i].x2());
							jarray_box["y2"] = Json::Int(result[i].y2());
							// jarray_box["label"] = Json::Int(result[i].category());
							jarray_box["score"] = Json::Value(result[i].confidence());
                            jarray_smoke_detected.append(jarray_box);
						}					
					}

					Json::Value jarray_info;

					jarray_info["norm_list"] = jarray_normal_detected;
					jarray_info["smoke_list"] = jarray_smoke_detected;

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
			
			inline Json::Value Smoke_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"smoke.delete", param);

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

			inline Json::Value Onphone_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try {
					int device = root["device"].asInt();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)},
								{u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"onphone.new", param));
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

			inline Json::Value Onphone_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();

					int roi_x = root["roi_x"].asInt();
					int roi_y = root["roi_y"].asInt();

					int roi_width  = root["roi_width"].asInt();
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
							{u8"object_id", box(instance)},

							{u8"roi_x", box(roi_x)},
							{u8"roi_y", box(roi_y)},

							{u8"roi_width",  box(roi_width)},
							{u8"roi_height", box(roi_height)},
							{u8"params", param_map_abi},
						});

					auto result = plugin.execute(u8"onphone.detect", param).as<exposing::param_vector<onphone::box_info>>();

					int work_detected = 0;
					int lying_detected = 0;
					int desk_detected = 0;
					int standing_detected = 0;

					Json::Value jarray_box;
					Json::Value jarray_normal_detected;
					Json::Value jarray_onphone_detected;

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
	
							jarray_box["score"] = Json::Value(result[i].confidence());
                            jarray_normal_detected.append(jarray_box);
						}
						else if (category == 0)
						{  
							jarray_box["x1"] = Json::Int(result[i].x1());
							jarray_box["y1"] = Json::Int(result[i].y1());
							jarray_box["x2"] = Json::Int(result[i].x2());
							jarray_box["y2"] = Json::Int(result[i].y2());
							//jarray_box["label"] = Json::Int(result[i].category());
							jarray_box["score"] = Json::Value(result[i].confidence());
                            jarray_onphone_detected.append(jarray_box);
						}					
					}

					Json::Value jarray_info;

					jarray_info["norm_list"] = jarray_normal_detected;
					jarray_info["onphone_list"] = jarray_onphone_detected;

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
			
			inline Json::Value Onphone_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"onphone.delete", param);

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

			inline Json::Value Sleep_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });
					
					auto version =plugin.execute(u8"sleep.version", param) ;

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

			inline Json::Value Onphone_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });
					
					auto version =plugin.execute(u8"onphone.version", param) ;

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

			inline Json::Value Gungnir_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int device = root["device"].asInt();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)},
						 {u8"models_directory", box(std::string_view(models_directory))} });
					instance = unbox<guid>(plugin.execute(u8"gungnir.new", param));
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

			inline Json::Value Gungnir_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"gungnir.delete", param);

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

			inline Json::Value Gungnir_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

					auto result = plugin.execute(u8"gungnir.detect", param).as<param_vector<gungnir::hat_info>>();

					Json::Value jarray_rect = Json::Value(Json::arrayValue);

					for (auto obj : result)
					{
						Json::Value jobj_face;
						jobj_face["x"] = Json::Int(obj.x());
						jobj_face["y"] = Json::Int(obj.y());
						jobj_face["width"] = Json::Int(obj.width());
						jobj_face["height"] = Json::Int(obj.height());
						jobj_face["prob"] = Json::Value(obj.prob());
						jobj_face["label"] = Json::Int(obj.label());

						jarray_rect.append(jobj_face);
					}

					value["hatrectwithhatinfo_list"] = jarray_rect;
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

			inline Json::Value Refvest_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try {
					std::string models_directory = root["models_directory"].asString();
					int device = root["device"].asInt();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)}, {u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"refvest.new", param));
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

			inline Json::Value Refvest_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });
					
					auto version =plugin.execute(u8"refvest.version", param) ;

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


			inline Json::Value Refvest_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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
					int channels =3;

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
						});

					auto result = plugin.execute(u8"refvest.detect", param).as<param_vector<refvest::box_info>>();
					Json::Value jarray_box;
					Json::Value jarray_offvest_detected(Json::arrayValue);
					Json::Value jarray_withvest_detected(Json::arrayValue);

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
                            jarray_withvest_detected.append(jarray_box);
						}
						else if (category == 0)
						{
							jarray_box["x1"] = Json::Int(result[i].x1());
							jarray_box["y1"] = Json::Int(result[i].y1());
							jarray_box["x2"] = Json::Int(result[i].x2());
							jarray_box["y2"] = Json::Int(result[i].y2());
							jarray_box["score"] = Json::Value(result[i].score());
                            jarray_offvest_detected.append(jarray_box);
						}
					}

					Json::Value jarray_info;

					jarray_info["with_refvtst_list"] = jarray_withvest_detected;
					jarray_info["without_refvtst_list"] = jarray_offvest_detected;

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

			inline Json::Value Refvest_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"refvest.delete", param);

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

			inline Json::Value Rail_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try {
					int device = root["device"].asInt();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)},
								{u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"rail.new", param));
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

			inline Json::Value Rail_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(image_span)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"object_id", box(instance)},
							{u8"params", param_map_abi},
						});

					auto result = plugin.execute(u8"rail.detect", param).as<exposing::param_vector<rail::box_info>>();

					// boxes
					Json::Value jarray_info;
					int len = result.size();

					jarray_info["boxes_num"] = len;

					Json::Value jarray_boxes;
					for (int i = 0; i < len; i++)
					{
						// boxes
						Json::Value jarray_box;
						jarray_box["x1"] = Json::Int(result[i].x1());
						jarray_box["y1"] = Json::Int(result[i].y1());
						jarray_box["x2"] = Json::Int(result[i].x2());
						jarray_box["y2"] = Json::Int(result[i].y2());
						jarray_box["category"] = Json::Int(result[i].category());
						jarray_boxes.append(jarray_box);
					}

					jarray_info["boxes_list"] = jarray_boxes;

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

			inline Json::Value Rail_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"rail.delete", param);

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

			inline Json::Value Plate_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

			constexpr int plate_aligned_base64_buffer_len = TB64ENCLEN(3 * 140 * 440);
			inline Json::Value Plate_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

			inline Json::Value Plate_trace_init_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

			inline Json::Value Plate_trace_update_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

			inline Json::Value Plate_recognize_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

			inline Json::Value Plate_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

			inline Json::Value Ring_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try {
					int device = root["device"].asInt();
					int factory_type = root["factory_type"].asInt();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)},
							{u8"factory_type", box(factory_type)},
							{u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"ring.new", param));
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

			inline Json::Value Ring_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();
					int border_orient = root["border_orient"].asInt();

					Json::Value roi = root.get("roi", Json::Value());
					bool flag = roi.empty();
					int x = flag ? 0 : roi["x"].asInt();
					int y = flag ? 0 : roi["y"].asInt();
					int roi_width = flag ? width : roi["width"].asInt();
					int roi_height = flag ? height : roi["height"].asInt();

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
							{u8"border_orient", box(border_orient)},
							{u8"order", box(static_cast<int>(frame->format_))},
							{u8"object_id", box(instance)},
							{u8"x", box(x)},
							{u8"y", box(y)},
							{u8"roi_width", box(roi_width)},
							{u8"roi_height", box(roi_height)},
							{u8"params", param_map_abi},
						});

					auto result = plugin.execute(u8"ring.detect", param).as<param_vector<ring::box_info>>();
					Json::Value jarray_boxes = Json::Value(Json::arrayValue);
					for (auto box : result)
					{
						Json::Value jobj_box;
						Json::Value jarray_points = Json::Value(Json::arrayValue);
						// location
						auto location = box.location();
						for (size_t i = 0; i < (int)location.size() / 2; i++)
						{
							Json::Value point;
							point["x"] = Json::Int(location[i * 2]);
							point["y"] = Json::Int(location[i * 2 + 1]);

							jarray_points.append(point);
						}
						jobj_box["location"] = jarray_points;
						// strinfos
						Json::Value jarray_strinfos = Json::Value(Json::arrayValue);
						for (auto strinfo : box.strinfos())
						{
							jarray_strinfos.append(Json::Value(exposing::to_narrow_string(strinfo)));
						}
						jobj_box["strinfo"] = jarray_strinfos;
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
						// cut_roi
						//jobj_box["cut_roi"] = Json::Value(Json::arrayValue);
						//auto cut_roi = box.cut_roi();
						//std::vector<std::uint8_t> temp(TB64ENCLEN(cut_roi.size()), 0);
						//std::uint8_t* ptr = temp.data();
						//std::vector<std::uint8_t> buffer(cut_roi.size());
						//cut_roi.copy_to(0, buffer);
						//size_t base64_len = tb64xenc(buffer.data(), buffer.size(), ptr);
						//jobj_box["cut_roi"].append(Json::Value(reinterpret_cast<char*>(ptr), reinterpret_cast<char*>(ptr) + base64_len));
						//// cut_roi_width
						//jobj_box["cut_roi_width"] = Json::Value(box.cut_roi_width());
						//// cut_roi_height
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

			inline Json::Value Ring_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"ring.delete", param);

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

			inline Json::Value Longinus_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int device = root["device"].asInt();
					//float nms = static_cast<float>(root["nms"].asDouble());
					float nms = 0.4f;
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)},
						 {u8"nms", box(nms)},
						 {u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"longinus.new", param));
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

			inline Json::Value Longinus_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"longinus.delete", param);

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

			inline Json::Value Longinus_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();
					int min_size = root["min_size"].asInt();
					float threshold = root["threshold"].asFloat();
					bool do_attributing = root["do_attributing"].asBool();

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"image", box(image_span)},
						 {u8"height", box(height)},
						 {u8"width", box(width)},
						 {u8"min_size", box(min_size)},
						 {u8"threshold", box(threshold)},
						 {u8"order", box(static_cast<int>(frame->format_))},
						 {u8"do_attributing", box(do_attributing)},
						 {u8"object_id", box(instance)} });

					auto result = plugin.execute(u8"longinus.detect", param).as<param_vector<longinus::face_info>>();

					Json::Value jarray_rect = Json::Value(Json::arrayValue);

					for (auto obj : result)
					{
						Json::Value jobj_face;
						jobj_face["x"] = Json::Int(obj.x());
						jobj_face["y"] = Json::Int(obj.y());
						jobj_face["width"] = Json::Int(obj.width());
						jobj_face["height"] = Json::Int(obj.height());
						jobj_face["confidence"] = Json::Value(obj.confidence());

						if (do_attributing)
						{
							jobj_face["attributes"]["yaw"] = Json::Value(obj.yaw());
							jobj_face["attributes"]["pitch"] = Json::Value(obj.pitch());
							jobj_face["attributes"]["roll"] = Json::Value(obj.roll());
							jobj_face["attributes"]["glass_index"] = Json::Int(obj.glass_index());
							jobj_face["attributes"]["mask_index"] = Json::Int(obj.mask_index());
						}
						else
							jobj_face["attributes"] = Json::Value(Json::nullValue);

						Json::Value jarray_landmark;

						for (const auto& pt : obj.pts())
						{

							Json::Value jobj_point;
							jobj_point["x"] = Json::Int((int)pt.key());
							jobj_point["y"] = Json::Int((int)pt.value());
							jarray_landmark.append(jobj_point);
						}
						jobj_face["landmark"] = jarray_landmark;
						jarray_rect.append(jobj_face);
					}

					value["facerectwithfaceinfo_list"] = jarray_rect;
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

			inline Json::Value Longinus_trace_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();
					auto face = make_exported_interface<longinus::face_info>();
					face.set_x(root["face"]["x"].asFloat());
					face.set_y(root["face"]["y"].asFloat());
					face.set_width(root["face"]["width"].asFloat());
					face.set_height(root["face"]["height"].asFloat());

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"image", box(image_span)},
						 {u8"height", box(height)},
						 {u8"width", box(width)},
						 {u8"face", face},
						 {u8"order", box(static_cast<int>(frame->format_))},
						 {u8"object_id", box(instance)} });
					auto result = plugin.execute(u8"longinus.trace", param).as<longinus::face_info>();

					Json::Value jobj_face;
					if (result.confidence() > 0.1f)
					{
						value["trace_success"] = Json::Value(true);
						jobj_face["x"] = Json::Int(result.x());
						jobj_face["y"] = Json::Int(result.y());
						jobj_face["width"] = Json::Int(result.width());
						jobj_face["height"] = Json::Int(result.height());
						jobj_face["confidence"] = Json::Value(result.confidence());

						jobj_face["attributes"]["glass_index"] = Json::Int(result.glass_index());
						jobj_face["attributes"]["mask_index"] = Json::Int(result.mask_index());
						jobj_face["attributes"]["yaw"] = Json::Value(result.yaw());
						jobj_face["attributes"]["pitch"] = Json::Value(result.pitch());
						jobj_face["attributes"]["roll"] = Json::Value(result.roll());

						Json::Value jarray_landmark;

						for (const auto& pt : result.pts())
						{

							Json::Value jobj_point;
							jobj_point["x"] = Json::Int((int)pt.key());
							jobj_point["y"] = Json::Int((int)pt.value());
							jarray_landmark.append(jobj_point);
						}
						jobj_face["landmark"] = jarray_landmark;
					}
					else
					{
						value["trace_success"] = Json::Value(false);
					}

					value["facerectwithfaceinfo"] = jobj_face;
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

			constexpr int longinus_align_aligned_base64_buffer_len = TB64ENCLEN(3 * 128 * 128);
			inline Json::Value Longinus_center_scale_alignFace_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					bool save2extrenal = false;
					if (external.size() >= root["image_infos"].size() * 128 * 128 * 3)
						save2extrenal = true;

					value["aligned_images"] = Json::Value(Json::arrayValue);

					int read_byte_size = 0;
					for (size_t i = 0; i < root["image_infos"].size(); i++)
					{
						int format = root["image_infos"][Json::Int(i)]["format"].asInt();
						int height = root["image_infos"][Json::Int(i)]["height"].asInt();
						int width = root["image_infos"][Json::Int(i)]["width"].asInt();
						float scale = root["image_infos"][Json::Int(i)]["scale"].asFloat();
						int image_byte_size = root["image_infos"][Json::Int(i)]["image_byte_size"].asInt();

						param_span<std::uint8_t> data_i(data.data() + read_byte_size, image_byte_size);
						read_byte_size += image_byte_size;
						auto frame = decode_and_convert(data_i, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
						param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

						auto param = make_param_hash_map<param_string, unknown_object>(
							{ {u8"image", box(image_span)},
							 {u8"height", box(height)},
							 {u8"width", box(width)},
							 {u8"scale", box(scale)},
							 {u8"order", box(static_cast<int>(frame->format_))},
							 {u8"object_id", box(instance)} });

						auto result = plugin.execute(u8"longinus.center_scale_alignFace", param).as<param_vector<param_vector<std::uint8_t>>>();
						if (save2extrenal)
						{
							result[0].copy_to(0, { external.data() + i * 128 * 128 * 3, 128 * 128 * 3 });
						}
						else
						{
							std::vector<std::uint8_t> temp(longinus_align_aligned_base64_buffer_len, 0);
							std::uint8_t* ptr = temp.data();
							std::vector<std::uint8_t> buffer(128 * 128 * 3);
							result[0].copy_to(0, buffer);

							tb64xenc(buffer.data(), buffer.size(), ptr);

							value["aligned_images"].append(Json::Value(reinterpret_cast<char*>(ptr), reinterpret_cast<char*>(ptr) + longinus_align_aligned_base64_buffer_len));
						}
					}

					value["format"] = Json::Value(0);
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

		

			inline Json::Value Leavepost_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{	

					int device = root["device"].asInt();
						printf("device:%d\n",device);
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)},
						 {u8"models_directory", box(std::string_view(models_directory))} });


					instance = unbox<guid>(plugin.execute(u8"leavepost.new", param));
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

			inline Json::Value Leavepost_version_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
					{ {u8"object_id", box(instance)} });
					
					auto version =plugin.execute(u8"leavepost.version", param);

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


			inline Json::Value Leavepost_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"leavepost.delete", param);

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

			inline Json::Value Leavepost_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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
					int channels =3;

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
						});

					auto result = plugin.execute(u8"leavepost.detect", param).as<param_vector<leavepost::box_info>>();
					Json::Value jarray_box;
					Json::Value jarray_info;
					Json::Value jarray_work = Json::Value(Json::arrayValue);
					Json::Value jarray_leave = Json::Value(Json::arrayValue);

					for (auto obj : result)
					{
						int category = Json::Int(obj.label());
						if(category==1)
						{
							jarray_box["x1"] = Json::Int(obj.x());
							jarray_box["y1"] = Json::Int(obj.y());
							jarray_box["x2"] = Json::Int(obj.x()+obj.width());
							jarray_box["y2"] = Json::Int(obj.height()+obj.y());
							jarray_box["score"] = Json::Value(obj.confidence());
							jarray_work.append(jarray_box);
						}

					}
					jarray_info["hat_list"] = jarray_work;

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

			inline Json::Value Mjollner_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

			inline Json::Value Mjollner_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

			inline Json::Value Mjollner_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

			inline Json::Value Valklyrs_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

			inline Json::Value Valklyrs_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

			inline Json::Value Valklyrs_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

			inline Json::Value Heimdall_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

			inline Json::Value Heimdall_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

			inline Json::Value Heimdall_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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

			inline Json::Value Banshee_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>();
					instance = unbox<guid>(plugin.execute(u8"banshee.new", param));
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

			inline Json::Value Banshee_init_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int format = root["format"].asInt();
					int width = root["width"].asInt();
					int height = root["height"].asInt();
					Json::Value roi = root.get("roi", Json::Value());
					int x = roi["x"].asInt();
					int y = roi["y"].asInt();
					int roi_width = roi["width"].asInt();
					int roi_height = roi["height"].asInt();
					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(image_span)},
							{u8"width", box(width)},
							{u8"height", box(height)},
							{u8"x", box(x)},
							{u8"y", box(y)},
							{u8"roi_width", box(roi_width)},
							{u8"roi_height", box(roi_height)},
							{u8"object_id", box(instance)},
						});
					plugin.execute(u8"banshee.init", param);
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

			inline Json::Value Banshee_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"banshee.delete", param);

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

			inline Json::Value Banshee_update_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
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
						{
							{u8"image", box(image_span)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"object_id", box(instance)},
						});

					auto result = plugin.execute(u8"banshee.update", param).as<banshee::track_info>();

					Json::Value jobj = Json::Value();
					jobj["x"] = Json::Value(result.x());
					jobj["y"] = Json::Value(result.y());
					jobj["width"] = Json::Value(result.width());
					jobj["height"] = Json::Value(result.height());
					jobj["prob"] = Json::Value(result.prob());

					value["result"] = jobj;
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

			inline Json::Value Romancia_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int device = root["device"].asInt();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)},
						 {u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"romancia.new", param));
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

			inline Json::Value Romancia_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"romancia.delete", param);

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

			constexpr int romancia_align_aligned_base64_buffer_len = TB64ENCLEN(3 * 128 * 128);
			inline Json::Value Romancia_alignFace_128_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();
					auto jarray_rect = root["facerectwithfaceinfo_list"];
					auto faces = exposing::make_param_vector<longinus::face_info>();
					for (auto i : jarray_rect)
					{
						auto face = exposing::make_exported_interface<longinus::face_info>();
						face.set_x(i["x"].asFloat());
						face.set_y(i["y"].asFloat());
						face.set_height(i["height"].asFloat());
						face.set_width(i["width"].asFloat());

						auto landmark_list = i["landmark"];
						if (landmark_list.size() != 5)
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "landmark_list.size() != 5");

						auto landmark = exposing::make_param_vector<exposing::param_pair<float, float>>();
						for (auto j : landmark_list)
						{
							auto pair = exposing::make_param_pair(j["x"].asFloat(), j["y"].asFloat());
							landmark.push_back(pair);
						}
						face.set_pts(landmark);

						faces.push_back(face);
					}

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"image", box(image_span)},
						 {u8"height", box(height)},
						 {u8"width", box(width)},
						 {u8"faces", faces},
						 {u8"order", box(static_cast<int>(frame->format_))},
						 {u8"object_id", box(instance)} });

					auto result = plugin.execute(u8"romancia.alignFace128", param).as<param_vector<param_vector<std::uint8_t>>>();

					value["aligned_images"] = Json::Value(Json::arrayValue);
					size_t total_size = 0;
					for (const auto& x : result)
						total_size += x.size();
					if (external.size() >= total_size)
					{
						size_t coped_size = 0;
						for (const auto& x : result)
						{
							x.copy_to(0, { external.data() + coped_size, static_cast<size_t>(x.size()) });
							coped_size += x.size();
						}
					}
					else
					{
						std::vector<std::uint8_t> temp(romancia_align_aligned_base64_buffer_len, 0);
						std::uint8_t* ptr = temp.data();
						for (size_t i = 0; i < result.size(); i++)
						{
							std::vector<std::uint8_t> buffer(result[i].size());
							result[i].copy_to(0, buffer);

							tb64xenc(buffer.data(), buffer.size(), ptr);

							value["aligned_images"].append(Json::Value(reinterpret_cast<char*>(ptr), reinterpret_cast<char*>(ptr) + romancia_align_aligned_base64_buffer_len));
						}
					}
					value["format"] = Json::Value(0);
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

			inline Json::Value Romancia_alignFace_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();
					auto jarray_rect = root["facerectwithfaceinfo_list"];
					auto faces = exposing::make_param_vector<longinus::face_info>();
					for (auto i : jarray_rect)
					{
						auto face = exposing::make_exported_interface<longinus::face_info>();
						face.set_x(i["x"].asFloat());
						face.set_y(i["y"].asFloat());
						face.set_height(i["height"].asFloat());
						face.set_width(i["width"].asFloat());

						auto landmark_list = i["landmark"];
						if (landmark_list.size() != 5)
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "landmark_list.size() != 5");

						auto landmark = exposing::make_param_vector<exposing::param_pair<float, float>>();
						for (auto j : landmark_list)
						{
							auto pair = exposing::make_param_pair(j["x"].asFloat(), j["y"].asFloat());
							landmark.push_back(pair);
						}
						face.set_pts(landmark);

						faces.push_back(face);
					}

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"image", box(image_span)},
						 {u8"height", box(height)},
						 {u8"width", box(width)},
						 {u8"faces", faces},
						 {u8"order", box(static_cast<int>(frame->format_))},
						 {u8"object_id", box(instance)} });

					auto result = plugin.execute(u8"romancia.alignFace", param).as<param_vector<param_vector<std::uint8_t>>>();

					value["aligned_images"] = Json::Value(Json::arrayValue);
					size_t total_size = 0;
					for (const auto& x : result)
						total_size += x.size();
					if (external.size() >= total_size)
					{
						size_t coped_size = 0;
						for (const auto& x : result)
						{
							x.copy_to(0, { external.data() + coped_size, static_cast<size_t>(x.size()) });
							coped_size += x.size();
						}
					}
					else
					{
						std::vector<std::uint8_t> temp(romancia_align_aligned_base64_buffer_len, 0);
						std::uint8_t* ptr = temp.data();
						for (size_t i = 0; i < result.size(); i++)
						{
							std::vector<std::uint8_t> buffer(result[i].size());
							result[i].copy_to(0, buffer);

							tb64xenc(buffer.data(), buffer.size(), ptr);

							value["aligned_images"].append(Json::Value(reinterpret_cast<char*>(ptr), reinterpret_cast<char*>(ptr) + romancia_align_aligned_base64_buffer_len));
						}
					}
					
					value["format"] = Json::Value(0);
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

			inline Json::Value Romancia_blur_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();
					Json::Value facerect_list = root["facerect_list"];
					auto faces = make_param_vector<longinus::face_info>();
					for (auto& i : facerect_list)
					{
						auto face = make_exported_interface<longinus::face_info>();
						face.set_x(i["x"].asFloat());
						face.set_y(i["y"].asFloat());
						face.set_width(i["width"].asFloat());
						face.set_height(i["height"].asFloat());
						faces.push_back(face);
					}

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"image", box(image_span)},
						 {u8"height", box(height)},
						 {u8"width", box(width)},
						 {u8"faces", faces},
						 {u8"order", box(static_cast<int>(frame->format_))},
						 {u8"object_id", box(instance)} });

					auto result = plugin.execute(u8"romancia.blur_detect", param).as<param_vector<float>>();

					Json::Value clarity = Json::Value(Json::arrayValue);
					for (auto i : result)
						clarity.append(i);
					value["clarity"] = Json::Value(clarity);
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

			inline Json::Value Romancia_mask_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();
					auto jarray_rect = root["facerectwithfaceinfo_list"];
					auto faces = exposing::make_param_vector<longinus::face_info>();
					for (auto& i : jarray_rect)
					{
						auto face = exposing::make_exported_interface<longinus::face_info>();
						face.set_x(i["x"].asFloat());
						face.set_y(i["y"].asFloat());
						face.set_height(i["height"].asFloat());
						face.set_width(i["width"].asFloat());

						auto landmark_list = i["landmark"];
						if (landmark_list.size() != 5)
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "landmark_list.size() != 5");

						auto landmark = exposing::make_param_vector<exposing::param_pair<float, float>>();
						for (auto& j : landmark_list)
						{
							auto pair = exposing::make_param_pair(j["x"].asFloat(), j["y"].asFloat());
							landmark.push_back(pair);
						}
						face.set_pts(landmark);

						faces.push_back(face);
					}

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"image", box(image_span)},
						 {u8"height", box(height)},
						 {u8"width", box(width)},
						 {u8"faces", faces},
						 {u8"order", box(static_cast<int>(frame->format_))},
						 {u8"object_id", box(instance)} });

					Json::Value mask_value = Json::Value(Json::arrayValue);
					auto result = plugin.execute(u8"romancia.mask_detect", param).as<param_vector<double>>();
					for (auto i : result)
						mask_value.append(i);

					value["mask_value"] = Json::Value(mask_value);
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

            inline Json::Value Romancia_rotate_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    int format = root["format"].asInt();
                    int height = root["height"].asInt();
                    int width = root["width"].asInt();
                    float angle = root["angle"].asFloat();

                    auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
                    param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

                    auto param = make_param_hash_map<param_string, unknown_object>(
                        { {u8"image", box(image_span)},
                         {u8"height", box(height)},
                         {u8"width", box(width)},
                         {u8"angle", box(angle)},
                         {u8"order", box(static_cast<int>(frame->format_))},
                         {u8"object_id", box(instance)} });

                    auto result = plugin.execute(u8"romancia.rotate", param).as<param_vector<std::uint8_t>>();
                    if (external.size() >= result.size())
                    {
                        //std::copy(exposing::begin(result), exposing::end(result), external.begin());
						result.copy_to(0, { external.data(), static_cast<size_t>(result.size()) });
						value["status"]["message"] = Json::Value("OK");
						value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::NO_EXCEPTION));
                    }
					else
					{
						value["status"]["message"] = Json::Value("external.size() not match");
						value["status"]["code"] = Json::Value(static_cast<int>(parser_exception::parser_exception_code::INVALID_OPERATION));
					}
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

			inline Json::Value Gaius_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int device = root["device"].asInt();
					bool use_int8 = root["use_int8"].asBool();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)},
						 {u8"use_int8", box(use_int8 ? 1 : 0)},
						 {u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"gaius.new", param));
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
			inline Json::Value Gaius_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"gaius.delete", param);

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

			constexpr int gaius_forward_aligned_buffer_len = 3 * 128 * 128;
			inline Json::Value Gaius_forward_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					int format = root["format"].asInt();
					bool has_mask = root["has_mask"].asBool();
					if (format < 0 || format > 1)
						throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: format < 0 || format > 1");

					int num = 0;
					param_vector<param_vector<float>> result;
					if (data.size() && (data.size() % gaius_forward_aligned_buffer_len == 0))
					{
						num = data.size() / gaius_forward_aligned_buffer_len;
						auto param = make_param_hash_map<param_string, unknown_object>(
							{ {u8"aligned_faces", box(data)},
							 {u8"num", box(num)},
							 {u8"order", box(format)},
							 {u8"has_mask", box(has_mask ? 1 : 0)},
							 {u8"object_id", box(instance)} });

						result = plugin.execute(u8"gaius.forward", param).as<param_vector<param_vector<float>>>();
					}
					else
					{
						auto aligned_face_array = root["aligned_images"];
						std::vector<uint8_t> aligned_faces_vec;
						std::vector<uint8_t> temp(gaius_forward_aligned_buffer_len, 0);
						std::uint8_t* ptr = temp.data();
						for (auto i : aligned_face_array)
						{
							std::string aligned_face_base64_str = i.asString();
							if (aligned_face_base64_str.size() != TB64ENCLEN(gaius_forward_aligned_buffer_len))
								throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: aligned_face_base64_str.size() != TB64ENCLEN(gaius_forward_aligned_buffer_len)");

							size_t aligned_face_decode_len = tb64xdec(reinterpret_cast<const std::uint8_t*>(aligned_face_base64_str.data()), aligned_face_base64_str.size(), ptr);
							if (aligned_face_decode_len != gaius_forward_aligned_buffer_len)
								throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "aligned_face_decode_len != gaius_forward_aligned_buffer_len");

							aligned_faces_vec.insert(aligned_faces_vec.end(), temp.begin(), temp.end());
							num++;
						}

						auto param = make_param_hash_map<param_string, unknown_object>(
							{ {u8"aligned_faces", box(param_span<std::uint8_t>{aligned_faces_vec})},
							 {u8"num", box(num)},
							 {u8"order", box(format)},
							 {u8"has_mask", box(has_mask ? 1 : 0)},
							 {u8"object_id", box(instance)} });

						result = plugin.execute(u8"gaius.forward", param).as<param_vector<param_vector<float>>>();
					}

					Json::Value jobj_features = Json::Value(Json::arrayValue);
					if (external.size() >= num * 128 * sizeof(float))
					{
						for (size_t i = 0; i < result.size(); i++)
							result[i].copy_to(0, { reinterpret_cast<float*>(external.data()) + i * 128, 128 });
					}
					else
					{
						for (size_t i = 0; i < result.size(); i++)
						{
							Json::Value jarray_feature;
							for (size_t j = 0; j < result[i].size(); j++)
								jarray_feature["feature"].append(result[i][j]);
							jobj_features.append(jarray_feature);
						}
					}

					value["features"] = jobj_features;
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

			inline Json::Value Gaius_make_mask_forward_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					int format = root["format"].asInt();
					if (format < 0 || format > 1)
						throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: format < 0 || format > 1");

					std::uint8_t mask[64 * 128] = { 0 };
					int num = 0;
					param_vector<param_vector<float>> result;
					if (data.size() && (data.size() % gaius_forward_aligned_buffer_len == 0))
					{
						num = data.size() / gaius_forward_aligned_buffer_len;
						std::vector<std::uint8_t> temp(num * gaius_forward_aligned_buffer_len, 0);
						std::copy(data.begin(), data.end(), temp.begin());
						std::uint8_t* ptr = temp.data();
						if (format == 0)
						{
							for (int i = 0; i < num; i++)
							{
								for (int j = 0; j < 3; j++)
								{
									std::copy(mask, mask + 64 * 128, ptr + i * gaius_forward_aligned_buffer_len + j * 128 * 128 + 64 * 128);
								}
							}
						}
						else
						{
							for (int i = 0; i < num; i++)
							{
								for (int j = 0; j < 3; j++)
								{
									std::copy(mask, mask + 64 * 128, ptr + i * gaius_forward_aligned_buffer_len + 3 * 64 * 128 + j * 64 * 128);
								}
							}
						}

						auto param = make_param_hash_map<param_string, unknown_object>(
							{ {u8"aligned_faces", box(param_span<std::uint8_t>{temp})},
							 {u8"num", box(num)},
							 {u8"order", box(format)},
							 {u8"has_mask", box(1)},
							 {u8"object_id", box(instance)} });

						result = plugin.execute(u8"gaius.forward", param).as<param_vector<param_vector<float>>>();
					}
					else
					{
						auto aligned_face_array = root["aligned_images"];
						std::vector<std::uint8_t> temp(gaius_forward_aligned_buffer_len, 0);
						std::uint8_t* ptr = temp.data();
						std::vector<uint8_t> aligned_faces_vec;
						for (auto i : aligned_face_array)
						{
							std::string aligned_face_base64_str = i.asString();
							if (aligned_face_base64_str.size() != TB64ENCLEN(gaius_forward_aligned_buffer_len))
								throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: aligned_face_base64_str.size() != TB64ENCLEN(gaius_forward_aligned_buffer_len)");

							size_t aligned_face_decode_len = tb64xdec(reinterpret_cast<const std::uint8_t*>(aligned_face_base64_str.data()), aligned_face_base64_str.size(), ptr);
							if (aligned_face_decode_len != gaius_forward_aligned_buffer_len)
								throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "aligned_face_decode_len != gaius_forward_aligned_buffer_len");

							if (format == 0)
							{
								for (size_t j = 0; j < 3; j++)
								{
									std::copy(mask, mask + 64 * 128, ptr + j * 128 * 128 + 64 * 128);
								}
							}
							else
							{
								for (size_t j = 0; j < 3; j++)
								{
									std::copy(mask, mask + 64 * 128, ptr + 3 * 64 * 128 + j * 64 * 128);
								}
							}
							aligned_faces_vec.insert(aligned_faces_vec.end(), temp.begin(), temp.end());
							num++;
						}

						auto param = make_param_hash_map<param_string, unknown_object>(
							{ {u8"aligned_faces", box(param_span<std::uint8_t>{aligned_faces_vec})},
							 {u8"num", box(num)},
							 {u8"order", box(format)},
							 {u8"has_mask", box(1)},
							 {u8"object_id", box(instance)} });

						result = plugin.execute(u8"gaius.forward", param).as<param_vector<param_vector<float>>>();
					}

					Json::Value jobj_features = Json::Value(Json::arrayValue);
					if (external.size() >= num * gaius_forward_aligned_buffer_len * sizeof(float))
					{
						for (size_t i = 0; i < result.size(); i++)
							result[i].copy_to(0, { reinterpret_cast<float*>(external.data()) + i * gaius_forward_aligned_buffer_len, gaius_forward_aligned_buffer_len });
					}
					else
					{
						for (size_t i = 0; i < result.size(); i++)
						{
							Json::Value jarray_feature;
							for (size_t j = 0; j < result[i].size(); j++)
								jarray_feature["feature"].append(result[i][j]);
							jobj_features.append(jarray_feature);
						}
					}

					value["features"] = jobj_features;
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

			inline Json::Value Cassius_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int device = root["device"].asInt();
					int model_type = root["model_type"].asInt();
					bool use_int8 = root["use_int8"].asBool();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)},
						 {u8"model_type", box(model_type)},
						 {u8"use_int8", box(use_int8 ? 1 : 0)},
						 {u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"cassius.new", param));
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
			inline Json::Value Cassius_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"cassius.delete", param);

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

			constexpr int cassius_forward_aligned_buffer_len = 3 * 128 * 128;
			inline Json::Value Cassius_forward_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int format = root["format"].asInt();
					if (format < 0 || format > 1)
						throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: format < 0 || format > 1");

					int num = 0;
					param_vector<param_vector<float>> result;
					if (data.size() && (data.size() % cassius_forward_aligned_buffer_len == 0))
					{
						num = data.size() / cassius_forward_aligned_buffer_len;
						auto param = make_param_hash_map<param_string, unknown_object>(
							{ {u8"aligned_faces", box(data)},
							 {u8"num", box(num)},
							 {u8"order", box(format)},
							 {u8"object_id", box(instance)} });

						result = plugin.execute(u8"cassius.forward", param).as<param_vector<param_vector<float>>>();
					}
					else
					{
						auto aligned_face_array = root["aligned_images"];
						std::vector<uint8_t> aligned_faces_vec;
						std::vector<uint8_t> temp(cassius_forward_aligned_buffer_len, 0);
						std::uint8_t* ptr = temp.data();
						for (auto i : aligned_face_array)
						{
							std::string aligned_face_base64_str = i.asString();
							if (aligned_face_base64_str.size() != TB64ENCLEN(cassius_forward_aligned_buffer_len))
								throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: aligned_face_base64_str.size() != TB64ENCLEN(cassius_forward_aligned_buffer_len)");

							size_t aligned_face_decode_len = tb64xdec(reinterpret_cast<const std::uint8_t*>(aligned_face_base64_str.data()), aligned_face_base64_str.size(), ptr);
							if (aligned_face_decode_len != cassius_forward_aligned_buffer_len)
								throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "aligned_face_decode_len != cassius_forward_aligned_buffer_len");

							aligned_faces_vec.insert(aligned_faces_vec.end(), temp.begin(), temp.end());
							num++;
						}

						auto param = make_param_hash_map<param_string, unknown_object>(
							{ {u8"aligned_faces", box(param_span<std::uint8_t>{aligned_faces_vec.data(), aligned_faces_vec.size()})},
							 {u8"num", box(num)},
							 {u8"order", box(format)},
							 {u8"object_id", box(instance)} });
						result = plugin.execute(u8"cassius.forward", param).as<param_vector<param_vector<float>>>();
					}

					Json::Value jobj_features = Json::Value(Json::arrayValue);
					if (external.size() >= num * 512 * sizeof(float))
					{
						for (size_t i = 0; i < result.size(); i++)
							result[i].copy_to(0, { reinterpret_cast<float*>(external.data()) + i * 512, 512 });
					}
					else
					{
						for (size_t i = 0; i < result.size(); i++)
						{
							Json::Value jarray_feature;
							for (size_t j = 0; j < result[i].size(); j++)
								jarray_feature["feature"].append(result[i][j]);
							jobj_features.append(jarray_feature);
						}
					}

					value["features"] = jobj_features;
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

			inline Json::Value Selene_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int model_type = root["model_type"].asInt();
					int device = root["device"].asInt();
					bool use_int8 = root["use_int8"].asBool();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"model_type", box(model_type)},
						 {u8"device", box(device)},
						 {u8"use_int8", box(use_int8 ? 1 : 0)},
						 {u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"selene.new", param));
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
						
			inline Json::Value Selene_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"selene.delete", param);

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

			constexpr int selene_forward_aligned_buffer_len = 3 * 128 * 128;
			inline Json::Value Selene_forward_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					int format = root["format"].asInt();
					if (format < 0 || format > 1)
						throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: format < 0 || format > 1");

					int num = 0;
					param_vector<param_vector<float>> result;
					if (data.size() && (data.size() % selene_forward_aligned_buffer_len == 0))
					{
						num = data.size() / selene_forward_aligned_buffer_len;
						auto param = make_param_hash_map<param_string, unknown_object>(
							{ {u8"aligned_faces", box(data)},
							 {u8"num", box(num)},
							 {u8"order", box(format)},
							 {u8"object_id", box(instance)} });

						result = plugin.execute(u8"selene.forward", param).as<param_vector<param_vector<float>>>();
					}
					else
					{
						auto aligned_face_array = root["aligned_images"];
						std::vector<uint8_t> aligned_faces_vec;
						std::vector<uint8_t> temp(selene_forward_aligned_buffer_len, 0);
						std::uint8_t* ptr = temp.data();
						for (auto i : aligned_face_array)
						{
							std::string aligned_face_base64_str = i.asString();
							if (aligned_face_base64_str.size() != TB64ENCLEN(selene_forward_aligned_buffer_len))
								throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: aligned_face_base64_str.size() != TB64ENCLEN(selene_forward_aligned_buffer_len)");

							size_t aligned_face_decode_len = tb64xdec(reinterpret_cast<const std::uint8_t*>(aligned_face_base64_str.data()), aligned_face_base64_str.size(), ptr);
							if (aligned_face_decode_len != selene_forward_aligned_buffer_len)
								throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "aligned_face_decode_len != selene_forward_aligned_buffer_len");

							aligned_faces_vec.insert(aligned_faces_vec.end(), temp.begin(), temp.end());
							num++;
						}

						auto param = make_param_hash_map<param_string, unknown_object>(
							{ {u8"aligned_faces", box(param_span<std::uint8_t>{aligned_faces_vec.data(), aligned_faces_vec.size()})},
							 {u8"num", box(num)},
							 {u8"order", box(format)},
							 {u8"object_id", box(instance)} });
						result = plugin.execute(u8"selene.forward", param).as<param_vector<param_vector<float>>>();
					}

					Json::Value jobj_features = Json::Value(Json::arrayValue);
					if (external.size() >= num * 256 * sizeof(float))
					{
						for (size_t i = 0; i < result.size(); i++)
							result[i].copy_to(0, { reinterpret_cast<float*>(external.data()) + i * 256, 256 });
					}
					else
					{
						for (size_t i = 0; i < result.size(); i++)
						{
							Json::Value jarray_feature;
							for (size_t j = 0; j < result[i].size(); j++)
								jarray_feature["feature"].append(result[i][j]);
							jobj_features.append(jarray_feature);
						}
					}

					value["features"] = jobj_features;
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

			inline Json::Value Selene_make_mask_forward_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				
				try
				{
					int format = root["format"].asInt();
					if (format < 0 || format > 1)
						throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: format < 0 || format > 1");

					std::uint8_t mask[64 * 128] = { 0 };
					int num = 0;
					param_vector<param_vector<float>> result;
					if (data.size() && (data.size() % selene_forward_aligned_buffer_len == 0))
					{
						num = data.size() / selene_forward_aligned_buffer_len;
						std::vector<std::uint8_t> temp(num * selene_forward_aligned_buffer_len, 0);
						std::copy(data.begin(), data.end(), temp.begin());
						std::uint8_t* ptr = temp.data();
						if (format == 0)
						{
							for (int i = 0; i < num; i++)
							{
								for (int j = 0; j < 3; j++)
								{
									std::copy(mask, mask + 64 * 128, ptr + i * selene_forward_aligned_buffer_len + j * 128 * 128 + 64 * 128);
								}
							}
						}
						else
						{
							for (int i = 0; i < num; i++)
							{
								for (int j = 0; j < 3; j++)
								{
									std::copy(mask, mask + 64 * 128, ptr + i * selene_forward_aligned_buffer_len + 3 * 64 * 128 + j * 64 * 128);
								}
							}
						}

						auto param = make_param_hash_map<param_string, unknown_object>(
							{ {u8"aligned_faces", box(param_span<std::uint8_t>{temp})},
							 {u8"num", box(num)},
							 {u8"order", box(format)},
							 {u8"object_id", box(instance)} });

						std::int32_t model_type = unbox<std::int32_t>(plugin.execute(u8"selene.get_model_type", param));
						if (model_type != 2)
							throw parser_exception(parser_exception::parser_exception_code::INVALID_OPERATION, "Illegal operation. model_type != 2");

						result = plugin.execute(u8"selene.forward", param).as<param_vector<param_vector<float>>>();
					}
					else
					{
						auto aligned_face_array = root["aligned_images"];
						std::vector<std::uint8_t> temp(selene_forward_aligned_buffer_len, 0);
						std::uint8_t* ptr = temp.data();
						std::vector<uint8_t> aligned_faces_vec;
						for (auto i : aligned_face_array)
						{
							std::string aligned_face_base64_str = i.asString();
							if (aligned_face_base64_str.size() != TB64ENCLEN(selene_forward_aligned_buffer_len))
								throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: aligned_face_base64_str.size() != TB64ENCLEN(selene_forward_aligned_buffer_len)");

							size_t aligned_face_decode_len = tb64xdec(reinterpret_cast<const std::uint8_t*>(aligned_face_base64_str.data()), aligned_face_base64_str.size(), ptr);
							if (aligned_face_decode_len != selene_forward_aligned_buffer_len)
								throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "aligned_face_decode_len != selene_forward_aligned_buffer_len");

							if (format == 0)
							{
								for (size_t j = 0; j < 3; j++)
								{
									std::copy(mask, mask + 64 * 128, ptr + j * 128 * 128 + 64 * 128);
								}
							}
							else
							{
								for (size_t j = 0; j < 3; j++)
								{
									std::copy(mask, mask + 64 * 128, ptr + 3 * 64 * 128 + j * 64 * 128);
								}
							}
							aligned_faces_vec.insert(aligned_faces_vec.end(), temp.begin(), temp.end());
							num++;
						}

						auto param = make_param_hash_map<param_string, unknown_object>(
							{ {u8"aligned_faces", box(param_span<std::uint8_t>{aligned_faces_vec})},
							 {u8"num", box(num)},
							 {u8"order", box(format)},
							 {u8"object_id", box(instance)} });

						std::int32_t model_type = unbox<std::int32_t>(plugin.execute(u8"selene.get_model_type", param));
						if (model_type != 2)
							throw parser_exception(parser_exception::parser_exception_code::INVALID_OPERATION, "Illegal operation. model_type != 2");
						
						result = plugin.execute(u8"selene.forward", param).as<param_vector<param_vector<float>>>();
					}

					Json::Value jobj_features = Json::Value(Json::arrayValue);
					if (external.size() >= num * selene_forward_aligned_buffer_len * sizeof(float))
					{
						for (size_t i = 0; i < result.size(); i++)
							result[i].copy_to(0, { reinterpret_cast<float*>(external.data()) + i * selene_forward_aligned_buffer_len, selene_forward_aligned_buffer_len });
					}
					else
					{
						for (size_t i = 0; i < result.size(); i++)
						{
							Json::Value jarray_feature;
							for (size_t j = 0; j < result[i].size(); j++)
								jarray_feature["feature"].append(result[i][j]);
							jobj_features.append(jarray_feature);
						}
					}

					value["features"] = jobj_features;
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

			inline Json::Value Damocles_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					int device = root["device"].asInt();
					// bool use_int8 = root["use_int8"].asBool();
					int model_type = root["model_type"].asInt();

					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"device", box(device)},
					 	{u8"model_type", box(model_type)},
						 {u8"models_directory", box(std::string_view(models_directory))} });

					instance = unbox<guid>(plugin.execute(u8"damocles.new", param));
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
			inline Json::Value Damocles_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"damocles.delete", param);

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

			inline Json::Value Damocles_spoofing_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();

					Json::Value facerect_list = root["facerect_list"];
					auto faces = make_param_vector<longinus::face_info>();
					for (auto& i : facerect_list)
					{
						auto face = make_exported_interface<longinus::face_info>();
						face.set_x(i["x"].asFloat());
						face.set_y(i["y"].asFloat());
						face.set_width(i["width"].asFloat());
						face.set_height(i["height"].asFloat());
						faces.push_back(face);
					}

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"image", box(image_span)},
						 {u8"height", box(height)},
						 {u8"width", box(width)},
						 {u8"faces", faces},
						 {u8"order", box(static_cast<int>(frame->format_))},
						 {u8"object_id", box(instance)} });

					auto result = plugin.execute(u8"damocles.spoofing_detect", param).as<param_vector<param_vector<float>>>();
					Json::Value jobj_results = Json::Value(Json::arrayValue);
					for (size_t i = 0; i < result.size(); i++)
					{
						Json::Value jarray_prob;
						for (size_t j = 0; j < result[i].size(); j++)
							jarray_prob["prob"].append(result[i][j]);
						jobj_results.append(jarray_prob);
					}

					value["spoofing_result"] = Json::Value(jobj_results);
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

			inline Json::Value Damocles_presentation_attack_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();
					int action_cmd = root["action_cmd"].asInt();

					Json::Value facerect = root["facerect"];
					auto face = make_exported_interface<longinus::face_info>();
					face.set_x(root["facerect"]["x"].asFloat());
					face.set_y(root["facerect"]["y"].asFloat());
					face.set_width(root["facerect"]["width"].asFloat());
					face.set_height(root["facerect"]["height"].asFloat());

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"image", box(image_span)},
						 {u8"action_cmd", box(action_cmd)},
						 {u8"height", box(height)},
						 {u8"width", box(width)},
						 {u8"face", face},
						 {u8"order", box(static_cast<int>(frame->format_))},
						 {u8"object_id", box(instance)} });

					auto result = unbox<bool>(plugin.execute(u8"damocles.presentation_attack_detect", param));

					value["presentation_attack_result"] = Json::Value(result);
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

			inline Json::Value Irisviel_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					//int single_database_capacity = root["single_database_capacity"].asInt();
					int single_database_capacity = 1000;
					int dimension = root["dimension"].asInt();
					std::string working_directory = root["working_directory"].asString();

					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"single_database_capacity", box(single_database_capacity)},
						 {u8"dimension", box(dimension)},
						 {u8"working_directory", box(std::string_view(working_directory))} });

					instance = unbox<guid>(plugin.execute(u8"irisviel.new", param));

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
			inline Json::Value Irisviel_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });

					plugin.execute(u8"irisviel.delete", param);

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

			inline Json::Value Irisviel_record_count_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{ u8"object_id", box(instance) }
						});

					value["result"] = static_cast<Json::UInt64>(unbox<std::uint64_t>(plugin.execute(u8"irisviel.record_count", param)));
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

			inline Json::Value Irisviel_contains_key_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				
				try
				{
					auto key = root["key"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{ u8"object_id", box(instance) },
							{ u8"key", box(to_param_string(key)) },
						});

					value["result"] = unbox<bool>(plugin.execute(u8"irisviel.contains_key", param));
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

			inline Json::Value Irisviel_try_get_record_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					auto key = root["key"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{ u8"object_id", box(instance) },
							{ u8"key", box(to_param_string(key)) },
						});

					if (auto result = plugin.execute(u8"irisviel.try_get_record", param).as<irisviel::record>())
					{
						value["result"]["key"] = Json::Value(exposing::to_narrow_string(result.key()));
						value["result"]["feature"] = [&]
						{
							Json::Value feature{ Json::arrayValue };
							
							for (auto&& item : result.feature())
							{
								feature.append(item);
							}

							return feature;
						}();

						
					}
					else
					{
						value["result"] = Json::Value(Json::nullValue);
					}

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
			inline Json::Value Irisviel_search_nf_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					value["result"] = Json::Value(Json::arrayValue);

					param_vector<float> feature = make_param_vector<float>();

					auto jarray_feature = root["feature"];
					for (auto i : jarray_feature)
						feature.push_back(i.asFloat());
					auto assuming_top = root.get("top", Json::nullValue);
					auto assuming_min_similarity = root.get("min_similarity", Json::nullValue);
					bool has_top = assuming_top.isIntegral();
					bool has_min_similarity = assuming_min_similarity.isNumeric();

					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"feature", feature},
						 {u8"object_id", box(instance)} });
					if (has_top)
					{
						param.add_or_update(u8"top", box(static_cast<std::uint32_t>(assuming_top.asUInt())));
					}

					if (has_min_similarity)
					{
						param.add_or_update(u8"min_similarity", box(assuming_min_similarity.asFloat()));
					}

					auto result = plugin.execute(u8"irisviel.search_nf", param).as<param_vector<irisviel::search_result>>();

					for (const auto& item : result)
					{
						Json::Value jobj_result;
						Json::Value jobj_data;
						jobj_data["key"] = Json::Value(to_narrow_string(item.key()));
						jobj_result["data"] = jobj_data;
						jobj_result["similarity"] = Json::Value(item.similarity());
						value["result"].append(jobj_result);
					}

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
			inline Json::Value Irisviel_search_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					value["result"] = Json::Value(Json::arrayValue);

					param_vector<float> feature = make_param_vector<float>();

					auto jarray_feature = root["feature"];
					for (auto i : jarray_feature)
						feature.push_back(i.asFloat());

					auto assuming_top = root.get("top", Json::nullValue);
					auto assuming_min_similarity = root.get("min_similarity", Json::nullValue);
					bool has_top = assuming_top.isIntegral();
					bool has_min_similarity = assuming_min_similarity.isNumeric();

					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"feature", feature},
						 {u8"object_id", box(instance)} });

					if (has_top)
					{
						param.add_or_update(u8"top", box(static_cast<std::uint32_t>(assuming_top.asUInt())));
					}

					if (has_min_similarity)
					{
						param.add_or_update(u8"min_similarity", box(assuming_min_similarity.asFloat()));
					}

					auto result = plugin.execute(u8"irisviel.search", param).as<param_vector<irisviel::search_result>>();

					for (const auto& item : result)
					{
						Json::Value jobj_result;
						Json::Value jobj_data;

						auto feature = item.feature();
						for (size_t j = 0; j < feature.size(); j++)
							jobj_data["feature"].append(Json::Value(feature[j]));

						jobj_data["key"] = Json::Value(to_narrow_string(item.key()));

						jobj_result["data"] = jobj_data;
						jobj_result["similarity"] = Json::Value(item.similarity());

						value["result"].append(jobj_result);
					}

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
			inline Json::Value Irisviel_clear_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });
					plugin.execute(u8"irisviel.clear", param);
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
			inline Json::Value Irisviel_remove_all_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });
					plugin.execute(u8"irisviel.remove_all", param);
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
			inline Json::Value Irisviel_load_databases_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"object_id", box(instance)} });
					plugin.execute(u8"irisviel.load_databases", param);
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
			inline Json::Value Irisviel_remove_records_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					auto keys = make_param_vector<param_string>();
					auto jarray_keys = root["keys"];
					for (auto i : jarray_keys)
						keys.push_back(to_param_string(i.asString()));

					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"keys", keys},
						 {u8"object_id", box(instance)} });

					auto result = plugin.execute(u8"irisviel.remove_records", param).as<exposing::param_vector<bool>>();

                    parse_aud_operation_result(value, result, u8"Could not find the key.");
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
			
			inline Json::Value Irisviel_add_records_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					auto jarray_data = root["data"];

					auto vec = make_param_vector<param_hash_map<param_string, unknown_object>>();
					for (auto i : jarray_data)
					{
						param_vector<float> feature = make_param_vector<float>();
						auto jarray_feature = i["feature"];
						for (auto j : jarray_feature)
							feature.push_back(j.asFloat());

						int dimension = static_cast<int>(feature.size());

						std::string key = i["key"].asString();

						auto data = make_param_hash_map<param_string, unknown_object>(
							{ {u8"dimension", box(dimension)},
							 {u8"key", box(std::string_view(key))},
							 {u8"feature", feature} });

						vec.push_back(data);
					}

					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"records", vec},
						 {u8"object_id", box(instance)} });

					auto result = plugin.execute(u8"irisviel.add_records", param).as<exposing::param_vector<bool>>();

					parse_aud_operation_result(value, result, u8"The key already exists.");
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
			
			inline Json::Value Irisviel_update_records_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					auto jarray_data = root["data"];

					auto vec = make_param_vector<param_hash_map<param_string, unknown_object>>();
					for (auto i : jarray_data)
					{
						param_vector<float> feature = make_param_vector<float>();
						auto jarray_feature = i["feature"];
						for (auto j : jarray_feature)
							feature.push_back(j.asFloat());

						int dimension = static_cast<int>(feature.size());

						std::string key = i["key"].asString();

						auto data = make_param_hash_map<param_string, unknown_object>(
							{ {u8"dimension", box(dimension)},
							 {u8"key", box(std::string_view(key))},
							 {u8"feature", feature} });

						vec.push_back(data);
					}

					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"records", vec},
						 {u8"object_id", box(instance)} });

					auto result = plugin.execute(u8"irisviel.update_records", param).as<exposing::param_vector<bool>>();
					
					parse_aud_operation_result(value, result, u8"Could not find the key.");
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

			inline Json::Value Fusion_Romancia_alignFace128_Gaius_forward_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, std::vector<guid>& guids, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();
					auto jarray_rect = root["facerectwithfaceinfo_list"];
					bool has_mask = root["has_mask"].asBool();

					auto faces = exposing::make_param_vector<longinus::face_info>();
					for (auto i : jarray_rect)
					{
						auto face = exposing::make_exported_interface<longinus::face_info>();
						face.set_x(i["x"].asFloat());
						face.set_y(i["y"].asFloat());
						face.set_height(i["height"].asFloat());
						face.set_width(i["width"].asFloat());

						auto landmark_list = i["landmark"];
						if (landmark_list.size() != 5)
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "landmark_list.size() != 5");
						auto landmark = exposing::make_param_vector<exposing::param_pair<float, float>>();
						for (auto j : landmark_list)
						{
							auto pair = exposing::make_param_pair(j["x"].asFloat(), j["y"].asFloat());
							landmark.push_back(pair);
						}
						face.set_pts(landmark);

						faces.push_back(face);
					}

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto romancia_param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"image", box(image_span)},
						 {u8"height", box(height)},
						 {u8"width", box(width)},
						 {u8"faces", faces},
						 {u8"order", box(static_cast<int>(frame->format_))},
						 {u8"object_id", box(guids[0])} });

					auto romancia_result = plugin.execute(u8"romancia.alignFace128", romancia_param).as<param_vector<param_vector<std::uint8_t>>>();

					std::vector<std::uint8_t> buffer(romancia_result.size() * gaius_forward_aligned_buffer_len);
					for (size_t i = 0; i < romancia_result.size(); i++)
					{
						if (romancia_result[i].size() != gaius_forward_aligned_buffer_len)
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "romancia_result[i].size() != gaius_forward_aligned_buffer_len");

						romancia_result[i].copy_to(0, {buffer.data() + i * gaius_forward_aligned_buffer_len, gaius_forward_aligned_buffer_len });
						//buffer.insert(buffer.end(), begin(romancia_result[i]), end(romancia_result[i]));
					}

					auto gaius_param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"aligned_faces", box(param_span<std::uint8_t>{buffer.data(), buffer.size()})},
						 {u8"num", box(static_cast<int>(romancia_result.size()))},
						 {u8"order", box(0)},
						 {u8"has_mask", box(has_mask ? 1 : 0)},
						 {u8"object_id", box(guids[1])} });

					auto gaius_result = plugin.execute(u8"gaius.forward", gaius_param).as<param_vector<param_vector<float>>>();

					Json::Value jobj_features = Json::Value(Json::arrayValue);
					if (external.size() >= gaius_result.size() * 128 * sizeof(float))
					{
						for (size_t i = 0; i < gaius_result.size(); i++)
							gaius_result[i].copy_to(0, { reinterpret_cast<float*>(external.data()) + i * 128, 128 });
					}
					else
					{
						for (size_t i = 0; i < gaius_result.size(); i++)
						{
							Json::Value jarray_feature;
							for (size_t j = 0; j < gaius_result[i].size(); j++)
								jarray_feature["feature"].append(gaius_result[i][j]);
							jobj_features.append(jarray_feature);
						}
					}

					value["features"] = jobj_features;
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

			inline Json::Value Fusion_Romancia_alignFace_Selene_forward_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, std::vector<guid>& guids, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();
					auto jarray_rect = root["facerectwithfaceinfo_list"];

					auto faces = exposing::make_param_vector<longinus::face_info>();
					for (auto i : jarray_rect)
					{
						auto face = exposing::make_exported_interface<longinus::face_info>();
						face.set_x(i["x"].asFloat());
						face.set_y(i["y"].asFloat());
						face.set_height(i["height"].asFloat());
						face.set_width(i["width"].asFloat());

						auto landmark_list = i["landmark"];
						if (landmark_list.size() != 5)
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "landmark_list.size() != 5");
						auto landmark = exposing::make_param_vector<exposing::param_pair<float, float>>();
						for (auto j : landmark_list)
						{
							auto pair = exposing::make_param_pair(j["x"].asFloat(), j["y"].asFloat());
							landmark.push_back(pair);
						}
						face.set_pts(landmark);

						faces.push_back(face);
					}

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto romancia_param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"image", box(image_span)},
						 {u8"height", box(height)},
						 {u8"width", box(width)},
						 {u8"faces", faces},
						 {u8"order", box(static_cast<int>(frame->format_))},
						 {u8"object_id", box(guids[0])} });

					auto romancia_result = plugin.execute(u8"romancia.alignFace", romancia_param).as<param_vector<param_vector<std::uint8_t>>>();

					std::vector<std::uint8_t> buffer(romancia_result.size() * selene_forward_aligned_buffer_len);
					for (size_t i = 0; i < romancia_result.size(); i++)
					{
						if (romancia_result[i].size() != selene_forward_aligned_buffer_len)
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "romancia_result[i].size() != selene_forward_aligned_buffer_len");
						
						romancia_result[i].copy_to(0, { buffer.data() + i * selene_forward_aligned_buffer_len, selene_forward_aligned_buffer_len });
						//buffer.insert(buffer.end(), begin(romancia_result[i]), end(romancia_result[i]));
					}

					auto selene_param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"aligned_faces", box(param_span<std::uint8_t>{buffer.data(), buffer.size()})},
						 {u8"num", box(static_cast<int>(romancia_result.size()))},
						 {u8"order", box(0)},
						 {u8"object_id", box(guids[1])} });

					auto selene_result = plugin.execute(u8"selene.forward", selene_param).as<param_vector<param_vector<float>>>();

					Json::Value jobj_features = Json::Value(Json::arrayValue);
					if (external.size() >= selene_result.size() * 256 * sizeof(float))
					{
						for (size_t i = 0; i < selene_result.size(); i++)
							selene_result[i].copy_to(0, { reinterpret_cast<float*>(external.data()) + i * 256, 256 });
					}
					else
					{
						for (size_t i = 0; i < selene_result.size(); i++)
						{
							Json::Value jarray_feature;
							for (size_t j = 0; j < selene_result[i].size(); j++)
								jarray_feature["feature"].append(selene_result[i][j]);
							jobj_features.append(jarray_feature);
						}
					}

					value["features"] = jobj_features;
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

			inline Json::Value Fusion_Romancia_alignFace_Cassius_forward_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, std::vector<guid>& guids, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();
					auto jarray_rect = root["facerectwithfaceinfo_list"];

					auto faces = exposing::make_param_vector<longinus::face_info>();
					for (auto i : jarray_rect)
					{
						auto face = exposing::make_exported_interface<longinus::face_info>();
						face.set_x(i["x"].asFloat());
						face.set_y(i["y"].asFloat());
						face.set_height(i["height"].asFloat());
						face.set_width(i["width"].asFloat());

						auto landmark_list = i["landmark"];
						if (landmark_list.size() != 5)
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "landmark_list.size() != 5");
						auto landmark = exposing::make_param_vector<exposing::param_pair<float, float>>();
						for (auto j : landmark_list)
						{
							auto pair = exposing::make_param_pair(j["x"].asFloat(), j["y"].asFloat());
							landmark.push_back(pair);
						}
						face.set_pts(landmark);

						faces.push_back(face);
					}

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto romancia_param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"image", box(image_span)},
						 {u8"height", box(height)},
						 {u8"width", box(width)},
						 {u8"faces", faces},
						 {u8"order", box(static_cast<int>(frame->format_))},
						 {u8"object_id", box(guids[0])} });

					auto romancia_result = plugin.execute(u8"romancia.alignFace", romancia_param).as<param_vector<param_vector<std::uint8_t>>>();

					std::vector<std::uint8_t> buffer(romancia_result.size() * cassius_forward_aligned_buffer_len);
					for (size_t i = 0; i < romancia_result.size(); i++)
					{
						if (romancia_result[i].size() != cassius_forward_aligned_buffer_len)
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "romancia_result[i].size() != cassius_forward_aligned_buffer_len");

						romancia_result[i].copy_to(0, {buffer.data()+ i * cassius_forward_aligned_buffer_len, cassius_forward_aligned_buffer_len});
						//buffer.insert(buffer.end(), begin(romancia_result[i]), end(romancia_result[i]));
					}

					auto cassius_param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"aligned_faces", box(param_span<std::uint8_t>{buffer.data(), buffer.size()})},
						 {u8"num", box(static_cast<int>(romancia_result.size()))},
						 {u8"order", box(0)},
						 {u8"object_id", box(guids[1])} });

					auto cassius_result = plugin.execute(u8"cassius.forward", cassius_param).as<param_vector<param_vector<float>>>();

					Json::Value jobj_features = Json::Value(Json::arrayValue);
					if (external.size() >= cassius_result.size() * 512 * sizeof(float))
					{
						for (size_t i = 0; i < cassius_result.size(); i++)
							cassius_result[i].copy_to(0, { reinterpret_cast<float*>(external.data()) + i * 512, 512 });
					}
					else
					{
						for (size_t i = 0; i < cassius_result.size(); i++)
						{
							Json::Value jarray_feature;
							for (size_t j = 0; j < cassius_result[i].size(); j++)
								jarray_feature["feature"].append(cassius_result[i][j]);
							jobj_features.append(jarray_feature);
						}
					}

					value["features"] = jobj_features;
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

            inline Json::Value Fusion_Romancia_rotate_Longinus_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, std::vector<guid>& guids, param_span<std::uint8_t>& external)
            {
                Json::Value value;

                try
                {
                    int format = root["format"].asInt();
                    int height = root["height"].asInt();
                    int width = root["width"].asInt();
                    int min_size = root["min_size"].asInt();
                    float threshold = root["threshold"].asFloat();
                    bool do_attributing = root["do_attributing"].asBool();
                    float angle = root["angle"].asFloat();

                    auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
                    param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

                    auto romancia_param = make_param_hash_map<param_string, unknown_object>(
                        { {u8"image", box(image_span)},
                         {u8"height", box(height)},
                         {u8"width", box(width)},
                         {u8"angle", box(angle)},
                         {u8"order", box(static_cast<int>(frame->format_))},
                         {u8"object_id", box(guids[0])} });

                    auto romancia_result = plugin.execute(u8"romancia.rotate", romancia_param).as<param_vector<std::uint8_t>>();

                    if (external.size() >= romancia_result.size())
						romancia_result.copy_to(0, { external.data(), static_cast<size_t>(romancia_result.size()) });

                    int rotated_height = height;
                    int rotated_width = width;
                    if (angle == 90.0f || angle == 270.0f)
                    {
                        rotated_height = width;
                        rotated_width = height;
                    }

                    std::vector<std::uint8_t> img_rotated(romancia_result.size());
					romancia_result.copy_to(0, img_rotated);

                    auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"image", box(param_span<std::uint8_t>{img_rotated})},
                         {u8"height", box(rotated_height)},
                         {u8"width", box(rotated_width)},
                         {u8"min_size", box(min_size)},
                         {u8"threshold", box(threshold)},
                         {u8"order", box(1)},
                         {u8"do_attributing", box(do_attributing)},
                         {u8"object_id", box(guids[1])} });

                    auto longinus_result = plugin.execute(u8"longinus.detect", param).as<param_vector<longinus::face_info>>();

                    Json::Value jarray_rect = Json::Value(Json::arrayValue);

                    for (auto obj : longinus_result)
                    {
                        Json::Value jobj_face;
                        jobj_face["x"] = Json::Int(obj.x());
                        jobj_face["y"] = Json::Int(obj.y());
                        jobj_face["width"] = Json::Int(obj.width());
                        jobj_face["height"] = Json::Int(obj.height());
                        jobj_face["confidence"] = Json::Value(obj.confidence());

                        if (do_attributing)
                        {
                            jobj_face["attributes"]["yaw"] = Json::Value(obj.yaw());
                            jobj_face["attributes"]["pitch"] = Json::Value(obj.pitch());
                            jobj_face["attributes"]["roll"] = Json::Value(obj.roll());
                            jobj_face["attributes"]["glass_index"] = Json::Int(obj.glass_index());
                            jobj_face["attributes"]["mask_index"] = Json::Int(obj.mask_index());
                        }
                        else
                            jobj_face["attributes"] = Json::Value(Json::nullValue);

                        Json::Value jarray_landmark;

                        for (const auto& pt : obj.pts())
                        {

                            Json::Value jobj_point;
                            jobj_point["x"] = Json::Int((int)pt.key());
                            jobj_point["y"] = Json::Int((int)pt.value());
                            jarray_landmark.append(jobj_point);
                        }
                        jobj_face["landmark"] = jarray_landmark;
                        jarray_rect.append(jobj_face);
                    }

                    value["facerectwithfaceinfo_list"] = jarray_rect;
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

			inline Json::Value Fusion_Romancia_rotate_Longinus_trace_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, std::vector<guid>& guids, param_span<std::uint8_t>& external)
			{
				Json::Value value;

				try
				{
					int format = root["format"].asInt();
					int height = root["height"].asInt();
					int width = root["width"].asInt();
					float angle = root["angle"].asFloat();
					auto face = make_exported_interface<longinus::face_info>();
					face.set_x(root["face"]["x"].asFloat());
					face.set_y(root["face"]["y"].asFloat());
					face.set_width(root["face"]["width"].asFloat());
					face.set_height(root["face"]["height"].asFloat());

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame->data_), frame->size_);

					auto romancia_param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"image", box(image_span)},
						 {u8"height", box(height)},
						 {u8"width", box(width)},
						 {u8"angle", box(angle)},
						 {u8"order", box(static_cast<int>(frame->format_))},
						 {u8"object_id", box(guids[0])} });

					auto romancia_result = plugin.execute(u8"romancia.rotate", romancia_param).as<param_vector<std::uint8_t>>();

					if (external.size() >= romancia_result.size())
						romancia_result.copy_to(0, { external.data(), static_cast<size_t>(romancia_result.size()) });

					int rotated_height = height;
					int rotated_width = width;
					if (angle == 90.0f || angle == 270.0f)
					{
						rotated_height = width;
						rotated_width = height;
					}

					std::vector<std::uint8_t> img_rotated(romancia_result.size());
					romancia_result.copy_to(0, img_rotated);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{ {u8"image", box(param_span<std::uint8_t>{img_rotated})},
						 {u8"height", box(rotated_height)},
						 {u8"width", box(rotated_width)},
						 {u8"face", face},
						 {u8"order", box(1)},
						 {u8"object_id", box(guids[1])} });
					auto result = plugin.execute(u8"longinus.trace", param).as<longinus::face_info>();

					Json::Value jobj_face;
					if (result.confidence() > 0.1f)
					{
						value["trace_success"] = Json::Value(true);
						jobj_face["x"] = Json::Int(result.x());
						jobj_face["y"] = Json::Int(result.y());
						jobj_face["width"] = Json::Int(result.width());
						jobj_face["height"] = Json::Int(result.height());
						jobj_face["confidence"] = Json::Value(result.confidence());

						jobj_face["attributes"]["glass_index"] = Json::Int(result.glass_index());
						jobj_face["attributes"]["mask_index"] = Json::Int(result.mask_index());
						jobj_face["attributes"]["yaw"] = Json::Value(result.yaw());
						jobj_face["attributes"]["pitch"] = Json::Value(result.pitch());
						jobj_face["attributes"]["roll"] = Json::Value(result.roll());

						Json::Value jarray_landmark;

						for (const auto& pt : result.pts())
						{

							Json::Value jobj_point;
							jobj_point["x"] = Json::Int((int)pt.key());
							jobj_point["y"] = Json::Int((int)pt.value());
							jarray_landmark.append(jobj_point);
						}
						jobj_face["landmark"] = jarray_landmark;
					}
					else
					{
						value["trace_success"] = Json::Value(false);
					}

					value["facerectwithfaceinfo"] = jobj_face;
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
        }
    }
}
#endif
