#pragma once
#include "vision_service_impl.hpp"
#include <mutex>
#include <functional>
#include <unordered_map>
#include <iostream>

#include <longinus/facedetector.hpp>
#include <irisviel/face_service.hpp>
#include <romancia/face_alignment.hpp>
#include <gaius/feature_extractor.hpp>
#include <cassius/feature_extractor.hpp>
#include <damocles/anti_spoofing.hpp>
#include <selene/feature_extractor.hpp>

using namespace glasssix::gaius;
using namespace glasssix::cassius;
using namespace glasssix::romancia;
using namespace glasssix::irisviel;
using namespace glasssix::longinus;
using namespace glasssix::damocles;
using namespace glasssix::selene;

namespace glasssix::exposing::nessus
{
	namespace
	{
		struct package_names
		{
			static constexpr utf8_string_view gaius{ u8"gaius" };
			static constexpr utf8_string_view cassius{ u8"cassius" };
			static constexpr utf8_string_view longinus{ u8"longinus" };
			static constexpr utf8_string_view romancia{ u8"romancia" };
			static constexpr utf8_string_view irisviel{ u8"irisviel" };
			static constexpr utf8_string_view damocles{ u8"damocles" };
			static constexpr utf8_string_view selene{ u8"selene" };
		};

		struct function_names final
		{
			static constexpr utf8_string_view gaius_new{ u8"gaius.new" };
			static constexpr utf8_string_view cassius_new{ u8"cassius.new" };
			static constexpr utf8_string_view longinus_new{ u8"longinus.new" };
			static constexpr utf8_string_view damocles_new{ u8"damocles.new" };
			static constexpr utf8_string_view romancia_new{ u8"romancia.new" };
			static constexpr utf8_string_view irisviel_new{ u8"irisviel.new" };
			static constexpr utf8_string_view selene_new{ u8"selene.new" };					
			static constexpr utf8_string_view gaius_delete{ u8"gaius.delete" };
			static constexpr utf8_string_view cassius_delete{ u8"cassius.delete" };
			static constexpr utf8_string_view longinus_delete{ u8"longinus.delete" };
			static constexpr utf8_string_view damocles_delete{ u8"damocles.delete" };
			static constexpr utf8_string_view romancia_delete{ u8"romancia.delete" };
			static constexpr utf8_string_view irisviel_delete{ u8"irisviel.delete" };
			static constexpr utf8_string_view selene_delete{ u8"selene.delete" };			
			static constexpr utf8_string_view gaius_forward{ u8"gaius.forward" };
			static constexpr utf8_string_view cassius_forward{ u8"cassius.forward" };
			static constexpr utf8_string_view selene_forward{ u8"selene.forward" };
			static constexpr utf8_string_view selene_get_model_type{ u8"selene.get_model_type" };
			static constexpr utf8_string_view longinus_detect{ u8"longinus.detect" };
			static constexpr utf8_string_view longinus_trace{ u8"longinus.trace" };
			static constexpr utf8_string_view longinus_center_scale_align_face{ u8"longinus.center_scale_alignFace" };
			static constexpr utf8_string_view damocles_spoofing_detect{ u8"damocles.spoofing_detect" };
			static constexpr utf8_string_view damocles_presentation_attack_detect{ u8"damocles.presentation_attack_detect" };
			static constexpr utf8_string_view romancia_align_face_128{ u8"romancia.alignFace128" };
			static constexpr utf8_string_view romancia_align_face{ u8"romancia.alignFace" };
			static constexpr utf8_string_view romancia_blur_detect{ u8"romancia.blur_detect" };
			static constexpr utf8_string_view romancia_mask_detect{ u8"romancia.mask_detect" };
			static constexpr utf8_string_view romancia_rotate{ u8"romancia.rotate" };
			static constexpr utf8_string_view irisviel_clear{ u8"irisviel.clear" };
			static constexpr utf8_string_view irisviel_remove_all{ u8"irisviel.remove_all" };
			static constexpr utf8_string_view irisviel_load_databases{ u8"irisviel.load_databases" };
			static constexpr utf8_string_view irisviel_record_count{ u8"irisviel.record_count" };
			static constexpr utf8_string_view irisviel_contains_key{ u8"irisviel.contains_key" };
			static constexpr utf8_string_view irisviel_try_get_record{ u8"irisviel.try_get_record" };
			static constexpr utf8_string_view irisviel_add_record{ u8"irisviel.add_record" };
			static constexpr utf8_string_view irisviel_add_records{ u8"irisviel.add_records" };
			static constexpr utf8_string_view irisviel_update_record{ u8"irisviel.update_record" };
			static constexpr utf8_string_view irisviel_update_records{ u8"irisviel.update_records" };
			static constexpr utf8_string_view irisviel_remove_record{ u8"irisviel.remove_record" };
			static constexpr utf8_string_view irisviel_remove_records{ u8"irisviel.remove_records" };
			static constexpr utf8_string_view irisviel_search{ u8"irisviel.search" };
			static constexpr utf8_string_view irisviel_search_nf{ u8"irisviel.search_nf" };
		};
	}


	class vision_service_impl::impl
	{
	public:

		impl();

		param_string name() const;

		param_string version() const;

		param_vector<param_string> available_functions() const;

		param_hash_map<guid, param_string> existing_instances() const;

		unknown_object execute(const param_string& function_name, const param_hash_map<param_string, unknown_object>& params) const;

		// =========================================
		static unknown_object cassius_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto model_type = unbox<std::int32_t>(params.get_value(u8"model_type"));
			auto use_int8 = unbox<std::int32_t>(params.get_value(u8"use_int8"));
			auto racy_path = unbox<param_string>(params.get_value(u8"models_directory"));
			// auto racy_path = model_type ? models_directory + (use_int8 ? u8"/unicorn_res101_int8.racy" : u8"/unicorn_res101.racy") : models_directory + (use_int8 ? u8"/unicorn_int8.racy" : u8"/unicorn.racy");
			return add_instance(package_names::cassius, make_exported_interface<cassius::feature_extractor>(model_type, racy_path, device, use_int8 ? true : false));
		}

		static unknown_object gaius_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto use_int8 = unbox<std::int32_t>(params.get_value(u8"use_int8"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::gaius, make_exported_interface<gaius::feature_extractor>(models_directory + (use_int8 ? u8"/mobile_unicorn_int8.racy" : u8"/mobile_unicorn.racy"),
				models_directory + (use_int8 ? u8"/mobile_unicorn_mask_int8.racy" : u8"/mobile_unicorn_mask.racy"), device, use_int8 ? true : false));
		}

		static unknown_object selene_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto model_type = unbox<std::int32_t>(params.get_value(u8"model_type"));
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto use_int8 = unbox<std::int32_t>(params.get_value(u8"use_int8"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::selene, make_exported_interface<selene::feature_extractor>(models_directory, model_type, device, use_int8 ? true : false));
		}

		static unknown_object longinus_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto algo_type = unbox<std::int32_t>(params.get_value(u8"algo_type"));
			auto model_type = unbox<std::int32_t>(params.get_value(u8"model_type"));
			auto nms = unbox<float>(params.get_value(u8"nms"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::longinus, make_exported_interface<facedetector>(models_directory, algo_type, model_type, nms, device));
		}

		static 	unknown_object damocles_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto model_type = unbox<std::int32_t>(params.get_value(u8"model_type"));
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::damocles, make_exported_interface<damocles::anti_spoofing>(models_directory, model_type, device));
		}

		static unknown_object romancia_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

            return add_instance(package_names::romancia, make_exported_interface<face_alignment>(models_directory + u8"/blur_detection_best.racy", device));
        }

		static unknown_object irisviel_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto single_database_capacity = unbox<std::int32_t>(params.get_value(u8"single_database_capacity"));
			auto dimension = unbox<std::int32_t>(params.get_value(u8"dimension"));
			auto working_directory = unbox<param_string>(params.get_value(u8"working_directory"));

			return add_instance(package_names::irisviel, make_exported_interface<face_service>(irisviel::face_service_implemention::brute_force, single_database_capacity, dimension, working_directory));
		}

				static unknown_object cassius_extract_feature(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<cassius::feature_extractor>(params);
			auto aligned_faces = unbox<param_span<std::uint8_t>>(params.get_value(u8"aligned_faces"));
			auto num = unbox<std::int32_t>(params.get_value(u8"num"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.get(aligned_faces, num, order);
		}

		static unknown_object gaius_extract_feature(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<gaius::feature_extractor>(params);
			auto aligned_faces = unbox<param_span<std::uint8_t>>(params.get_value(u8"aligned_faces"));
			auto num = unbox<std::int32_t>(params.get_value(u8"num"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto has_mask = unbox<std::int32_t>(params.get_value(u8"has_mask"));

			return instance.get(aligned_faces, num, order, has_mask ? true : false);
		}

		static unknown_object selene_extract_feature(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<selene::feature_extractor>(params);
			auto aligned_faces = unbox<param_span<std::uint8_t>>(params.get_value(u8"aligned_faces"));
			auto num = unbox<std::int32_t>(params.get_value(u8"num"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.get(aligned_faces, num, order);
		}

		static unknown_object selene_get_model_type(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<selene::feature_extractor>(params);

			return box(instance.get_model_type());
		}

		static unknown_object longinus_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<facedetector>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto min_size = unbox<std::int32_t>(params.get_value(u8"min_size"));
			auto threshold = unbox<float>(params.get_value(u8"threshold"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto do_attributing = unbox<bool>(params.get_value(u8"do_attributing"));

			return instance.detect(image, channels, height, width, min_size, threshold, order, do_attributing);
		}

		static unknown_object damocles_spoofing_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<anti_spoofing>(params);
			auto faces = params.get_value(u8"faces").as<param_vector<face_info>>();
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.spoofing_detect(faces, image, channels, height, width, order);
		}

		static unknown_object damocles_presentation_attack_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<anti_spoofing>(params);
			auto action_cmd = unbox<std::int32_t>(params.get_value(u8"action_cmd"));
			auto face = params.get_value(u8"face").as<face_info>();
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return box(instance.presentation_attack_detect(action_cmd, face, image, channels, height, width, order));
		}

		static unknown_object longinus_trace(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<facedetector>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto face = params.get_value(u8"face").as<face_info>();
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto result = instance.single_trace(face, image, channels, height, width, order);

			return result;
		}

		static unknown_object longinus_center_scale_align_face(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<facedetector>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto scale = unbox<float>(params.get_value(u8"scale"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.center_scale_align(image, channels, height, width, scale, order);
		}

		static unknown_object romancia_align_face_128(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<face_alignment>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto faces = params.get_value(u8"faces").as<param_vector<face_info>>();
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.align128(image, channels, height, width, faces, order);
		}

		static unknown_object romancia_align_face(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<face_alignment>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto faces = params.get_value(u8"faces").as<param_vector<face_info>>();
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.align(image, channels, height, width, faces, order);
		}

		static unknown_object romancia_blur_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<face_alignment>(params);
			auto faces = params.get_value(u8"faces").as<param_vector<face_info>>();
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.blur_detect(faces, image, channels, height, width, order);
		}

		static unknown_object romancia_mask_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<face_alignment>(params);
			auto faces = params.get_value(u8"faces").as<param_vector<face_info>>();
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.mask_detect(faces, image, channels, height, width, order);
		}

		static unknown_object romancia_rotate(const param_hash_map<param_string, unknown_object>& params)
        {
            constexpr std::int32_t channels = 3;
            auto instance = get_instance<face_alignment>(params);
            auto angle = unbox<float>(params.get_value(u8"angle"));
            auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
            auto height = unbox<std::int32_t>(params.get_value(u8"height"));
            auto width = unbox<std::int32_t>(params.get_value(u8"width"));
            auto order = unbox<std::int32_t>(params.get_value(u8"order"));

            return instance.rotate(angle, image, channels, height, width, order);
        }

		static void irisviel_clear(const param_hash_map<param_string, unknown_object>& params)
		{
			get_instance<face_service>(params).clear();
		}

		static void irisviel_remove_all(const param_hash_map<param_string, unknown_object>& params)
		{
			get_instance<face_service>(params).remove_all();
		}

		static void irisviel_load_databases(const param_hash_map<param_string, unknown_object>& params)
		{
			get_instance<face_service>(params).load_databases();
		}

		static unknown_object irisviel_record_count(const param_hash_map<param_string, unknown_object>& params)
		{
			return box(get_instance<face_service>(params).record_count());
		}

		static unknown_object irisviel_contains_key(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<face_service>(params);
			auto key = unbox<param_string>(params.get_value(u8"key"));

			return box(get_instance<face_service>(params).contains_key(key));
		}

		static unknown_object irisviel_try_get_record(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<face_service>(params);
			auto key = unbox<param_string>(params.get_value(u8"key"));

			return get_instance<face_service>(params).try_get_record(key);
		}

		static exposing::param_vector<bool> irisviel_add_records(const param_hash_map<param_string, unknown_object>& params)
		{
			return irisviel_add_or_update_records_helper(params, false);
		}

		static exposing::param_vector<bool> irisviel_update_records(const param_hash_map<param_string, unknown_object>& params)
		{
			return irisviel_add_or_update_records_helper(params, true);
		}

		static exposing::param_vector<bool> irisviel_remove_records(const param_hash_map<param_string, unknown_object>& params)
		{
			auto keys = params.get_value(u8"keys").as<param_vector<param_string>>();

			return get_instance<face_service>(params).remove_records(keys);
		}

		static unknown_object irisviel_search(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<face_service>(params);
			auto feature = params.get_value(u8"feature").as<param_vector<float>>();

			unknown_object assuming_top{ nullptr };
			unknown_object assuming_min_similarity{ nullptr };
			bool has_top = params.try_get_value(u8"top", assuming_top) && assuming_top;
			bool has_min_similarity = params.try_get_value(u8"min_similarity", assuming_min_similarity) && assuming_min_similarity;

			if (has_top && has_min_similarity)
			{
				return instance.search(feature, unbox<float>(assuming_min_similarity), unbox<std::uint32_t>(assuming_top));
			}

			if (has_top)
			{
				return instance.search(feature, unbox<std::uint32_t>(assuming_top));
			}

			if (has_min_similarity)
			{
				return instance.search(feature, unbox<float>(assuming_min_similarity));
			}

			throw abi_invalid_argument{ "Missing required parameters: top or min_similarity." };
		}

		static unknown_object irisviel_search_nf(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<face_service>(params);
			auto feature = params.get_value(u8"feature").as<param_vector<float>>();

			unknown_object assuming_top{ nullptr };
			unknown_object assuming_min_similarity{ nullptr };
			bool has_top = params.try_get_value(u8"top", assuming_top) && assuming_top;
			bool has_min_similarity = params.try_get_value(u8"min_similarity", assuming_min_similarity) && assuming_min_similarity;

			if (has_top && has_min_similarity)
			{
				return instance.search_nf(feature, unbox<float>(assuming_min_similarity), unbox<std::uint32_t>(assuming_top));
			}

			throw abi_invalid_argument{ "Missing required parameters: top and min_similarity." };
		}

		static record irisviel_create_record_helper(const param_hash_map<param_string, unknown_object>& params)
		{
			auto dimension = unbox<std::int32_t>(params.get_value(u8"dimension"));
			auto key = unbox<param_string>(params.get_value(u8"key"));
			auto feature = params.get_value(u8"feature").as<param_vector<float>>();
			auto result = make_exported_interface<record>(dimension);

			result.key(key);
			result.feature(feature);

			return result;
		}

		static exposing::param_vector<bool> irisviel_add_or_update_records_helper(const param_hash_map<param_string, unknown_object>& params, bool update)
		{
			auto instance = get_instance<face_service>(params);
			auto param_records = params.get_value(u8"records").as<param_vector<param_hash_map<param_string, unknown_object>>>();
			auto records = make_param_vector<record>();

			for (const auto& item : param_records)
			{
				records.push_back(irisviel_create_record_helper(item));
			}

			if (update)
			{
				return instance.update_records(records);
			}
			else
			{
				return instance.add_records(records);
			}
		}

		// =========================================
		static unknown_object add_instance(utf8_string_view package_name, const unknown_object& instance)
		{
			auto id = create_guid_from_bytes(meta::to_array(reinterpret_cast<std::size_t>(get_abi(instance))));
			{
				std::scoped_lock lock{ mutex_ };

				return (instances_.insert_or_assign(id, std::tuple{ package_name, instance }), box(id));
			}
		}

		static void delete_instance_by_id(const guid& id)
		{
			std::scoped_lock lock{ mutex_ };

			if (auto iter = instances_.find(id); iter != instances_.end())
			{
				instances_.erase(iter);
			}
		}

		static void delete_instance(const param_hash_map<param_string, unknown_object>& params)
		{
			delete_instance_by_id(get_instance_id(params));
		}

		static guid get_instance_id(const param_hash_map<param_string, unknown_object>& params)
		{
			return unbox<guid>(params.get_value(u8"object_id"));
		}

		template <typename T>
		static T get_instance(const guid& id)
		{
			std::scoped_lock lock{ mutex_ };
			auto iter = instances_.find(id);

			return iter != instances_.end() ? std::get<unknown_object>(iter->second).as<T>() : throw abi_key_not_found{ exposing::format(u8"Cannot find instance: {}.", to_param_string(id)) };
		}

		template <typename T>
		static T get_instance(const param_hash_map<param_string, unknown_object>& params)
		{
			return get_instance<T>(unbox<guid>(params.get_value(u8"object_id")));
		}

		inline static std::mutex mutex_;
		inline static std::unordered_map<guid, std::tuple<param_string, unknown_object>> instances_;
		inline static std::unordered_map<param_string, std::function<unknown_object(const param_hash_map<param_string, unknown_object>&)>> functions_;
	};


}
