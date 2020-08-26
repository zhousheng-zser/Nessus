#ifndef _MESSAGEPROTOCOL_SIMDJSON_HPP_
#define _MESSAGEPROTOCOL_SIMDJSON_HPP_

#include "simdjson.h"
#include "json.h"
#include "base64.hpp"
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
			inline Json::Value Longinus_new_json(plugin_interface &plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;
				try
				{
					int device = static_cast<int>(root["device"].get<int64_t>().value());
					float nms = static_cast<float>(root["nms"].get<double>().value());
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"device", box(device)},
							{u8"nms", box(nms)}
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
			
			inline Json::Value Longinus_detect_json(plugin_interface &plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;
				try
				{
					std::string_view format_str = root["format"].get<std::string_view>().value();
					std::string_view image_base64_str = root["image"].get<std::string_view>().value();
					auto image_str = base64_decode(image_base64_str.data(), static_cast<std::uint32_t>(image_base64_str.length()));
					int height = static_cast<int>(root["height"].get<int64_t>().value());
					int width = static_cast<int>(root["width"].get<int64_t>().value());
					int min_size = static_cast<int>(root["min_size"].get<int64_t>().value());
					float threshold = static_cast<float>(root["threshold"].get<double>().value());
					int order = static_cast<int>(root["order"].get<int64_t>().value());

					if (image_str.size() != 3 * height * width)
						throw parser_exception("image_str.size() != 3 * height * width");

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(exposing::param_span<std::uint8_t>{reinterpret_cast<std::uint8_t*>(image_str.data()), image_str.size()})},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"min_size", box(min_size)},
							{u8"threshold", box(threshold)},
							{u8"order", box(order)},
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
			inline Json::Value Romancia_alignFace_json(plugin_interface &plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;

				try
				{
					std::string_view format_str = root["format"].get<std::string_view>().value();
					std::string_view gray_base64_str = root["gray"].get<std::string_view>().value();
					auto gray_str = base64_decode(gray_base64_str.data(), static_cast<std::uint32_t>(gray_base64_str.size()));
					int height = static_cast<int>(root["height"].get<int64_t>().value());
					int width = static_cast<int>(root["width"].get<int64_t>().value());
					auto jarray_rect = root["facerectwithfaceinfo_list"].get<simdjson::dom::array>().value();

					if (gray_str.size() != height * width)
						throw parser_exception("gray_str.size() != height * width");

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

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"gray", box(std::string_view{ gray_str.data(), gray_str.size() })},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"faces", faces},
							{u8"object_id", box(instance)}
						});

					auto result = plugin.execute(u8"romancia.alignFace", param).as<param_vector<param_vector<std::uint8_t>>>();

					value["aligned_images"] = Json::Value(Json::arrayValue);
					for (size_t i = 0; i < result.size(); i++)
					{
						std::vector<unsigned char> buffer(begin(result[i]), end(result[i]));
						std::string aligned_str = base64_encode((char*)buffer.data(), buffer.size());
						value["aligned_images"].append(Json::Value(aligned_str));
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

			inline Json::Value Gaius_new_json(plugin_interface &plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;
				try
				{
					int device = static_cast<int>(root["device"].get<int64_t>().value());
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"device", box(device)}
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
			inline Json::Value Gaius_Forward_json(plugin_interface &plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;

				try
				{
					auto aligned_face_array = root["aligned_images"].get<simdjson::dom::array>().value();
					std::string aligned_faces_str;
					int num = 0;
					for (auto i : aligned_face_array)
					{
						std::string_view aligned_face_base64_str = i.get<std::string_view>().value();
						aligned_faces_str.append(base64_decode(aligned_face_base64_str.data(), static_cast<std::uint32_t>(aligned_face_base64_str.length())));
						num++;
					}

					int order = static_cast<int>(root["order"].get<int64_t>().value());

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"aligned_faces_str", box(std::string_view(aligned_faces_str))},
							{u8"num", box(num)},
							{u8"order", box(order)},
							{u8"object_id", box(instance)}
						});

					auto result = plugin.execute(u8"gaius.Forward", param).as<param_vector<param_vector<float>>>();

					Json::Value jobj_features;
					for (int i = 0; i < result.size(); i++)
					{
						Json::Value jarray_feature;
						for (int j = 0; j < result[i].size(); j++)
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
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"device", box(device)}
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
			inline Json::Value Cassius_Forward_json(plugin_interface &plugin, simdjson::dom::element& root, guid &instance)
			{
				Json::Value value;
				try
				{
					auto aligned_face_array = root["aligned_images"].get<simdjson::dom::array>().value();
					std::string aligned_faces_str;
					int num = 0;
					for (auto i : aligned_face_array)
					{
						std::string_view aligned_face_base64_str = i.get<std::string_view>().value();
						aligned_faces_str.append(base64_decode(aligned_face_base64_str.data(), static_cast<std::uint32_t>(aligned_face_base64_str.length())));
						num++;
					}

					int order = static_cast<int>(root["order"].get<int64_t>().value());

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"aligned_faces_str", box(std::string_view(aligned_faces_str))},
							{u8"num", box(num)},
							{u8"order", box(order)},
							{u8"object_id", box(instance)}
						});

					auto result = plugin.execute(u8"cassius.Forward", param).as<param_vector<param_vector<float>>>();

					Json::Value jobj_features;
					for (int i = 0; i < result.size(); i++)
					{
						Json::Value jarray_feature;
						for (int j = 0; j < result[i].size(); j++)
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
						for (int j = 0; j < feature.size(); j++)
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
