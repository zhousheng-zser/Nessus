#ifndef _MESSAGEPROTOCOL_SIMDJSON_HPP_
#define _MESSAGEPROTOCOL_SIMDJSON_HPP_

#include "simdjson.h"
#include "json.h"
#include "base64.hpp"
#include "plugin_interface.hpp"
#include "parser_exception.hpp"
#include <string>
#include <memory>
#include <unordered_map>

namespace glasssix
{
	namespace exposing
	{
		namespace nessus
		{
			inline Json::Value Longinus_new_json(plugin_interface &plugin, simdjson::dom::element& root, uint64_t &instance)
			{
				Json::Value value;
				try
				{
					int device = static_cast<int>(root["device"].get<int64_t>().value());
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"device", box(device)}
						});

					instance = unbox<uint64_t>(plugin.execute(u8"longinus.new", param));
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
			inline Json::Value Longinus_delete_json(plugin_interface& plugin, simdjson::dom::element& root, uint64_t &instance)
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
			inline Json::Value Longinus_detectEx_json(plugin_interface &plugin, simdjson::dom::element& root, uint64_t &instance)
			{
				Json::Value value;

				try
				{
					std::string_view format_str = root["format"].get<std::string_view>().value();
					std::string_view image_base64_str = root["image"].get<std::string_view>().value();
					auto image_str = base64_decode(image_base64_str.data(), static_cast<std::uint32_t>(image_base64_str.length()));
					int height = static_cast<int>(root["height"].get<int64_t>().value());
					int width = static_cast<int>(root["width"].get<int64_t>().value());
					int minSize = static_cast<int>(root["min_size"].get<int64_t>().value());
					auto jarray_threshold = root["threshold"].get<simdjson::dom::array>().value();
					std::vector<float> threshold;
					for (auto i : jarray_threshold)
						threshold.push_back(static_cast<float>(i.get<double>().value()));

					if (threshold.size() != 3)
						throw parser_exception("threshold size != 3");

					float factor = static_cast<float>(root["factor"].get<double>().value());
					int stage = static_cast<int>(root["stage"].get<int64_t>().value());
					int order = static_cast<int>(root["order"].get<int64_t>().value());

					auto threold_array_param = make_param_vector<float>(threshold[0], threshold[1], threshold[2]);
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(std::string_view{ image_str.data(), image_str.size() })},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"minSize", box(minSize)},
							{u8"threshold", threold_array_param},
							{u8"factor", box(factor)},
							{u8"stage", box(stage)},
							{u8"order", box(order)},
							{u8"object_id", box(instance)}
						});

					auto result = plugin.execute(u8"longinus.detectEx", param).as<param_vector<param_hash_map<param_string, unknown_object>>>();

					Json::Value jarray_rect = Json::Value(Json::arrayValue);

					for (auto obj : result)
					{
						Json::Value jobj_rect;
						jobj_rect["x"] = Json::Int(/*20*/unbox<int>(obj.get_value(u8"x")));
						jobj_rect["y"] = Json::Int(/*20*/unbox<int>(obj.get_value(u8"y")));
						jobj_rect["width"] = Json::Int(/*260*/unbox<int>(obj.get_value(u8"width")));
						jobj_rect["height"] = Json::Int(/*200*/unbox<int>(obj.get_value(u8"height")));
						jobj_rect["confidence"] = Json::Value(/*0.99*/unbox<float>(obj.get_value(u8"confidence")));
						jobj_rect["yaw"] = Json::Value(/*-15.2*/unbox<float>(obj.get_value(u8"yaw")));
						jobj_rect["pitch"] = Json::Value(/*10.4*/unbox<float>(obj.get_value(u8"pitch")));
						jobj_rect["roll"] = Json::Value(/*20.3*/unbox<float>(obj.get_value(u8"roll")));
						Json::Value jarray_landmark;

						auto landmark_param_vec = obj.get_value(u8"landmark").as<param_vector<int>>();
						for (int j = 0; j < 5; j++)
						{

							Json::Value jobj_point;
							jobj_point["x"] = Json::Int(/*45*/landmark_param_vec[j * 2]);
							jobj_point["y"] = Json::Int(/*45*/landmark_param_vec[j * 2 + 1]);
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
			inline Json::Value Longinus_detectRetina_json(plugin_interface &plugin, simdjson::dom::element& root, uint64_t &instance)
			{
				Json::Value value;
				try
				{
					std::string_view format_str = root["format"].get<std::string_view>().value();
					std::string_view image_base64_str = root["image"].get<std::string_view>().value();
					auto image_str = base64_decode(image_base64_str.data(), static_cast<std::uint32_t>(image_base64_str.length()));
					int height = static_cast<int>(root["height"].get<int64_t>().value());
					int width = static_cast<int>(root["width"].get<int64_t>().value());
					int min_win = static_cast<int>(root["min_size"].get<int64_t>().value());
					float threshold = static_cast<float>(root["threshold"].get<double>().value());
					int order = static_cast<int>(root["order"].get<int64_t>().value());

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"image", box(std::string_view{ image_str.data(), image_str.size() }) },
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"min_win", box(min_win)},
							{u8"threshold", box(threshold)},
							{u8"order", box(order)},
							{u8"object_id", box(instance)}
						});

					auto result = plugin.execute(u8"longinus.detectRetina", param).as<param_vector<param_hash_map<param_string, unknown_object>>>();

					Json::Value jarray_rect = Json::Value(Json::arrayValue);

					for (auto obj : result)
					{
						Json::Value jobj_rect;
						jobj_rect["x"] = Json::Int(/*20*/unbox<int>(obj.get_value(u8"x")));
						jobj_rect["y"] = Json::Int(/*20*/unbox<int>(obj.get_value(u8"y")));
						jobj_rect["width"] = Json::Int(/*260*/unbox<int>(obj.get_value(u8"width")));
						jobj_rect["height"] = Json::Int(/*200*/unbox<int>(obj.get_value(u8"height")));
						jobj_rect["confidence"] = Json::Value(/*0.99*/unbox<float>(obj.get_value(u8"confidence")));
						jobj_rect["yaw"] = Json::Value(/*-15.2*/unbox<float>(obj.get_value(u8"yaw")));
						jobj_rect["pitch"] = Json::Value(/*10.4*/unbox<float>(obj.get_value(u8"pitch")));
						jobj_rect["roll"] = Json::Value(/*20.3*/unbox<float>(obj.get_value(u8"roll")));
						Json::Value jarray_landmark;

						auto landmark_param_vec = obj.get_value(u8"landmark").as<param_vector<int>>();
						for (int j = 0; j < 5; j++)
						{

							Json::Value jobj_point;
							jobj_point["x"] = Json::Int(/*45*/landmark_param_vec[j * 2]);
							jobj_point["y"] = Json::Int(/*45*/landmark_param_vec[j * 2 + 1]);
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
			inline Json::Value Longinus_alignFace_json(plugin_interface &plugin, simdjson::dom::element& root, uint64_t &instance)
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

					auto bboxes = make_param_vector<param_vector<int>>();
					auto landmarks = make_param_vector<param_vector<int>>();

					for (auto i : jarray_rect)
					{
						auto bbox = make_param_vector<int>();
						bbox.push_back(static_cast<int>(i["x"].get<int64_t>().value()));
						bbox.push_back(static_cast<int>(i["y"].get<int64_t>().value()));
						bbox.push_back(static_cast<int>(i["height"].get<int64_t>().value()));
						bbox.push_back(static_cast<int>(i["width"].get<int64_t>().value()));
						bboxes.push_back(bbox);
						auto landmark_list = i["landmark"].get<simdjson::dom::array>().value();
						auto landmark = make_param_vector<int>();
						for (auto j : landmark_list)
						{
							landmark.push_back(static_cast<int>(j["x"].get<int64_t>().value()));
							landmark.push_back(static_cast<int>(j["y"].get<int64_t>().value()));
						}

						landmarks.push_back(landmark);
					}

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"gray", box(std::string_view{ gray_str.data(), gray_str.size() })},
							{u8"height", box(height)},
							{u8"width", box(width)},
							{u8"bboxes", bboxes},
							{u8"landmarks", landmarks},
							{u8"object_id", box(instance)}
						});

					auto result = plugin.execute(u8"longinus.alignFace", param).as<param_vector<unsigned char>>();

					value["aligned_faces"] = Json::Value(Json::arrayValue);
					for (size_t i = 0; i < bboxes.size(); i++)
					{
						std::vector<unsigned char> buffer(begin(result), end(result));
						std::string aligned_str = base64_encode((char*)buffer.data() + i * 3 * 128 * 128, 3 * 128 * 128);
						value["aligned_faces"].append(Json::Value(aligned_str));
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

			inline Json::Value Gaius_new_json(plugin_interface &plugin, simdjson::dom::element& root, uint64_t &instance)
			{
				Json::Value value;
				try
				{
					int device = static_cast<int>(root["device"].get<int64_t>().value());
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"device", box(device)}
						});

					instance = unbox<uint64_t>(plugin.execute(u8"gaius.new", param));
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
			inline Json::Value Gaius_delete_json(plugin_interface& plugin, simdjson::dom::element& root, uint64_t &instance)
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
			inline Json::Value Gaius_Forward_json(plugin_interface &plugin, simdjson::dom::element& root, uint64_t &instance)
			{
				Json::Value value;

				try
				{
					auto aligned_face_array = root["aligned_faces"].get<simdjson::dom::array>().value();
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
			inline Json::Value Cassius_new_json(plugin_interface &plugin, simdjson::dom::element& root, uint64_t &instance)
			{
				Json::Value value;
				try
				{
					int device = static_cast<int>(root["device"].get<int64_t>().value());
					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"device", box(device)}
						});

					instance = unbox<uint64_t>(plugin.execute(u8"cassius.new", param));
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
			inline Json::Value Cassius_delete_json(plugin_interface& plugin, simdjson::dom::element& root, uint64_t &instance)
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
			inline Json::Value Cassius_Forward_json(plugin_interface &plugin, simdjson::dom::element& root, uint64_t &instance)
			{
				Json::Value value;
				try
				{
					auto aligned_face_array = root["aligned_faces"].get<simdjson::dom::array>().value();
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

			inline Json::Value Irisviel_new_json(plugin_interface& plugin, simdjson::dom::element& root, uint64_t &instance)
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

					instance = unbox<uint64_t>(plugin.execute(u8"irisviel.new", param));

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
			inline Json::Value Irisviel_delete_json(plugin_interface& plugin, simdjson::dom::element& root, uint64_t &instance)
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
			inline Json::Value Irisviel_search_json(plugin_interface& plugin, simdjson::dom::element& root, uint64_t &instance)
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

					auto result = plugin.execute(u8"irisviel.search", param).as<param_vector<param_hash_map<param_string, unknown_object>>>();

					for (int i = 0; i < result.size(); i++)
					{
						Json::Value jobj_result;
						Json::Value jobj_data;

						auto feature = result[i].get_value(u8"feature").as<param_vector<float>>();
						for (int j = 0; j < feature.size(); j++)
							jobj_data["feature"].append(Json::Value(feature[j]));

						auto key = unbox<param_string>(result[i].get_value(u8"key"));
						jobj_data["key"] = Json::Value(to_narrow_string(key));

						jobj_result["data"] = jobj_data;
						jobj_result["similarity"] = Json::Value(unbox<float>(result[i].get_value(u8"similarity")));

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
			inline Json::Value Irisviel_clear_json(plugin_interface& plugin, simdjson::dom::element& root, uint64_t &instance)
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
			inline Json::Value Irisviel_remove_all_json(plugin_interface& plugin, simdjson::dom::element& root, uint64_t &instance)
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
			inline Json::Value Irisviel_load_databases_json(plugin_interface& plugin, simdjson::dom::element& root, uint64_t &instance)
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
			inline Json::Value Irisviel_remove_records_json(plugin_interface& plugin, simdjson::dom::element& root, uint64_t &instance)
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
			inline Json::Value Irisviel_remove_record_json(plugin_interface& plugin, simdjson::dom::element& root, uint64_t &instance)
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
			inline Json::Value Irisviel_add_record_json(plugin_interface& plugin, simdjson::dom::element& root, uint64_t &instance)
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
			inline Json::Value Irisviel_add_records_json(plugin_interface& plugin, simdjson::dom::element& root, uint64_t &instance)
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
			inline Json::Value Irisviel_update_record_json(plugin_interface& plugin, simdjson::dom::element& root, uint64_t &instance)
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
			inline Json::Value Irisviel_update_records_json(plugin_interface& plugin, simdjson::dom::element& root, uint64_t &instance)
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
								{u8"feature", feature},
								{u8"object_id", box(instance)}
							});

						vec.push_back(data);
					}

					auto param = make_param_hash_map<param_string, unknown_object>(
						{
							{u8"records", vec}
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
