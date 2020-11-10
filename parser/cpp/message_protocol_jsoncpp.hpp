#ifndef _MESSAGEPROTOCOL_SIMDJSON_HPP_
#define _MESSAGEPROTOCOL_SIMDJSON_HPP_

#include <libyuv.h>
#include "json.h"
#include "base64_x.hpp"
#include "plugin_interface.hpp"
#include "parser_exception.hpp"
#include "../../common/include/longinus/face_info.hpp"
#include "../../common/include/irisviel/search_result.hpp"
#include "../../common/include/irisviel/record.hpp"
#include "../../common/include/Primitives/tensor.hpp"
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
				PROTOCOL_IMAGE_NV21 = 2
			};

			inline void convert_to_bgr(memory::tensor<std::uint8_t>& src, PROTOCOL_IMAGE_FORMAT src_format, memory::tensor<std::uint8_t>& dst, int width, int height)
			{
				switch (src_format)
				{
				case PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_BGR_NCHW:
				{
					if (width * height * 3 != src.count())
						throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "BGR_NCHW, width * height * 3 != src.count()");
					dst = src;
					break;
				}
				case PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_BGR_NHWC:
				{
					int step = src.count() / height;

					if (step * height != src.count())
						throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "BGR_NHWC, step * height != src.count()");
					
					if(step == width * 3)
						dst = src;
					else
					{
						dst = memory::tensor<std::uint8_t>(std::vector<int>{1, 3, height, width}, src.device(), src.order(), src.allocator());
						std::uint8_t* dst_ptr = dst.mutable_cpu_data();
						const std::uint8_t* src_ptr = src.cpu_data();
						for (size_t i = 0; i < height; i++)
							std::copy(src_ptr + i * step, src_ptr + i * step + width * 3, dst_ptr);
					}

					break;
				}
				case PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_NV21:
				{
					if (src.count() != (width * height * 3 >> 1))
						throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "convert_to_bgr: src.count() != (width * height * 3 >> 1)");

					dst = memory::tensor<std::uint8_t>(std::vector<int>{1, height, width, 3}, -1, memory::NHWC, & memory::pool_allocator_default<std::uint8_t>::get());
					int aligned_src_width = (width + 1) & ~1;
					const uint8_t* y = src.cpu_data();
					const uint8_t* uv = src.cpu_data() + aligned_src_width * height;
					if (libyuv::NV21ToRGB24(y, width, uv, aligned_src_width, dst.mutable_cpu_data(), width * 3, width, height))
						throw parser_exception(parser_exception::parser_exception_code::INTERNAL_FUNCTION_FAILURE, "NV21ToRGB24 failed.");

					break;
				}
				default:
					throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Unsupported image format.");
					break;
				}
			}

			inline memory::tensor<std::uint8_t> decode_and_convert(param_span<std::uint8_t> src, bool is_base64, PROTOCOL_IMAGE_FORMAT format, int width, int height)
			{
				if (height <= 0 || width <= 0)
					throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Invalid argument: height <= 0 || width <= 0");

				if (src.size() <= 0)
					throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Invalid argument: src.size() <= 0");

				memory::orderType order;
				switch (format)
				{
				case glasssix::exposing::nessus::PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_BGR_NCHW:
					order = memory::NCHW;
					break;
				case glasssix::exposing::nessus::PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_BGR_NHWC:
					order = memory::NHWC;
					break;
				default:
					order = memory::NHWC;
					break;
				}

				memory::tensor<std::uint8_t> temp;
				if (is_base64)
				{
					int current_image_str_len = TB64DECLEN(src.size());

					temp = memory::tensor<std::uint8_t>(current_image_str_len, -1, order, &memory::pool_allocator_default<std::uint8_t>::get());
					tb64xdec(reinterpret_cast<const std::uint8_t*>(src.data()), src.size(), temp.mutable_cpu_data());
				}
				else
				{
					temp = memory::tensor<std::uint8_t>(src.size(), -1, order, &memory::pool_allocator_default<std::uint8_t>::get());
					std::copy(src.begin(), src.end(), temp.mutable_cpu_data());
				}

				memory::tensor<std::uint8_t> dst;

				convert_to_bgr(temp, format, dst, width, height);

				return dst;
			}

			inline Json::Value Longinus_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;
				try
				{
					int device = root["device"].asInt();
					//float nms = static_cast<float>(root["nms"].asDouble());
					float nms = 0.4f;
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"device", box(device)},
							{u8"nms", box(nms)},
							{u8"models_directory", box(std::string_view(models_directory))}
						});

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
			inline Json::Value Longinus_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"object_id", box(instance)}
						});

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

			inline Json::Value Longinus_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
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
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame.cpu_data()), frame.count());

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(image_span)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"min_size", box(min_size)},
							{u8"threshold", box(threshold)},
							{u8"order", box(static_cast<int>(frame.order()))},
							{u8"do_attributing", box(do_attributing ? 1 : 0)},
							{u8"object_id", box(instance)}
						});

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
							jobj_face["attributes"]["prob_age_index"] = Json::Int(obj.prob_age_index());
							jobj_face["attributes"]["prob_gender_index"] = Json::Int(obj.prob_gender_index());
						}

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

			inline Json::Value Longinus_trace_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
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
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame.cpu_data()), frame.count());

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(image_span)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"face", face },
							{u8"order", box(static_cast<int>(frame.order()))},
							{u8"object_id", box(instance)}
						});
					auto result = plugin.execute(u8"longinus.trace", param).as<longinus::face_info>();

					Json::Value jobj_face;
					if (result.confidence() > 0.1)
					{
						value["trace_success"] = Json::Value(true);
						jobj_face["x"] = Json::Int(result.x());
						jobj_face["y"] = Json::Int(result.y());
						jobj_face["width"] = Json::Int(result.width());
						jobj_face["height"] = Json::Int(result.height());
						jobj_face["confidence"] = Json::Value(result.confidence());

						jobj_face["attributes"]["prob_age_index"] = Json::Int(result.prob_age_index());
						jobj_face["attributes"]["prob_gender_index"] = Json::Int(result.prob_gender_index());
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

			inline Json::Value Romancia_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;
				try
				{
					int device = root["device"].asInt();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"device", box(device)},
							{u8"models_directory", box(std::string_view(models_directory))}
						});

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
			inline Json::Value Romancia_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"object_id", box(instance)}
						});

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
			inline Json::Value Romancia_alignFace_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
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
						if(landmark_list.size() != 5)
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
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame.cpu_data()), frame.count());

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(image_span)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"faces", faces },
							{u8"order", box(static_cast<int>(frame.order()))},
							{u8"object_id", box(instance)}
						});

					auto result = plugin.execute(u8"romancia.alignFace", param).as<param_vector<param_vector<std::uint8_t>>>();

					value["aligned_images"] = Json::Value(Json::arrayValue);
					memory::tensor<std::uint8_t> temp(romancia_align_aligned_base64_buffer_len, -1, memory::NCHW, &memory::pool_allocator_default<std::uint8_t>::get());
					std::uint8_t* ptr = temp.mutable_cpu_data();
					for (size_t i = 0; i < result.size(); i++)
					{
						std::vector<std::uint8_t> buffer(begin(result[i]), end(result[i]));

						tb64xenc(buffer.data(), buffer.size(), ptr);

						value["aligned_images"].append(Json::Value(reinterpret_cast<char*>(ptr), reinterpret_cast<char*>(ptr) + romancia_align_aligned_base64_buffer_len));
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

			inline Json::Value Romancia_blur_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
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
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame.cpu_data()), frame.count());

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(image_span)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"face", face },
							{u8"order", box(static_cast<int>(frame.order()))},
							{u8"object_id", box(instance)}
						});

					auto result = unbox<double>(plugin.execute(u8"romancia.blur_detect", param));

					value["clarity"] = Json::Value(result);
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

			inline Json::Value Romancia_mask_detect_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
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

					auto landmark_list = root["face"]["landmark"];
					if (landmark_list.size() != 5)
						throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "landmark_list.size() != 5");

					auto landmark = exposing::make_param_vector<exposing::param_pair<float, float>>();
					for (auto j : landmark_list)
					{
						auto pair = exposing::make_param_pair(j["x"].asFloat(), j["y"].asFloat());
						landmark.push_back(pair);
					}
					face.set_pts(landmark);

					auto frame = decode_and_convert(data, false, static_cast<PROTOCOL_IMAGE_FORMAT>(format), width, height);
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame.cpu_data()), frame.count());

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(image_span)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"face", face },
							{u8"order", box(static_cast<int>(frame.order()))},
							{u8"object_id", box(instance)}
						});

					//auto result = unbox<std::int32_t>(plugin.execute(u8"romancia.mask_detect", param));
					auto result = unbox<double>(plugin.execute(u8"romancia.mask_detect", param));

					//value["has_mask"] = Json::Value(result ? true : false);
					value["mask_value"] = Json::Value(result);
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

			inline Json::Value Romancia_antispoofing_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
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
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame.cpu_data()), frame.count());

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(image_span)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"face", face },
							{u8"order", box(static_cast<int>(frame.order()))},
							{u8"object_id", box(instance)}
						});

					auto result = unbox<bool>(plugin.execute(u8"romancia.antispoofing", param));

					value["is_alive"] = Json::Value(result);
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

			inline Json::Value Gaius_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;
				try
				{
					int device = root["device"].asInt();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"device", box(device)},
							{u8"models_directory", box(std::string_view(models_directory))}
						});

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
			inline Json::Value Gaius_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"object_id", box(instance)}
						});

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
			inline Json::Value Gaius_Forward_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;

				try
				{
					int format = root["format"].asInt();
					bool has_mask = root["has_mask"].asBool();
					if (format < 0 || format > 1)
						throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: format < 0 || format > 1");

					auto aligned_face_array = root["aligned_images"];
					std::vector<uint8_t> aligned_faces_vec;
					int num = 0;
					memory::tensor<std::uint8_t> temp(gaius_forward_aligned_buffer_len, -1, memory::NCHW, &memory::pool_allocator_default<std::uint8_t>::get());
					std::uint8_t* ptr = temp.mutable_cpu_data();
					for (auto i : aligned_face_array)
					{
						std::string aligned_face_base64_str = i.asString();
						if (aligned_face_base64_str.size() != TB64ENCLEN(gaius_forward_aligned_buffer_len))
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: aligned_face_base64_str.size() != TB64ENCLEN(gaius_forward_aligned_buffer_len)");

						size_t aligned_face_decode_len = tb64xdec(reinterpret_cast<const std::uint8_t*>(aligned_face_base64_str.data()), aligned_face_base64_str.size(), ptr);
						if (aligned_face_decode_len != gaius_forward_aligned_buffer_len)
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "aligned_face_decode_len != gaius_forward_aligned_buffer_len");

						aligned_faces_vec.insert(aligned_faces_vec.end(), ptr, ptr + gaius_forward_aligned_buffer_len);
						num++;
					}

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"aligned_faces", box(exposing::param_span<std::uint8_t>{aligned_faces_vec.data(), aligned_faces_vec.size()})},
							{u8"num", box(num)},
							{u8"order", box(format)},
							{u8"has_mask", box(has_mask ? 1 : 0)},
							{u8"object_id", box(instance)}
						});

					auto result = plugin.execute(u8"gaius.Forward", param).as<param_vector<param_vector<float>>>();

					Json::Value jobj_features;
					for (size_t i = 0; i < result.size(); i++)
					{
						Json::Value jarray_feature;
						for (size_t j = 0; j < result[i].size(); j++)
							jarray_feature["feature"].append(result[i][j]);
						jobj_features.append(jarray_feature);
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

			inline Json::Value Gaius_make_mask_Forward_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;

				std::uint8_t mask[64 * 128] = { 0 };
				try
				{
					int format = root["format"].asInt();
					if (format < 0 || format > 1)
						throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: format < 0 || format > 1");

					auto aligned_face_array = root["aligned_images"];
					std::vector<uint8_t> aligned_faces_vec;
					int num = 0;
					memory::tensor<std::uint8_t> temp(gaius_forward_aligned_buffer_len, -1, memory::NCHW, &memory::pool_allocator_default<std::uint8_t>::get());
					std::uint8_t* ptr = temp.mutable_cpu_data();
					for (auto i : aligned_face_array)
					{
						std::string aligned_face_base64_str = i.asString();
						if (aligned_face_base64_str.size() != TB64ENCLEN(gaius_forward_aligned_buffer_len))
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: aligned_face_base64_str.size() != TB64ENCLEN(gaius_forward_aligned_buffer_len)");

						size_t aligned_face_decode_len = tb64xdec(reinterpret_cast<const std::uint8_t*>(aligned_face_base64_str.data()), aligned_face_base64_str.size(), ptr);
						if (aligned_face_decode_len != gaius_forward_aligned_buffer_len)
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "aligned_face_decode_len != gaius_forward_aligned_buffer_len");


						for (size_t j = 0; j < 3; j++)
						{
							std::copy(mask, mask + 64 * 64, ptr + j * 128 * 128 + 64 * 128);
						}

						aligned_faces_vec.insert(aligned_faces_vec.end(), ptr, ptr + gaius_forward_aligned_buffer_len);
						num++;
					}

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"aligned_faces", box(exposing::param_span<std::uint8_t>{aligned_faces_vec.data(), aligned_faces_vec.size()})},
							{u8"num", box(num)},
							{u8"order", box(format)},
							{u8"has_mask", box(1)},
							{u8"object_id", box(instance)}
						});

					auto result = plugin.execute(u8"gaius.Forward", param).as<param_vector<param_vector<float>>>();

					Json::Value jobj_features;
					for (size_t i = 0; i < result.size(); i++)
					{
						Json::Value jarray_feature;
						for (size_t j = 0; j < result[i].size(); j++)
							jarray_feature["feature"].append(result[i][j]);
						jobj_features.append(jarray_feature);
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

			inline Json::Value Cassius_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;
				try
				{
					int device = root["device"].asInt();
					std::string models_directory = root["models_directory"].asString();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"device", box(device)},
							{u8"models_directory", box(std::string_view(models_directory))}
						});

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
			inline Json::Value Cassius_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"object_id", box(instance)}
						});

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
			inline Json::Value Cassius_Forward_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;
				try
				{
					int format = root["format"].asInt();
					if (format < 0 || format > 1)
						throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: format < 0 || format > 1");

					auto aligned_face_array = root["aligned_images"];
					std::vector<uint8_t> aligned_faces_vec;
					int num = 0;
					memory::tensor<std::uint8_t> temp(cassius_forward_aligned_buffer_len, -1, memory::NCHW, &memory::pool_allocator_default<std::uint8_t>::get());
					std::uint8_t* ptr = temp.mutable_cpu_data();
					for (auto i : aligned_face_array)
					{
						std::string aligned_face_base64_str = i.asString();
						if (aligned_face_base64_str.size() != TB64ENCLEN(cassius_forward_aligned_buffer_len))
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "Error: aligned_face_base64_str.size() != TB64ENCLEN(cassius_forward_aligned_buffer_len)");

						size_t aligned_face_decode_len = tb64xdec(reinterpret_cast<const std::uint8_t*>(aligned_face_base64_str.data()), aligned_face_base64_str.size(), ptr);
						if (aligned_face_decode_len != cassius_forward_aligned_buffer_len)
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "aligned_face_decode_len != gaius_forward_aligned_buffer_len");

						aligned_faces_vec.insert(aligned_faces_vec.end(), ptr, ptr + cassius_forward_aligned_buffer_len);
						num++;
					}

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"aligned_faces", box(exposing::param_span<std::uint8_t>{aligned_faces_vec.data(), aligned_faces_vec.size()})},
							{u8"num", box(num)},
							{u8"order", box(format)},
							{u8"object_id", box(instance)}
						});

					auto result = plugin.execute(u8"cassius.Forward", param).as<param_vector<param_vector<float>>>();

					Json::Value jobj_features;
					for (size_t i = 0; i < result.size(); i++)
					{
						Json::Value jarray_feature;
						for (size_t j = 0; j < result[i].size(); j++)
							jarray_feature["feature"].append(result[i][j]);
						jobj_features.append(jarray_feature);
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

			inline Json::Value Irisviel_new_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;

				try
				{
					//int single_database_capacity = root["single_database_capacity"].asInt();
					int single_database_capacity = 1000;
					int dimension = root["dimension"].asInt();
					std::string working_directory = root["working_directory"].asString();

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"single_database_capacity", box(single_database_capacity)},
							{u8"dimension", box(dimension)},
							{u8"working_directory", box(std::string_view(working_directory))}
						});

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
			inline Json::Value Irisviel_delete_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"object_id", box(instance)}
						});

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
			inline Json::Value Irisviel_search_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
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
					auto assuming_min_similarity = root.get("min_simiarity", Json::nullValue);
					bool has_top = assuming_top.isIntegral();
					bool has_min_similarity = assuming_min_similarity.isNumeric();

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"feature", feature},
							{u8"object_id", box(instance)}
						});

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
			inline Json::Value Irisviel_clear_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"object_id", box(instance)}
						});
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
			inline Json::Value Irisviel_remove_all_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"object_id", box(instance)}
						});
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
			inline Json::Value Irisviel_load_databases_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"object_id", box(instance)}
						});
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
			inline Json::Value Irisviel_remove_records_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;

				try
				{
					auto keys = make_param_vector<param_string>();
					auto jarray_keys = root["keys"];
					for (auto i : jarray_keys)
						keys.push_back(to_param_string(i.asString()));

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"keys", keys},
							{u8"object_id", box(instance)}
						});

					plugin.execute(u8"irisviel.remove_records", param);
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
			inline Json::Value Irisviel_remove_record_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;

				try
				{
					std::string key = root["key"].asString();

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"key", box(std::string_view(key))},
							{u8"object_id", box(instance)}
						});

					plugin.execute(u8"irisviel.remove_record", param);
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
			inline Json::Value Irisviel_add_record_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;

				try
				{
					param_vector<float> feature = make_param_vector<float>();
					auto jarray_feature = root["data"]["feature"];
					for (auto i : jarray_feature)
						feature.push_back(i.asFloat());

					int dimension = static_cast<int>(feature.size());

					std::string key = root["data"]["key"].asString();

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"dimension", box(dimension)},
							{u8"key", box(std::string_view(key))},
							{u8"feature", feature},
							{u8"object_id", box(instance)}
						});

					plugin.execute(u8"irisviel.add_record", param);
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
			inline Json::Value Irisviel_add_records_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
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
							{
								{u8"dimension", box(dimension)},
								{u8"key", box(std::string_view(key))},
								{u8"feature", feature}
							});

						vec.push_back(data);
					}

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"records", vec},
							{u8"object_id", box(instance)}
						});

					plugin.execute(u8"irisviel.add_records", param);
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
			inline Json::Value Irisviel_update_record_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
			{
				Json::Value value;

				try
				{
					param_vector<float> feature = make_param_vector<float>();
					auto jarray_feature = root["data"]["feature"];
					for (auto i : jarray_feature)
						feature.push_back(i.asFloat());

					int dimension = static_cast<int>(feature.size());

					std::string key = root["data"]["key"].asString();

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"dimension", box(dimension)},
							{u8"key", box(std::string_view(key))},
							{u8"feature", feature},
							{u8"object_id", box(instance)}
						});

					plugin.execute(u8"irisviel.update_record", param);
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
			inline Json::Value Irisviel_update_records_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, guid& instance)
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
							{
								{u8"dimension", box(dimension)},
								{u8"key", box(std::string_view(key))},
								{u8"feature", feature}
							});

						vec.push_back(data);
					}

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"records", vec},
							{u8"object_id", box(instance)}
						});

					plugin.execute(u8"irisviel.update_records", param);
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

			inline Json::Value Fusion_Romancia_alignFace_Gaius_Forward_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, std::vector<guid>& guids)
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
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame.cpu_data()), frame.count());

					auto romancia_param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(image_span)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"faces", faces },
							{u8"order", box(static_cast<int>(frame.order()))},
							{u8"object_id", box(guids[0])}
						});

					auto romancia_result = plugin.execute(u8"romancia.alignFace", romancia_param).as<param_vector<param_vector<std::uint8_t>>>();

					std::vector<std::uint8_t> buffer;
					for (size_t i = 0; i < romancia_result.size(); i++)
					{
						if (romancia_result[i].size() != gaius_forward_aligned_buffer_len)
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "romancia_result[i].size() != gaius_forward_aligned_buffer_len");

						buffer.insert(buffer.end(), begin(romancia_result[i]), end(romancia_result[i]));
					}

					auto gaius_param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"aligned_faces", box(param_span<std::uint8_t>{buffer.data(), buffer.size()})},
							{u8"num", box(static_cast<int>(romancia_result.size()))},
							{u8"order", box(0)},
							{u8"has_mask", box(has_mask ? 1 : 0)},
							{u8"object_id", box(guids[1])}
						});

					auto gaius_result = plugin.execute(u8"gaius.Forward", gaius_param).as<param_vector<param_vector<float>>>();

					Json::Value jobj_features;
					for (size_t i = 0; i < gaius_result.size(); i++)
					{
						Json::Value jarray_feature;
						for (size_t j = 0; j < gaius_result[i].size(); j++)
							jarray_feature["feature"].append(gaius_result[i][j]);
						jobj_features.append(jarray_feature);
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

			inline Json::Value Fusion_Romancia_alignFace_Cassius_Forward_json(plugin_interface& plugin, Json::Value& root, param_span<std::uint8_t>& data, std::vector<guid>& guids)
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
					param_span<std::uint8_t> image_span(const_cast<std::uint8_t*>(frame.cpu_data()), frame.count());

					auto romancia_param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(image_span)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"faces", faces },
							{u8"order", box(static_cast<int>(frame.order()))},
							{u8"object_id", box(guids[0])}
						});

					auto romancia_result = plugin.execute(u8"romancia.alignFace", romancia_param).as<param_vector<param_vector<std::uint8_t>>>();

					std::vector<std::uint8_t> buffer;
					for (size_t i = 0; i < romancia_result.size(); i++)
					{
						if (romancia_result[i].size() != cassius_forward_aligned_buffer_len)
							throw parser_exception(parser_exception::parser_exception_code::INVALID_ARGUMENT, "romancia_result[i].size() != cassius_forward_aligned_buffer_len");

						buffer.insert(buffer.end(), begin(romancia_result[i]), end(romancia_result[i]));
					}

					auto cassius_param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"aligned_faces", box(param_span<std::uint8_t>{buffer.data(), buffer.size()})},
							{u8"num", box(static_cast<int>(romancia_result.size()))},
							{u8"order", box(0)},
							{u8"object_id", box(guids[1])}
						});

					auto cassius_result = plugin.execute(u8"cassius.Forward", cassius_param).as<param_vector<param_vector<float>>>();

					Json::Value jobj_features;
					for (size_t i = 0; i < cassius_result.size(); i++)
					{
						Json::Value jarray_feature;
						for (size_t j = 0; j < cassius_result[i].size(); j++)
							jarray_feature["feature"].append(cassius_result[i][j]);
						jobj_features.append(jarray_feature);
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
		}
	}
}
#endif
