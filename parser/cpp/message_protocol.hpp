#ifndef _MESSAGEPROTOCOL_SIMDJSON_HPP_
#define _MESSAGEPROTOCOL_SIMDJSON_HPP_

#include "simdjson.h"
#include <libyuv.h>
#include "json.h"
#include "base64_x.hpp"
#include "plugin_interface.hpp"
#include "parser_exception.hpp"
#include "../../common/include/longinus/face_info.hpp"
#include "../../common/include/irisviel/search_result.hpp"
#include "../../common/include/irisviel/record.hpp"
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

			inline int convert_to_bgr(exposing::param_span<uint8_t>& src, PROTOCOL_IMAGE_FORMAT src_format, exposing::param_span<uint8_t>& dst, int width, int height)
			{
				switch (src_format)
				{
				case PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_BGR_NCHW:
					dst = src;
					return 0;
				case PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_BGR_NHWC:
					dst = src;
					return 1;
				case PROTOCOL_IMAGE_FORMAT::PROTOCOL_IMAGE_NV21:
				{
					if (src.size() != (width * height * 3 >> 1))
						return -1;
					int aligned_src_width = (width + 1) & ~1;
					const uint8_t* y = src.data();
					const uint8_t* uv = src.data() + aligned_src_width * height;
					if (libyuv::NV21ToRGB24(y, width, uv, aligned_src_width, dst.data(), width * 3, width, height))
						return -1;
				}
					return 1;
				default:
					return -1;
				}
			}

			typedef struct image_rgb_frame
			{
				image_rgb_frame(int order, param_span<uint8_t> span):order(order), image_span(span) {}
				int order;
				param_span<uint8_t> image_span;
			}bgr_frame;

			inline image_rgb_frame decode_and_convert(std::string_view image_base64_str, uint8_t** decode_buffer, size_t* decode_buffer_len, PROTOCOL_IMAGE_FORMAT format, uint8_t** bgr_buffer, size_t* bgr_buffer_len, int width, int height)
			{
				if (height <= 0 || width <= 0)
					throw parser_exception("Invalid argument: height <= 0 || width <= 0");

				int current_image_str_len = TB64DECLEN(image_base64_str.size());
				if (current_image_str_len != *decode_buffer_len)
				{
					_vfree(*decode_buffer, *decode_buffer_len);
					*decode_buffer_len = current_image_str_len;
					*decode_buffer = static_cast<std::uint8_t*>(_valloc(*decode_buffer_len));
				}

				if (height * width * 3 != *bgr_buffer_len)
				{
					_vfree(*bgr_buffer, *bgr_buffer_len);
					*bgr_buffer_len = height * width * 3;
					*bgr_buffer = static_cast<std::uint8_t*>(_valloc(*bgr_buffer_len));
				}

				tb64xdec(reinterpret_cast<const std::uint8_t*>(image_base64_str.data()), image_base64_str.size(), *decode_buffer);

				param_span<std::uint8_t> image_src_span(*decode_buffer, *decode_buffer_len);
				param_span<std::uint8_t> image_dst_span(*bgr_buffer, *bgr_buffer_len);
				int order = convert_to_bgr(image_src_span, format, image_dst_span, width, height);
				if (order < 0)
					throw parser_exception("Error: convert_to_bgr failed");

				return { order, image_dst_span };
			}

			inline Json::Value Longinus_new_json(plugin_interface &plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;
				try
				{
					int device = static_cast<int>(root["device"].get<int64_t>().value());
					float nms = static_cast<float>(root["nms"].get<double>().value());
					std::string_view models_directory = root["models_directory"].get<std::string_view>().value();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"device", box(device)},
							{u8"nms", box(nms)},
							{u8"models_directory", box(models_directory)}
						});

					instance = unbox<guid>(plugin.execute(u8"longinus.new", param));
					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}

				return value;
			}
			inline Json::Value Longinus_delete_json(plugin_interface& plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"object_id", box(instance)}
						});

					plugin.execute(u8"longinus.delete", param);

					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				return value;
			}

			static std::uint8_t* longinus_detect_base64_decode_buffer = nullptr;
			static size_t longinus_detect_base64_decode_buffer_len = 0;
			static std::uint8_t* longinus_detect_bgr24_buffer = nullptr;
			static size_t longinus_detect_bgr24_buffer_len = 0;
			inline Json::Value Longinus_detect_json(plugin_interface &plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;
				try
				{
					int format = root["format"].get<int64_t>().value();
					std::string_view image_base64_str = root["image"].get<std::string_view>().value();
					int height = static_cast<int>(root["height"].get<int64_t>().value());
					int width = static_cast<int>(root["width"].get<int64_t>().value());
					int min_size = static_cast<int>(root["min_size"].get<int64_t>().value());
					float threshold = static_cast<float>(root["threshold"].get<double>().value());

					auto frame = decode_and_convert(image_base64_str, &longinus_detect_base64_decode_buffer, &longinus_detect_base64_decode_buffer_len,
						static_cast<PROTOCOL_IMAGE_FORMAT>(format), &longinus_detect_bgr24_buffer, &longinus_detect_bgr24_buffer_len, width, height);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(frame.image_span)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"min_size", box(min_size)},
							{u8"threshold", box(threshold)},
							{u8"order", box(frame.order)},
							{u8"object_id", box(instance)}
						});

					auto result = plugin.execute(u8"longinus.detect", param).as<param_vector<longinus::face_info>>();

					Json::Value jarray_rect = Json::Value(Json::arrayValue);

					for (auto obj : result)
					{
						Json::Value jobj_rect;
						jobj_rect["x"] = Json::Int(obj.x());
						jobj_rect["y"] = Json::Int(obj.y());
						jobj_rect["width"] = Json::Int(obj.width());
						jobj_rect["height"] = Json::Int(obj.height());
						jobj_rect["confidence"] = Json::Value(obj.confidence());
						jobj_rect["yaw"] = Json::Value(obj.yaw());
						jobj_rect["pitch"] = Json::Value(obj.pitch());
						jobj_rect["roll"] = Json::Value(obj.roll());
						Json::Value jarray_landmark;

						for (const auto &pt : obj.pts())
						{

							Json::Value jobj_point;
							jobj_point["x"] = Json::Int((int)pt.key());
							jobj_point["y"] = Json::Int((int)pt.value());
							jarray_landmark.append(jobj_point);
						}
						jobj_rect["landmark"] = jarray_landmark;
						jarray_rect.append(jobj_rect);
					}

					value["facerectwithfaceinfo_list"] = jarray_rect;
					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}
				
				return value;
			}

			inline Json::Value Longinus_trace_json(plugin_interface& plugin, simdjson::dom::element& root, guid& instance)
			{
				Json::Value value;
				try
				{
					int format = root["format"].get<int64_t>().value();
					std::string_view image_base64_str = root["image"].get<std::string_view>().value();
					int height = static_cast<int>(root["height"].get<int64_t>().value());
					int width = static_cast<int>(root["width"].get<int64_t>().value());
					auto trace_face = make_exported_interface<longinus::face_info>();
					trace_face.set_x(root["trace_face"]["x"].get<int64_t>().value());
					trace_face.set_y(root["trace_face"]["y"].get<int64_t>().value());
					trace_face.set_width(root["trace_face"]["width"].get<int64_t>().value());
					trace_face.set_height(root["trace_face"]["height"].get<int64_t>().value());

					auto frame = decode_and_convert(image_base64_str, &longinus_detect_base64_decode_buffer, &longinus_detect_base64_decode_buffer_len,
						static_cast<PROTOCOL_IMAGE_FORMAT>(format), &longinus_detect_bgr24_buffer, &longinus_detect_bgr24_buffer_len, width, height);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(frame.image_span)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"trace_face", trace_face },
							{u8"order", box(frame.order)},
							{u8"object_id", box(instance)}
						});
					auto result = plugin.execute(u8"longinus.trace", param).as<longinus::face_info>();

					Json::Value jarray_rect = Json::Value(Json::arrayValue);
					if (result.confidence() > 0.1)
					{
						Json::Value jobj_rect;
						jobj_rect["x"] = Json::Int(result.x());
						jobj_rect["y"] = Json::Int(result.y());
						jobj_rect["width"] = Json::Int(result.width());
						jobj_rect["height"] = Json::Int(result.height());
						jobj_rect["confidence"] = Json::Value(result.confidence());
						jobj_rect["yaw"] = Json::Value(result.yaw());
						jobj_rect["pitch"] = Json::Value(result.pitch());
						jobj_rect["roll"] = Json::Value(result.roll());
						Json::Value jarray_landmark;

						for (const auto& pt : result.pts())
						{

							Json::Value jobj_point;
							jobj_point["x"] = Json::Int((int)pt.key());
							jobj_point["y"] = Json::Int((int)pt.value());
							jarray_landmark.append(jobj_point);
						}
						jobj_rect["landmark"] = jarray_landmark;
						jarray_rect.append(jobj_rect);

						value["status"] = Json::Value("OK");
					}
					else
					{
						value["status"] = Json::Value("Trace failed");
					}

					value["facerectwithfaceinfo_list"] = jarray_rect;
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}

				return value;
			}

			inline Json::Value Romancia_new_json(plugin_interface& plugin, simdjson::dom::element& root, guid& instance)
			{
				Json::Value value;
				try
				{
					int device = static_cast<int>(root["device"].get<int64_t>().value());
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"device", box(device)}
						});

					instance = unbox<guid>(plugin.execute(u8"romancia.new", param));
					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}

				return value;
			}
			inline Json::Value Romancia_delete_json(plugin_interface& plugin, simdjson::dom::element& root, guid& instance)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"object_id", box(instance)}
						});

					plugin.execute(u8"romancia.delete", param);

					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				return value;
			}

			static std::uint8_t* romancia_align_base64_decode_buffer = nullptr;
			static size_t romancia_align_base64_decode_buffer_len = 0;
			static std::uint8_t* romancia_align_bgr24_buffer = nullptr;
			static size_t romancia_align_bgr24_buffer_len = 0;
			const static int romancia_align_aligned_base64_buffer_len = TB64ENCLEN(3 * 128 * 128);
			static std::uint8_t* romancia_align_aligned_base64_buffer = static_cast<std::uint8_t*>(_valloc(romancia_align_aligned_base64_buffer_len));
			inline Json::Value Romancia_alignFace_json(plugin_interface &plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;

				try
				{
					int format = root["format"].get<int64_t>().value();
					std::string_view image_base64_str = root["image"].get<std::string_view>().value();
					int height = static_cast<int>(root["height"].get<int64_t>().value());
					int width = static_cast<int>(root["width"].get<int64_t>().value());
					auto jarray_rect = root["facerectwithfaceinfo_list"].get<simdjson::dom::array>().value();
					auto faces = exposing::make_param_vector<longinus::face_info>();
					for (auto i : jarray_rect)
					{
						auto face = exposing::make_exported_interface<longinus::face_info>();
						face.set_x(static_cast<int>(i["x"].get<int64_t>().value()));
						face.set_y(static_cast<int>(i["y"].get<int64_t>().value()));
						face.set_height(static_cast<int>(i["height"].get<int64_t>().value()));
						face.set_width(static_cast<int>(i["width"].get<int64_t>().value()));

						auto landmark_list = i["landmark"].get<simdjson::dom::array>().value();

						auto landmark = exposing::make_param_vector<exposing::param_pair<float, float>>();
						for (auto j : landmark_list)
						{
							auto pair = exposing::make_param_pair(static_cast<float>(j["x"].get<int64_t>().value()), static_cast<float>(j["y"].get<int64_t>().value()));
							landmark.push_back(pair);
						}
						face.set_pts(landmark);

						faces.push_back(face);
					}

					auto frame = decode_and_convert(image_base64_str, &romancia_align_base64_decode_buffer, &romancia_align_base64_decode_buffer_len,
						static_cast<PROTOCOL_IMAGE_FORMAT>(format), &romancia_align_bgr24_buffer, &romancia_align_bgr24_buffer_len, width, height);

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(frame.image_span)},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"faces", faces},
							{u8"order", box(frame.order)},
							{u8"object_id", box(instance)}
						});

					auto result = plugin.execute(u8"romancia.alignFace", param).as<param_vector<param_vector<std::uint8_t>>>();

					value["aligned_images"] = Json::Value(Json::arrayValue);
					for (size_t i = 0; i < result.size(); i++)
					{
						std::vector<std::uint8_t> buffer(begin(result[i]), end(result[i]));

						tb64xenc(buffer.data(), buffer.size(), romancia_align_aligned_base64_buffer);

						value["aligned_images"].append(Json::Value(reinterpret_cast<char *>(romancia_align_aligned_base64_buffer), reinterpret_cast<char*>(romancia_align_aligned_base64_buffer) + romancia_align_aligned_base64_buffer_len));
					}
					value["format"] = Json::Value(0);
					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}

				return value;
			}

			inline Json::Value Gaius_new_json(plugin_interface &plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;
				try
				{
					int device = static_cast<int>(root["device"].get<int64_t>().value());
					std::string_view models_directory = root["models_directory"].get<std::string_view>().value();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"device", box(device)},
							{u8"models_directory", box(models_directory)}
						});

					instance = unbox<guid>(plugin.execute(u8"gaius.new", param));
					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}

				return value;
			}
			inline Json::Value Gaius_delete_json(plugin_interface& plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"object_id", box(instance)}
						});

					plugin.execute(u8"gaius.delete", param);

					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}

				return value;
			}

			const static int gaius_forward_aligned_buffer_len = 3 * 128 * 128;
			static std::uint8_t* gaius_forward_aligned_buffer = static_cast<std::uint8_t*>(_valloc(gaius_forward_aligned_buffer_len));
			inline Json::Value Gaius_Forward_json(plugin_interface &plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;

				try
				{
					int format = static_cast<int>(root["format"].get<int64_t>().value());
					if (format < 0 || format > 1)
						throw parser_exception("Error: format < 0 || format > 1");

					auto aligned_face_array = root["aligned_images"].get<simdjson::dom::array>().value();
					std::vector<uint8_t> aligned_faces_vec;
					int num = 0;
					for (auto i : aligned_face_array)
					{
						std::string_view aligned_face_base64_str = i.get<std::string_view>().value();
						if (aligned_face_base64_str.size() != TB64ENCLEN(gaius_forward_aligned_buffer_len))
							throw parser_exception("Error: aligned_face_base64_str.size() != TB64ENCLEN(gaius_forward_aligned_buffer_len)");

						size_t aligned_face_decode_len = tb64xdec(reinterpret_cast<const std::uint8_t*>(aligned_face_base64_str.data()), aligned_face_base64_str.size(), gaius_forward_aligned_buffer);
						aligned_faces_vec.insert(aligned_faces_vec.end(), gaius_forward_aligned_buffer, gaius_forward_aligned_buffer + gaius_forward_aligned_buffer_len);
						num++;
					}

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"aligned_faces", box(exposing::param_span<std::uint8_t>{aligned_faces_vec.data(), aligned_faces_vec.size()})},
							{u8"num", box(num)},
							{u8"order", box(format)},
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
					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}
				
				return value;
			}

			inline Json::Value Cassius_new_json(plugin_interface &plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;
				try
				{
					int device = static_cast<int>(root["device"].get<int64_t>().value());
					std::string_view models_directory = root["models_directory"].get<std::string_view>().value();
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"device", box(device)},
							{u8"models_directory", box(models_directory)}
						});

					instance = unbox<guid>(plugin.execute(u8"cassius.new", param));
					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}

				return value;
			}
			inline Json::Value Cassius_delete_json(plugin_interface& plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"object_id", box(instance)}
						});

					plugin.execute(u8"cassius.delete", param);

					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}
				return value;
			}

			static int cassius_forward_aligned_buffer_len = 3 * 128 * 128;
			static std::uint8_t* cassius_forward_aligned_buffer = static_cast<std::uint8_t*>(_valloc(cassius_forward_aligned_buffer_len));
			inline Json::Value Cassius_Forward_json(plugin_interface &plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;
				try
				{
					int format = static_cast<int>(root["format"].get<int64_t>().value());
					if (format < 0 || format > 1)
						throw parser_exception("Error: format < 0 || format > 1");

					auto aligned_face_array = root["aligned_images"].get<simdjson::dom::array>().value();
					std::vector<uint8_t> aligned_faces_vec;
					int num = 0;
					for (auto i : aligned_face_array)
					{
						std::string_view aligned_face_base64_str = i.get<std::string_view>().value();
						if (aligned_face_base64_str.size() != TB64ENCLEN(cassius_forward_aligned_buffer_len))
							throw parser_exception("Error: aligned_face_base64_str.size() != TB64ENCLEN(cassius_forward_aligned_buffer_len)");

						size_t aligned_face_decode_len = tb64xdec(reinterpret_cast<const std::uint8_t*>(aligned_face_base64_str.data()), aligned_face_base64_str.size(), cassius_forward_aligned_buffer);
						aligned_faces_vec.insert(aligned_faces_vec.end(), cassius_forward_aligned_buffer, cassius_forward_aligned_buffer + cassius_forward_aligned_buffer_len);
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
					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}

				return value;
			}

			inline Json::Value Irisviel_new_json(plugin_interface& plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;

				try
				{
					int single_database_capacity = static_cast<int>(root["single_database_capacity"].get<int64_t>().value());
					int dimension = static_cast<int>(root["dimension"].get<int64_t>().value());
					std::string_view working_directory = root["working_directory"].get<std::string_view>().value();

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"single_database_capacity", box(single_database_capacity)},
							{u8"dimension", box(dimension)},
							{u8"working_directory", box(working_directory)}
						});

					instance = unbox<guid>(plugin.execute(u8"irisviel.new", param));

					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}

				return value;
			}
			inline Json::Value Irisviel_delete_json(plugin_interface& plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"object_id", box(instance)}
						});
					
					plugin.execute(u8"irisviel.delete", param);

					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}
				return value;
			}
			inline Json::Value Irisviel_search_json(plugin_interface& plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;
				try
				{
					value["result"] = Json::Value(Json::arrayValue);

					param_vector<float> feature = make_param_vector<float>();

					auto jarray_feature = root["feature"].get<simdjson::dom::array>().value();
					for (auto i : jarray_feature)
						feature.push_back(static_cast<float>(i.get<double>().value()));
					int top = static_cast<int>(root["top"].get<int64_t>());

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"feature", feature},
							{u8"top", box(top)},
							{u8"object_id", box(instance)}
						});

					auto result = plugin.execute(u8"irisviel.search", param).as<param_vector<irisviel::search_result>>();

					for (const auto &item : result)
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

					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}

				return value;
			}
			inline Json::Value Irisviel_clear_json(plugin_interface& plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"object_id", box(instance)}
						});
					plugin.execute(u8"irisviel.clear", param);
					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}
				return value;
			}
			inline Json::Value Irisviel_remove_all_json(plugin_interface& plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"object_id", box(instance)}
						});
					plugin.execute(u8"irisviel.remove_all", param);
					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}
				
				return value;
			}
			inline Json::Value Irisviel_load_databases_json(plugin_interface& plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;
				try
				{
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"object_id", box(instance)}
						});
					plugin.execute(u8"irisviel.load_databases", param);
					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}

				return value;
			}
			inline Json::Value Irisviel_remove_records_json(plugin_interface& plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;

				try
				{
					auto keys = make_param_vector<param_string>();
					auto jarray_keys = root["keys"].get<simdjson::dom::array>().value();
					for (auto i : jarray_keys)
						keys.push_back(to_param_string(i.get<std::string_view>().value()));

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"keys", keys},
							{u8"object_id", box(instance)}
						});

					plugin.execute(u8"irisviel.remove_records", param);
					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}

				return value;
			}
			inline Json::Value Irisviel_remove_record_json(plugin_interface& plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;

				try
				{
					std::string_view key = root["key"].get<std::string_view>().value();

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"key", box(key)},
							{u8"object_id", box(instance)}
						});

					plugin.execute(u8"irisviel.remove_record", param);
					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}

				return value;
			}
			inline Json::Value Irisviel_add_record_json(plugin_interface& plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;

				try
				{
					param_vector<float> feature = make_param_vector<float>();
					auto jarray_feature = root["data"]["feature"].get<simdjson::dom::array>().value();
					for (auto i : jarray_feature)
						feature.push_back(static_cast<float>(i.get<double>().value()));

					int dimension = static_cast<int>(feature.size());

					std::string_view key = root["data"]["key"].get<std::string_view>().value();

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"dimension", box(dimension)},
							{u8"key", box(key)},
							{u8"feature", feature},
							{u8"object_id", box(instance)}
						});

					plugin.execute(u8"irisviel.add_record", param);
					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}

				return value;
			}
			inline Json::Value Irisviel_add_records_json(plugin_interface& plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;

				try
				{
					auto jarray_data = root["data"].get<simdjson::dom::array>().value();

					auto vec = make_param_vector<param_hash_map<param_string, unknown_object>>();
					for (auto i : jarray_data)
					{
						param_vector<float> feature = make_param_vector<float>();
						auto jarray_feature = i["feature"].get<simdjson::dom::array>().value();
						for (auto j : jarray_feature)
							feature.push_back(static_cast<float>(j.get<double>().value()));

						int dimension = static_cast<int>(feature.size());

						std::string_view key = i["key"].get<std::string_view>().value();

						auto data = make_param_hash_map<param_string, unknown_object>(
							{
								{u8"dimension", box(dimension)},
								{u8"key", box(key)},
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
					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}

				return value;
			}
			inline Json::Value Irisviel_update_record_json(plugin_interface& plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;

				try
				{
					param_vector<float> feature = make_param_vector<float>();
					auto jarray_feature = root["data"]["feature"].get<simdjson::dom::array>().value();
					for (auto i : jarray_feature)
						feature.push_back(static_cast<float>(i.get<double>().value()));

					int dimension = static_cast<int>(feature.size());

					std::string_view key = root["data"]["key"].get<std::string_view>().value();

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"dimension", box(dimension)},
							{u8"key", box(key)},
							{u8"feature", feature},
							{u8"object_id", box(instance)}
						});

					plugin.execute(u8"irisviel.update_record", param);
					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}

				return value;
			}
			inline Json::Value Irisviel_update_records_json(plugin_interface& plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;

				try
				{
					auto jarray_data = root["data"].get<simdjson::dom::array>().value();

					auto vec = make_param_vector<param_hash_map<param_string, unknown_object>>();
					for (auto i : jarray_data)
					{
						param_vector<float> feature = make_param_vector<float>();
						auto jarray_feature = i["feature"].get<simdjson::dom::array>().value();
						for (auto j : jarray_feature)
							feature.push_back(static_cast<float>(j.get<double>().value()));

						int dimension = static_cast<int>(feature.size());

						std::string_view key = i["key"].get<std::string_view>().value();

						auto data = make_param_hash_map<param_string, unknown_object>(
							{
								{u8"dimension", box(dimension)},
								{u8"key", box(key)},
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
					value["status"] = Json::Value("OK");
				}
				catch (const std::exception& ex)
				{
					value["status"] = Json::Value(ex.what());
				}
				catch (const abi_error& ex)
				{
					value["status"] = Json::Value(ex.what_to_narrow());
				}

				return value;
			}
		}
	}
}
#endif
