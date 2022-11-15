#include "vision_service_impl.hpp"

#include <mutex>
#include <functional>
#include <unordered_map>

#include <longinus/retina_net.hpp>
#include <irisviel/face_service.hpp>
#include <romancia/face_alignment.hpp>
#include <gaius/feature_extractor.hpp>
#include <cassius/feature_extractor.hpp>
#include <damocles/anti_spoofing.hpp>
#include <selene/feature_extractor.hpp>
#include <gungnir/yolo_net.hpp>
#include <mjollner/ocr_net.hpp>
#include <valklyrs/yolov5s_net.hpp>
#include <heimdall/material_code.hpp>
#include <banshee/kcf_tracker.hpp>
#include <ring/material_code.hpp>
#include <plate/ocr_code.hpp>

#include <iostream>

using namespace glasssix::gaius;
using namespace glasssix::cassius;
using namespace glasssix::romancia;
using namespace glasssix::irisviel;
using namespace glasssix::longinus;
using namespace glasssix::damocles;
using namespace glasssix::selene;
using namespace glasssix::gungnir;
using namespace glasssix::mjollner;
using namespace glasssix::valklyrs;
using namespace glasssix::heimdall;
using namespace glasssix::banshee;
using namespace glasssix::plate;

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
			static constexpr utf8_string_view gungnir{ u8"gungnir" };
			static constexpr utf8_string_view mjollner{ u8"mjollner" };
			static constexpr utf8_string_view valklyrs{ u8"valklyrs" };
			static constexpr utf8_string_view heimdall{ u8"heimdall" };
			static constexpr utf8_string_view banshee{ u8"banshee" };
			static constexpr utf8_string_view ring{ u8"ring" };
			static constexpr utf8_string_view plate{ u8"plate" };
		};

		struct function_names final
		{
			static constexpr utf8_string_view plate_new{ u8"plate.new" };
			static constexpr utf8_string_view plate_detect{ u8"plate.detect" };
			static constexpr utf8_string_view plate_trace_init{ u8"plate.trace_init" };
			static constexpr utf8_string_view plate_trace_update{ u8"plate.trace_update" };
			static constexpr utf8_string_view plate_recognize{ u8"plate.recognize" };
			static constexpr utf8_string_view plate_delete{ u8"plate.delete" };
			static constexpr utf8_string_view ring_new{ u8"ring.new" };
			static constexpr utf8_string_view ring_detect{ u8"ring.detect" };
			static constexpr utf8_string_view ring_delete{ u8"ring.delete" };
			static constexpr utf8_string_view gaius_new{ u8"gaius.new" };
			static constexpr utf8_string_view cassius_new{ u8"cassius.new" };
			static constexpr utf8_string_view longinus_new{ u8"longinus.new" };
			static constexpr utf8_string_view damocles_new{ u8"damocles.new" };
			static constexpr utf8_string_view romancia_new{ u8"romancia.new" };
			static constexpr utf8_string_view irisviel_new{ u8"irisviel.new" };
			static constexpr utf8_string_view selene_new{ u8"selene.new" };
			static constexpr utf8_string_view selene_new_test{ u8"selene.new.test" };
			static constexpr utf8_string_view gungnir_new{ u8"gungnir.new" };
			static constexpr utf8_string_view mjollner_new{ u8"mjollner.new" };
			static constexpr utf8_string_view valklyrs_new{ u8"valklyrs.new" };
			static constexpr utf8_string_view heimdall_new{ u8"heimdall.new" };
			static constexpr utf8_string_view banshee_new{ u8"banshee.new" };
			static constexpr utf8_string_view gaius_delete{ u8"gaius.delete" };
			static constexpr utf8_string_view cassius_delete{ u8"cassius.delete" };
			static constexpr utf8_string_view longinus_delete{ u8"longinus.delete" };
			static constexpr utf8_string_view damocles_delete{ u8"damocles.delete" };
			static constexpr utf8_string_view romancia_delete{ u8"romancia.delete" };
			static constexpr utf8_string_view irisviel_delete{ u8"irisviel.delete" };
			static constexpr utf8_string_view selene_delete{ u8"selene.delete" };
			static constexpr utf8_string_view gungnir_delete{ u8"gungnir.delete" };
			static constexpr utf8_string_view mjollner_delete{ u8"mjollner.delete" };
			static constexpr utf8_string_view valklyrs_delete{ u8"valklyrs.delete" };
			static constexpr utf8_string_view heimdall_delete{ u8"heimdall.delete" };
			static constexpr utf8_string_view banshee_delete{ u8"banshee.delete" };
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
			static constexpr utf8_string_view gungnir_detect{ u8"gungnir.detect" };
			static constexpr utf8_string_view mjollner_detect{ u8"mjollner.detect" };
			static constexpr utf8_string_view valklyrs_detect{ u8"valklyrs.detect" };
			static constexpr utf8_string_view heimdall_detect{ u8"heimdall.detect" };
			static constexpr utf8_string_view banshee_init{ u8"banshee.init" };
			static constexpr utf8_string_view banshee_update{ u8"banshee.update" };
		};
	}

	class vision_service_impl::impl
	{
	public:
		impl()
		{
			// New
			functions_.insert_or_assign(function_names::plate_new, std::bind(&impl::plate_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::ring_new, std::bind(&impl::ring_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::gaius_new, std::bind(&impl::gaius_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::cassius_new, std::bind(&impl::cassius_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::longinus_new, std::bind(&impl::longinus_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::damocles_new, std::bind(&impl::damocles_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::romancia_new, std::bind(&impl::romancia_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::irisviel_new, std::bind(&impl::irisviel_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::selene_new, std::bind(&impl::selene_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::selene_new_test, std::bind(&impl::selene_new_test, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::gungnir_new, std::bind(&impl::gungnir_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::mjollner_new, std::bind(&impl::mjollner_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::valklyrs_new, std::bind(&impl::valklyrs_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::heimdall_new, std::bind(&impl::heimdall_new, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::banshee_new, std::bind(&impl::banshee_new, this, std::placeholders::_1));

			// Delete
			functions_.insert_or_assign(function_names::plate_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::ring_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::gaius_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::cassius_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::longinus_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::damocles_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::romancia_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::selene_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::gungnir_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::mjollner_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::valklyrs_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::heimdall_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::banshee_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, this, std::placeholders::_1)));

			// Business
			functions_.insert_or_assign(function_names::plate_detect, std::bind(&impl::plate_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::plate_trace_init, std::bind(&impl::plate_trace_init, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::plate_trace_update, std::bind(&impl::plate_trace_update, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::plate_recognize, std::bind(&impl::plate_recognize, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::ring_detect, std::bind(&impl::ring_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::damocles_spoofing_detect, std::bind(&impl::damocles_spoofing_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::damocles_presentation_attack_detect, std::bind(&impl::damocles_presentation_attack_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::longinus_detect, std::bind(&impl::longinus_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::gungnir_detect, std::bind(&impl::gungnir_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::mjollner_detect, std::bind(&impl::mjollner_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::valklyrs_detect, std::bind(&impl::valklyrs_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::heimdall_detect, std::bind(&impl::heimdall_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::banshee_init, std::bind(&impl::banshee_init, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::banshee_update, std::bind(&impl::banshee_update, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::longinus_trace, std::bind(&impl::longinus_trace, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::longinus_center_scale_align_face, std::bind(&impl::longinus_center_scale_align_face, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::romancia_align_face_128, std::bind(&impl::romancia_align_face_128, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::romancia_align_face, std::bind(&impl::romancia_align_face, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::romancia_blur_detect, std::bind(&impl::romancia_blur_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::romancia_mask_detect, std::bind(&impl::romancia_mask_detect, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::romancia_rotate, std::bind(&impl::romancia_rotate, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::gaius_forward, std::bind(&impl::gaius_extract_feature, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::cassius_forward, std::bind(&impl::cassius_extract_feature, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::selene_forward, std::bind(&impl::selene_extract_feature, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::selene_get_model_type, std::bind(&impl::selene_get_model_type, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::irisviel_clear, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_clear, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_remove_all, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_remove_all, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_load_databases, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_load_databases, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_record_count, std::bind(&impl::irisviel_record_count, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::irisviel_contains_key, std::bind(&impl::irisviel_contains_key, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::irisviel_try_get_record, std::bind(&impl::irisviel_try_get_record, this, std::placeholders::_1));
			functions_.insert_or_assign(function_names::irisviel_add_record, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_add_record, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_add_records, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_add_records, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_update_record, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_update_record, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_update_records, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_update_records, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_remove_record, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_remove_record, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_remove_records, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_remove_records, this, std::placeholders::_1)));
			functions_.insert_or_assign(function_names::irisviel_search, std::bind(&impl::irisviel_search, this, std::placeholders::_1));
		}

		~impl()
		{
			std::cout << "instances_.size: " << instances_.size() << std::endl;
		}

		param_string name() const
		{
			return u8"Glasssix Vision Service";
		}

		param_string version() const
		{
			return u8"1.0.0";
		}

		param_vector<param_string> available_functions() const
		{
			auto result = make_param_vector<param_string>();

			for (const auto& [key, value] : functions_)
			{
				result.push_back(key);
			}

			return result;
		}

		param_hash_map<guid, param_string> existing_instances() const
		{
			auto result = make_param_hash_map<guid, param_string>();

			for (const auto& [key, value] : instances_)
			{
				result.add_or_update(key, std::get<param_string>(value));
			}

			return result;
		}

		unknown_object execute(const param_string& function_name, const param_hash_map<param_string, unknown_object>& params) const
		{
			auto iter = functions_.find(function_name);

			return iter != functions_.end() ? iter->second(params) : throw abi_key_not_found{ function_name };
		}

	private:
		unknown_object plate_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::plate, make_exported_interface<plate::ocr_code>(models_directory, device));
		}

		unknown_object ring_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto factory_type = unbox<std::int32_t>(params.get_value(u8"factory_type"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::ring, make_exported_interface<ring::material_code>(models_directory, factory_type, device));
		}

		unknown_object cassius_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto model_type = unbox<std::int32_t>(params.get_value(u8"model_type"));
			auto use_int8 = unbox<std::int32_t>(params.get_value(u8"use_int8"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			auto racy_path = model_type ? models_directory + (use_int8 ? u8"/unicorn_res101_int8.racy" : u8"/unicorn_res101.racy") : models_directory + (use_int8 ? u8"/unicorn_int8.racy" : u8"/unicorn.racy");
			return add_instance(package_names::cassius, make_exported_interface<cassius::feature_extractor>(model_type, racy_path, device, use_int8 ? true : false));
		}

		unknown_object gaius_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto use_int8 = unbox<std::int32_t>(params.get_value(u8"use_int8"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::gaius, make_exported_interface<gaius::feature_extractor>(models_directory + (use_int8 ? u8"/mobile_unicorn_int8.racy" : u8"/mobile_unicorn.racy"),
				models_directory + (use_int8 ? u8"/mobile_unicorn_mask_int8.racy" : u8"/mobile_unicorn_mask.racy"), device, use_int8 ? true : false));
		}

		unknown_object selene_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto model_type = unbox<std::int32_t>(params.get_value(u8"model_type"));
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto use_int8 = unbox<std::int32_t>(params.get_value(u8"use_int8"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			param_string model_name = u8"";
			switch (model_type)
			{
			case 0:
				model_name = u8"unicorn_light_universal";
				break;
			case 1:
				model_name = u8"unicorn_light_id";
				break;
			case 2:
				model_name = u8"unicorn_light_universal_mask";
				break;
			default:
				throw abi_invalid_argument("Invalid model_type value. ");
				break;
			}

			return add_instance(package_names::selene, make_exported_interface<selene::feature_extractor>(models_directory + u8"/" + model_name + (use_int8 ? +u8"_int8.racy" : u8".racy"), model_type, device, use_int8 ? true : false));
		}

		unknown_object selene_new_test(const param_hash_map<param_string, unknown_object>& params)
		{
			auto model_path = unbox<param_string>(params.get_value(u8"model_path"));
			auto model_type = unbox<std::int32_t>(params.get_value(u8"model_type"));
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto use_int8 = unbox<std::int32_t>(params.get_value(u8"use_int8"));

			return add_instance(package_names::selene, make_exported_interface<selene::feature_extractor>(model_path, model_type, device, use_int8 ? true : false));
		}

		unknown_object longinus_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto nms = unbox<float>(params.get_value(u8"nms"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::longinus, make_exported_interface<retina_net>(models_directory + u8"/longinus.racy", models_directory + u8"/pfld_land71_simp.racy", nms, device));
		}

		unknown_object damocles_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto use_int8 = unbox<std::int32_t>(params.get_value(u8"use_int8"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			auto FAMSV2_racy_path = models_directory + (use_int8 ? u8"/FASMV2_int8.racy" : u8"/FASMV2.racy");
			auto land65_racy_path = models_directory + (use_int8 ? u8"/pfld11_landmark65_simp.racy" : u8"/pfld11_landmark65_simp.racy");

			return add_instance(package_names::damocles, make_exported_interface<damocles::anti_spoofing>(FAMSV2_racy_path, land65_racy_path, device, use_int8 ? true : false));
		}

		unknown_object romancia_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

            return add_instance(package_names::romancia, make_exported_interface<face_alignment>(models_directory + u8"/blur_detection_best.racy", device));
        }

		unknown_object irisviel_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto single_database_capacity = unbox<std::int32_t>(params.get_value(u8"single_database_capacity"));
			auto dimension = unbox<std::int32_t>(params.get_value(u8"dimension"));
			auto working_directory = unbox<param_string>(params.get_value(u8"working_directory"));

			return add_instance(package_names::irisviel, make_exported_interface<face_service>(irisviel::face_service_implemention::brute_force, single_database_capacity, dimension, working_directory));
		}

		unknown_object gungnir_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::gungnir, make_exported_interface<yolo_net>(models_directory + u8"/gungnir.racy", device));
		}

		unknown_object mjollner_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::mjollner, make_exported_interface<ocr_net>(models_directory + u8"/det_db_resnet18.racy", models_directory + u8"/rec_crnn_resnet34.racy", models_directory + u8"/ppocr_keys_v1.txt", device));
		}

		unknown_object valklyrs_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return add_instance(package_names::valklyrs, make_exported_interface<yolov5s_net>(models_directory + u8"/yolov5s.racy", models_directory + u8"/vehicle_attri.racy", models_directory + u8"/person_attri.racy", device));
		}

		unknown_object heimdall_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto factory_type = unbox<std::int32_t>(params.get_value(u8"factory_type"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();
			return add_instance(package_names::heimdall, make_exported_interface<material_code>(models_directory, factory_type, device, params_map_abi));
		}

		unknown_object banshee_new(const param_hash_map<param_string, unknown_object>& params)
		{

			return add_instance(package_names::banshee, make_exported_interface<kcf_tracker>());
		}

		unknown_object plate_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<plate::ocr_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto x = unbox<std::int32_t>(params.get_value(u8"x"));
			auto y = unbox<std::int32_t>(params.get_value(u8"y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(image, channels, height, width, order, x, y, roi_width, roi_height, params_map_abi);

		}

		unknown_object plate_recognize(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<plate::ocr_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.recognize(image, channels, height, width, order);
		}

		unknown_object plate_trace_init(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<plate::ocr_code >(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			auto roi_x = unbox<std::int32_t>(params.get_value(u8"roi_x"));
			auto roi_y = unbox<std::int32_t>(params.get_value(u8"roi_y"));
			auto roi_w = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_h = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			instance.trace_init(image, channels, height, width, order, roi_x, roi_y, roi_w, roi_h);

			return unknown_object();
		}

		unknown_object plate_trace_update(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<plate::ocr_code >(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.trace_update(image, channels, height, width, order);

		}

		unknown_object ring_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<ring::material_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto border_orient = unbox<std::int32_t>(params.get_value(u8"border_orient"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto x = unbox<std::int32_t>(params.get_value(u8"x"));
			auto y = unbox<std::int32_t>(params.get_value(u8"y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();
			return instance.detect(image, channels, height, width, border_orient, order, x, y, roi_width, roi_height, params_map_abi);
		}

		unknown_object cassius_extract_feature(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<cassius::feature_extractor>(params);
			auto aligned_faces = unbox<param_span<std::uint8_t>>(params.get_value(u8"aligned_faces"));
			auto num = unbox<std::int32_t>(params.get_value(u8"num"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.get(aligned_faces, num, order);
		}

		unknown_object gaius_extract_feature(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<gaius::feature_extractor>(params);
			auto aligned_faces = unbox<param_span<std::uint8_t>>(params.get_value(u8"aligned_faces"));
			auto num = unbox<std::int32_t>(params.get_value(u8"num"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto has_mask = unbox<std::int32_t>(params.get_value(u8"has_mask"));

			return instance.get(aligned_faces, num, order, has_mask ? true : false);
		}

		unknown_object selene_extract_feature(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<selene::feature_extractor>(params);
			auto aligned_faces = unbox<param_span<std::uint8_t>>(params.get_value(u8"aligned_faces"));
			auto num = unbox<std::int32_t>(params.get_value(u8"num"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.get(aligned_faces, num, order);
		}

		unknown_object selene_get_model_type(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<selene::feature_extractor>(params);

			return box(instance.get_model_type());
		}

		unknown_object longinus_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<retina_net>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto min_size = unbox<std::int32_t>(params.get_value(u8"min_size"));
			auto threshold = unbox<float>(params.get_value(u8"threshold"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto do_attributing = unbox<bool>(params.get_value(u8"do_attributing"));

			return instance.detect(image, channels, height, width, min_size, threshold, order, do_attributing);
		}

		unknown_object damocles_spoofing_detect(const param_hash_map<param_string, unknown_object>& params)
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

		unknown_object damocles_presentation_attack_detect(const param_hash_map<param_string, unknown_object>& params)
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

		unknown_object longinus_trace(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<retina_net>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto face = params.get_value(u8"face").as<face_info>();
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto result = instance.single_trace(face, image, channels, height, width, order);

			return result;
		}

		unknown_object longinus_center_scale_align_face(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<retina_net>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto scale = unbox<float>(params.get_value(u8"scale"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.center_scale_align(image, channels, height, width, scale, order);
		}

		unknown_object romancia_align_face_128(const param_hash_map<param_string, unknown_object>& params)
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

		unknown_object romancia_align_face(const param_hash_map<param_string, unknown_object>& params)
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

		unknown_object romancia_blur_detect(const param_hash_map<param_string, unknown_object>& params)
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

		unknown_object romancia_mask_detect(const param_hash_map<param_string, unknown_object>& params)
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

        unknown_object romancia_rotate(const param_hash_map<param_string, unknown_object>& params)
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

        unknown_object gungnir_detect(const param_hash_map<param_string, unknown_object> &params)
        {
            constexpr std::int32_t channels = 3;
            auto instance = get_instance<yolo_net>(params);
            auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
            auto height = unbox<std::int32_t>(params.get_value(u8"height"));
            auto width = unbox<std::int32_t>(params.get_value(u8"width"));
            auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.detect(image, channels, height, width, order);
		}

		unknown_object mjollner_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<ocr_net>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto x = unbox<std::int32_t>(params.get_value(u8"x"));
			auto y = unbox<std::int32_t>(params.get_value(u8"y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			return instance.detect(image, channels, height, width, order, x, y, roi_width, roi_height);
		}

		unknown_object valklyrs_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<yolov5s_net>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.detect(image, channels, height, width, order);
		}

		unknown_object heimdall_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = get_instance<material_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto top_five = unbox<std::int32_t>(params.get_value(u8"top_five"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto x = unbox<std::int32_t>(params.get_value(u8"x"));
			auto y = unbox<std::int32_t>(params.get_value(u8"y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			return instance.detect(image, channels, height, width, top_five, order, x, y, roi_width, roi_height);
		}

		unknown_object banshee_init(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<kcf_tracker>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto x = unbox<std::int32_t>(params.get_value(u8"x"));
			auto y = unbox<std::int32_t>(params.get_value(u8"y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			instance.init_trace(image, width, height, x, y, roi_width, roi_height);
			return unknown_object();
		}

		unknown_object banshee_update(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<kcf_tracker>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));

			return instance.update(image, width, height);
		}

		void irisviel_clear(const param_hash_map<param_string, unknown_object>& params)
		{
			get_instance<face_service>(params).clear();
		}

		void irisviel_remove_all(const param_hash_map<param_string, unknown_object>& params)
		{
			get_instance<face_service>(params).remove_all();
		}

		void irisviel_load_databases(const param_hash_map<param_string, unknown_object>& params)
		{
			get_instance<face_service>(params).load_databases();
		}

		unknown_object irisviel_record_count(const param_hash_map<param_string, unknown_object>& params)
		{
			return box(get_instance<face_service>(params).record_count());
		}

		unknown_object irisviel_contains_key(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<face_service>(params);
			auto key = unbox<param_string>(params.get_value(u8"key"));

			return box(get_instance<face_service>(params).contains_key(key));
		}

		unknown_object irisviel_try_get_record(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = get_instance<face_service>(params);
			auto key = unbox<param_string>(params.get_value(u8"key"));

			return get_instance<face_service>(params).try_get_record(key);
		}

		void irisviel_add_record(const param_hash_map<param_string, unknown_object>& params)
		{
			irisviel_add_or_update_record_helper(params, false);
		}

		void irisviel_add_records(const param_hash_map<param_string, unknown_object>& params)
		{
			irisviel_add_or_update_records_helper(params, false);
		}

		void irisviel_update_record(const param_hash_map<param_string, unknown_object>& params)
		{
			irisviel_add_or_update_record_helper(params, true);
		}

		void irisviel_update_records(const param_hash_map<param_string, unknown_object>& params)
		{
			irisviel_add_or_update_records_helper(params, true);
		}

		void irisviel_remove_record(const param_hash_map<param_string, unknown_object>& params)
		{
			auto key = unbox<param_string>(params.get_value(u8"key"));

			get_instance<face_service>(params).remove_record(key);
		}

		void irisviel_remove_records(const param_hash_map<param_string, unknown_object>& params)
		{
			auto keys = params.get_value(u8"keys").as<param_vector<param_string>>();

			get_instance<face_service>(params).remove_records(keys);
		}

		unknown_object irisviel_search(const param_hash_map<param_string, unknown_object>& params)
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

		record irisviel_create_record_helper(const param_hash_map<param_string, unknown_object>& params)
		{
			auto dimension = unbox<std::int32_t>(params.get_value(u8"dimension"));
			auto key = unbox<param_string>(params.get_value(u8"key"));
			auto feature = params.get_value(u8"feature").as<param_vector<float>>();
			auto result = make_exported_interface<record>(dimension);

			result.key(key);
			result.feature(feature);

			return result;
		}

		void irisviel_add_or_update_record_helper(const param_hash_map<param_string, unknown_object>& params, bool update)
		{
			auto instance = get_instance<face_service>(params);
			auto record = irisviel_create_record_helper(params);

			if (update)
			{
				instance.update_record(record);
			}
			else
			{
				instance.add_record(record);
			}
		}

		void irisviel_add_or_update_records_helper(const param_hash_map<param_string, unknown_object>& params, bool update)
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
				instance.update_records(records);
			}
			else
			{
				instance.add_records(records);
			}
		}

		unknown_object add_instance(utf8_string_view package_name, const unknown_object& instance)
		{
			auto id = create_guid_from_bytes(meta::to_array(reinterpret_cast<std::size_t>(get_abi(instance))));
			{
				std::scoped_lock lock{ mutex_ };

				return (instances_.insert_or_assign(id, std::tuple{ package_name, instance }), box(id));
			}
		}

		void delete_instance_by_id(const guid& id)
		{
			std::scoped_lock lock{ mutex_ };

			if (auto iter = instances_.find(id); iter != instances_.end())
			{
				instances_.erase(iter);
			}
		}

		void delete_instance(const param_hash_map<param_string, unknown_object>& params)
		{
			delete_instance_by_id(get_instance_id(params));
		}

		static guid get_instance_id(const param_hash_map<param_string, unknown_object>& params)
		{
			return unbox<guid>(params.get_value(u8"object_id"));
		}

		template <typename T>
		T get_instance(const guid& id)
		{
			std::scoped_lock lock{ mutex_ };
			auto iter = instances_.find(id);

			return iter != instances_.end() ? std::get<unknown_object>(iter->second).as<T>() : throw abi_key_not_found{ exposing::format(u8"Cannot find instance: {}.", to_param_string(id)) };
		}

		template <typename T>
		T get_instance(const param_hash_map<param_string, unknown_object>& params)
		{
			return get_instance<T>(unbox<guid>(params.get_value(u8"object_id")));
		}

		std::mutex mutex_;
		std::unordered_map<guid, std::tuple<param_string, unknown_object>> instances_;
		std::unordered_map<param_string, std::function<unknown_object(const param_hash_map<param_string, unknown_object>&)>> functions_;
	};

	vision_service_impl::vision_service_impl() : impl_{ std::make_unique<impl>() }
	{
	}

	vision_service_impl::~vision_service_impl()
	{
	}

	param_string vision_service_impl::name() const
	{
		return impl_->name();
	}

	param_string vision_service_impl::version() const
	{
		return impl_->version();
	}

	param_vector<param_string> vision_service_impl::available_functions() const
	{
		return impl_->available_functions();
	}

	param_hash_map<guid, param_string> vision_service_impl::existing_instances() const
	{
		return impl_->existing_instances();
	}

	unknown_object vision_service_impl::execute(const param_string& function_name, const param_hash_map<param_string, unknown_object>& params) const
	{
		return impl_->execute(function_name, params);
	}
}
