#include "vision_service.hpp"

#include <vector>
#include <memory>
#include <cstdint>
#include <algorithm>

#include <gaius_c.h>
#include <cassius_c.h>
#include <irisviel_c.h>
#include <longinus_c.h>
#include <memory.hpp>

namespace glasssix::exposing::nessus
{
	namespace
	{
		using namespace longinus;

		unknown_object longinus_create_face_info_helper(face_rect_with_face_info* info, std::size_t size)
		{
			auto result = make_param_vector<param_hash_map<param_string, unknown_object>>();

			for (auto ptr = info; ptr < info + size; ptr++)
			{
				auto landmarks = make_param_vector<int>();

				for (std::size_t i = 0; i < sizeof(ptr->pts) / sizeof(ptr->pts[0]); i++)
				{
					landmarks.push_back(ptr->pts[i].x);
					landmarks.push_back(ptr->pts[i].y);
				}

				result.push_back(make_param_hash_map<param_string, unknown_object>(
					{
						{ u8"x", box(ptr->x) },
						{ u8"y", box(ptr->y) },
						{ u8"width", box(ptr->width) },
						{ u8"height", box(ptr->height) },
						{ u8"confidence", box(ptr->confidence) },
						{ u8"yaw", box(ptr->yaw) },
						{ u8"pitch", box(ptr->pitch) },
						{ u8"roll", box(ptr->roll) },
						{ u8"landmark", landmarks }
					}
				));
			}

			return result;
		}

		irisviel_database_record_handle irisviel_create_record_helper(const param_hash_map<param_string, unknown_object>& params)
		{
			auto dimension = unbox<int>(params.get_value(u8"dimension"));
			auto key = unbox<param_string>(params.get_value(u8"key"));
			auto feature = params.get_value(u8"feature").as<param_vector<float>>();

			return ::irisviel_create_record_with_arguments(
				dimension == 512 ? irisviel_feature_model_large : irisviel_feature_model_small,
				to_narrow_string(key).c_str(),
				std::vector<float>(begin(feature), end(feature)).data()
			);
		}

		unknown_object irisviel_add_or_update_record_helper(const param_hash_map<param_string, unknown_object>& params, bool update)
		{
			if (auto instance = reinterpret_cast<irisviel_face_service_handle>(unbox<std::uintptr_t>(params.get_value(u8"object_id"))))
			{
				auto record = irisviel_create_record_helper(params);

				update ? ::irisviel_update_record(instance, record) : ::irisviel_add_record(instance, record);
				::irisviel_free_record(record);
			}

			throw abi_null_pointer{};
		}

		unknown_object irisviel_add_or_update_records_helper(const param_hash_map<param_string, unknown_object>& params, bool update)
		{
			if (auto instance = reinterpret_cast<irisviel_face_service_handle>(unbox<std::uintptr_t>(params.get_value(u8"object_id"))))
			{
				auto records = params.get_value(u8"records").as<param_vector<param_hash_map<param_string, unknown_object>>>();
				std::vector<irisviel_database_record_handle> kernel_records(records.size());

				for (std::size_t i = 0; i < kernel_records.size(); i++)
				{
					kernel_records[i] = irisviel_create_record_helper(records[i]);
				}

				update ? ::irisviel_update_records(instance, kernel_records.data(), kernel_records.size()) : ::irisviel_update_records(instance, kernel_records.data(), kernel_records.size());
				std::for_each(kernel_records.begin(), kernel_records.end(), [](irisviel_database_record_handle record) { ::irisviel_free_record(record); });
			}

			throw abi_null_pointer{};
		}

		unknown_object longinus_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<int>(params.get_value(u8"device"));

			return box(reinterpret_cast<std::uintptr_t>(::Longinus_NewInstance(device)));
		}

		unknown_object longinus_delete(const param_hash_map<param_string, unknown_object>& params)
		{
			if (auto instance = reinterpret_cast<LonginusDetector*>(unbox<std::uintptr_t>(params.get_value(u8"object_id"))))
			{
				::Longinus_ReleaseInstance(instance);
			}

			return nullptr;
		}

		unknown_object longinus_align_face(const param_hash_map<param_string, unknown_object>& params)
		{
			if (auto instance = reinterpret_cast<LonginusDetector*>(unbox<std::uintptr_t>(params.get_value(u8"object_id"))))
			{
				auto gray = unbox<param_string>(params.get_value(u8"gray"));
				auto height = unbox<int>(params.get_value(u8"height"));
				auto width = unbox<int>(params.get_value(u8"width"));
				auto bboxes = params.get_value(u8"minSize").as<param_vector<int>>();
				auto landmarks = params.get_value(u8"threshold").as<param_vector<int>>();

				std::vector<int> kernel_bboxes(begin(bboxes), end(bboxes));
				std::vector<int> kernel_landmarks(begin(landmarks), end(landmarks));
				//llvm不支持shared_ptr数组泛型
				//std::shared_ptr<std::uint8_t[]> kernel_result{ ::Longinus_alignFace(instance, reinterpret_cast<const std::uint8_t*>(gray.data()), 1, height, width, kernel_bboxes.data(), kernel_landmarks.data()), static_cast<void(*)(void*)>(&glasssix::memory::heap_free) };
				std::unique_ptr<std::uint8_t[], void(*)(void *)> kernel_result{ ::Longinus_alignFace(instance, reinterpret_cast<const std::uint8_t*>(gray.data()), 1, height, width, kernel_bboxes.data(), kernel_landmarks.data()), static_cast<void(*)(void*)>(&glasssix::memory::heap_free) };
				auto result = make_param_vector<std::uint8_t>();

				for (std::size_t i = 0; i < 3 * 128 * 128; i++)
				{
					result.set_at(i, kernel_result[i]);
				}

				return result;
			}

			throw abi_null_pointer{};
		}

		unknown_object longinus_detect_ex(const param_hash_map<param_string, unknown_object>& params)
		{
			if (auto instance = reinterpret_cast<LonginusDetector*>(unbox<std::uintptr_t>(params.get_value(u8"object_id"))))
			{
				auto image = unbox<param_string>(params.get_value(u8"image"));
				auto height = unbox<int>(params.get_value(u8"height"));
				auto width = unbox<int>(params.get_value(u8"width"));
				auto min_size = unbox<int>(params.get_value(u8"minSize"));
				auto threshold = params.get_value(u8"threshold").as<param_vector<float>>();
				auto factor = unbox<float>(params.get_value(u8"factor"));
				auto stage = unbox<int>(params.get_value(u8"stage"));
				auto order = unbox<int>(params.get_value(u8"order"));

				face_rect_with_face_info* face_info = nullptr;
				std::vector<float> kernel_threshold(begin(threshold), end(threshold));
				std::size_t size = ::Longinus_detectEx(instance, &face_info, reinterpret_cast<const std::uint8_t*>(image.data()), height, width, min_size, kernel_threshold.data(), factor, stage, order);

				if (size == 0 || face_info == nullptr)
				{
					throw abi_invalid_argument{};
				}

				std::shared_ptr<face_rect_with_face_info> face_info_scope{ face_info, &glasssix::memory::aligned_heap_free };

				return longinus_create_face_info_helper(face_info, size);
			}

			throw abi_null_pointer{};
		}

		unknown_object longinus_detect_retina(const param_hash_map<param_string, unknown_object>& params)
		{
			if (auto instance = reinterpret_cast<LonginusDetector*>(unbox<std::uintptr_t>(params.get_value(u8"object_id"))))
			{
				auto image = unbox<param_string>(params.get_value(u8"image"));
				auto height = unbox<int>(params.get_value(u8"height"));
				auto width = unbox<int>(params.get_value(u8"width"));
				auto min_win = unbox<int>(params.get_value(u8"min_win"));
				auto threshold = unbox<float>(params.get_value(u8"threshold"));
				auto order = unbox<int>(params.get_value(u8"order"));

				face_rect_with_face_info* face_info = nullptr;
				std::size_t size = ::Longinus_detectRetina(instance, &face_info, reinterpret_cast<const std::uint8_t*>(image.data()), min_win, height, width, order, threshold);

				if (size == 0 || face_info == nullptr)
				{
					throw abi_invalid_argument{};
				}
				
				std::shared_ptr<face_rect_with_face_info> face_info_scope{ face_info, &glasssix::memory::aligned_heap_free };

				return longinus_create_face_info_helper(face_info, size);
			}

			throw abi_null_pointer{};
		}

		unknown_object cassius_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<int>(params.get_value(u8"device"));

			return box(reinterpret_cast<std::uintptr_t>(::Cassius_NewInstance(device)));
		}

		unknown_object cassius_delete(const param_hash_map<param_string, unknown_object>& params)
		{
			if (auto instance = reinterpret_cast<cassius_handle>(unbox<std::uintptr_t>(params.get_value(u8"object_id"))))
			{
				::Cassius_ReleaseInstance(instance);
			}

			return nullptr;
		}

		unknown_object cassius_extract_feature(const param_hash_map<param_string, unknown_object>& params)
		{
			if (auto instance = reinterpret_cast<cassius_handle>(unbox<std::uintptr_t>(params.get_value(u8"object_id"))))
			{
				auto aligned_faces_data = unbox<param_string>(params.get_value(u8"aligned_faces_str"));
				auto num = unbox<int>(params.get_value(u8"num"));
				auto order = unbox<int>(params.get_value(u8"order"));

				::Cassius_Forward(instance, reinterpret_cast<const std::uint8_t*>(aligned_faces_data.data()), num, order);
			}

			throw abi_null_pointer{};
		}

		unknown_object gaius_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<int>(params.get_value(u8"device"));

			return box(reinterpret_cast<std::uintptr_t>(::Gaius_NewInstance(device)));
		}

		unknown_object gaius_delete(const param_hash_map<param_string, unknown_object>& params)
		{
			if (auto instance = reinterpret_cast<gaius_handle>(unbox<std::uintptr_t>(params.get_value(u8"object_id"))))
			{
				::Cassius_ReleaseInstance(instance);
			}

			return nullptr;
		}

		unknown_object gaius_extract_feature(const param_hash_map<param_string, unknown_object>& params)
		{
			if (auto instance = reinterpret_cast<cassius_handle>(unbox<std::uintptr_t>(params.get_value(u8"object_id"))))
			{
				auto aligned_faces_data = unbox<param_string>(params.get_value(u8"aligned_faces_str"));
				auto num = unbox<int>(params.get_value(u8"num"));
				auto order = unbox<int>(params.get_value(u8"order"));

				::Gaius_Forward(instance, reinterpret_cast<const std::uint8_t*>(aligned_faces_data.data()), num, order, false);
			}

			throw abi_null_pointer{};
		}

		unknown_object irisviel_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto single_database_capacity = unbox<int>(params.get_value(u8"single_database_capacity"));
			auto dimension = unbox<int>(params.get_value(u8"dimension"));
			auto working_directory = unbox<param_string>(params.get_value(u8"working_directory"));

			return box(reinterpret_cast<std::uintptr_t>(irisviel_create_instance(single_database_capacity, dimension, to_narrow_string(working_directory).c_str())));
		}

		unknown_object irisviel_delete(const param_hash_map<param_string, unknown_object>& params)
		{
			if (auto instance = reinterpret_cast<irisviel_face_service_handle>(unbox<std::uintptr_t>(params.get_value(u8"object_id"))))
			{
				::irisviel_free_instance(instance);
			}

			return nullptr;
		}

		unknown_object irisviel_clear(const param_hash_map<param_string, unknown_object>& params)
		{
			if (auto instance = reinterpret_cast<irisviel_face_service_handle>(unbox<std::uintptr_t>(params.get_value(u8"object_id"))))
			{
				::irisviel_clear(instance);
			}

			throw abi_null_pointer{};
		}

		unknown_object irisviel_remove_all(const param_hash_map<param_string, unknown_object>& params)
		{
			if (auto instance = reinterpret_cast<irisviel_face_service_handle>(unbox<std::uintptr_t>(params.get_value(u8"object_id"))))
			{
				::irisviel_remove_all(instance);
			}

			throw abi_null_pointer{};
		}

		unknown_object irisviel_load_databases(const param_hash_map<param_string, unknown_object>& params)
		{
			if (auto instance = reinterpret_cast<irisviel_face_service_handle>(unbox<std::uintptr_t>(params.get_value(u8"object_id"))))
			{
				::irisviel_load_databases(instance);
			}

			throw abi_null_pointer{};
		}

		unknown_object irisviel_add_record(const param_hash_map<param_string, unknown_object>& params)
		{
			return irisviel_add_or_update_record_helper(params, false);
		}

		unknown_object irisviel_add_records(const param_hash_map<param_string, unknown_object>& params)
		{
			return irisviel_add_or_update_records_helper(params, false);
		}

		unknown_object irisviel_update_record(const param_hash_map<param_string, unknown_object>& params)
		{
			return irisviel_add_or_update_record_helper(params, true);
		}

		unknown_object irisviel_update_records(const param_hash_map<param_string, unknown_object>& params)
		{
			return irisviel_add_or_update_records_helper(params, true);
		}

		unknown_object irisviel_remove_record(const param_hash_map<param_string, unknown_object>& params)
		{
			if (auto instance = reinterpret_cast<irisviel_face_service_handle>(unbox<std::uintptr_t>(params.get_value(u8"object_id"))))
			{
				auto key = unbox<param_string>(params.get_value(u8"key"));

				::irisviel_remove_record(instance, to_narrow_string(key).c_str());
			}

			throw abi_null_pointer{};
		}

		unknown_object irisviel_remove_records(const param_hash_map<param_string, unknown_object>& params)
		{
			if (auto instance = reinterpret_cast<irisviel_face_service_handle>(unbox<std::uintptr_t>(params.get_value(u8"object_id"))))
			{
				auto keys = params.get_value(u8"keys").as<param_vector<param_string>>();
				std::vector<std::string> narrow_keys;
				auto kernel_keys = std::make_unique<const char* []>(keys.size());

				for (std::size_t i = 0; i < keys.size(); i++)
				{
					kernel_keys[i] = narrow_keys.emplace_back(to_narrow_string(keys[i])).c_str();
				}

				::irisviel_remove_records(instance, kernel_keys.get(), keys.size());
			}

			throw abi_null_pointer{};
		}
	}

	vision_service::vision_service()
	{
		functions_.insert_or_assign(u8"longinus.new", &longinus_new);
		functions_.insert_or_assign(u8"longinus.delete", &longinus_delete);
		functions_.insert_or_assign(u8"longinus.alignFace", &longinus_align_face);
		functions_.insert_or_assign(u8"longinus.detectEx", &longinus_detect_ex);
		functions_.insert_or_assign(u8"longinus.detectRetina", &longinus_detect_retina);
		functions_.insert_or_assign(u8"gaius.new", &gaius_new);
		functions_.insert_or_assign(u8"gaius.delete", &gaius_delete);
		functions_.insert_or_assign(u8"gaius.Forward", &gaius_extract_feature);
		functions_.insert_or_assign(u8"cassius.new", &cassius_new);
		functions_.insert_or_assign(u8"cassius.delete", &cassius_delete);
		functions_.insert_or_assign(u8"cassius.Forward", &cassius_extract_feature);
		functions_.insert_or_assign(u8"irisviel.new", &irisviel_new);
		functions_.insert_or_assign(u8"irisviel.delete", &irisviel_delete);
		functions_.insert_or_assign(u8"irisviel.clear", &irisviel_clear);
		functions_.insert_or_assign(u8"irisviel.remove_all", &irisviel_remove_all);
		functions_.insert_or_assign(u8"irisviel.load_databases", &irisviel_load_databases);
		functions_.insert_or_assign(u8"irisviel.add_record", &irisviel_add_record);
		functions_.insert_or_assign(u8"irisviel.add_records", &irisviel_add_records);
		functions_.insert_or_assign(u8"irisviel.update_record", &irisviel_update_record);
		functions_.insert_or_assign(u8"irisviel.update_records", &irisviel_update_records);
		functions_.insert_or_assign(u8"irisviel.remove_record", &irisviel_remove_record);
		functions_.insert_or_assign(u8"irisviel.remove_records", &irisviel_remove_records);
	}

	param_string vision_service::name() const
	{
		return u8"Glasssix Vision Service";
	}

	param_string vision_service::version() const
	{
		return u8"1.0.0";
	}

	param_vector<param_string> vision_service::get_available_functions() const
	{
		auto result = make_param_vector<param_string>();

		for (auto [key, value] : functions_)
		{
			result.push_back(key);
		}

		return result;
	}

	unknown_object vision_service::execute(const param_string& function_name, const param_hash_map<param_string, unknown_object>& params) const
	{
		auto iter = functions_.find(function_name);

		return iter != functions_.end() ? iter->second(params) : throw abi_key_not_found{};
	}
}
