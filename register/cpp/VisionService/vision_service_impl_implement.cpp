#include "vision_service_impl.hpp"
#include "vision_service_impl_implement.hpp"
#include "service_register.hpp"
#include <mutex>
#include <functional>
#include <unordered_map>

#include <iostream>

namespace glasssix::exposing::nessus
{
	// impl
	// =========================================================================================
	vision_service_impl::impl::impl()
	{
		// ADD
		
		Service::AddService(functions_);

		// FACE SDK
		// New
		functions_.insert_or_assign(function_names::gaius_new, std::bind(&impl::gaius_new, std::placeholders::_1));
		functions_.insert_or_assign(function_names::cassius_new, std::bind(&impl::cassius_new, std::placeholders::_1));
		functions_.insert_or_assign(function_names::longinus_new, std::bind(&impl::longinus_new, std::placeholders::_1));
		functions_.insert_or_assign(function_names::damocles_new, std::bind(&impl::damocles_new, std::placeholders::_1));
		functions_.insert_or_assign(function_names::romancia_new, std::bind(&impl::romancia_new, std::placeholders::_1));
		functions_.insert_or_assign(function_names::irisviel_new, std::bind(&impl::irisviel_new, std::placeholders::_1));
		functions_.insert_or_assign(function_names::selene_new, std::bind(&impl::selene_new, std::placeholders::_1));

		// Delete
		functions_.insert_or_assign(function_names::gaius_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, std::placeholders::_1)));
		functions_.insert_or_assign(function_names::cassius_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, std::placeholders::_1)));
		functions_.insert_or_assign(function_names::irisviel_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, std::placeholders::_1)));
		functions_.insert_or_assign(function_names::longinus_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, std::placeholders::_1)));
		functions_.insert_or_assign(function_names::damocles_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, std::placeholders::_1)));
		functions_.insert_or_assign(function_names::romancia_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, std::placeholders::_1)));
		functions_.insert_or_assign(function_names::selene_delete, meta::replace_return<unknown_object>(std::bind(&impl::delete_instance, std::placeholders::_1)));

		// Business
		functions_.insert_or_assign(function_names::damocles_spoofing_detect, std::bind(&impl::damocles_spoofing_detect, std::placeholders::_1));
		functions_.insert_or_assign(function_names::damocles_presentation_attack_detect, std::bind(&impl::damocles_presentation_attack_detect, std::placeholders::_1));
		functions_.insert_or_assign(function_names::longinus_detect, std::bind(&impl::longinus_detect, std::placeholders::_1));
		functions_.insert_or_assign(function_names::longinus_trace, std::bind(&impl::longinus_trace, std::placeholders::_1));
		functions_.insert_or_assign(function_names::longinus_center_scale_align_face, std::bind(&impl::longinus_center_scale_align_face, std::placeholders::_1));
		functions_.insert_or_assign(function_names::romancia_align_face_128, std::bind(&impl::romancia_align_face_128, std::placeholders::_1));
		functions_.insert_or_assign(function_names::romancia_align_face, std::bind(&impl::romancia_align_face, std::placeholders::_1));
		functions_.insert_or_assign(function_names::romancia_blur_detect, std::bind(&impl::romancia_blur_detect, std::placeholders::_1));
		functions_.insert_or_assign(function_names::romancia_mask_detect, std::bind(&impl::romancia_mask_detect, std::placeholders::_1));
		functions_.insert_or_assign(function_names::romancia_rotate, std::bind(&impl::romancia_rotate, std::placeholders::_1));
		functions_.insert_or_assign(function_names::gaius_forward, std::bind(&impl::gaius_extract_feature, std::placeholders::_1));
		functions_.insert_or_assign(function_names::cassius_forward, std::bind(&impl::cassius_extract_feature, std::placeholders::_1));
		functions_.insert_or_assign(function_names::selene_forward, std::bind(&impl::selene_extract_feature, std::placeholders::_1));
		functions_.insert_or_assign(function_names::selene_get_model_type, std::bind(&impl::selene_get_model_type, std::placeholders::_1));
		functions_.insert_or_assign(function_names::irisviel_clear, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_clear, std::placeholders::_1)));
		functions_.insert_or_assign(function_names::irisviel_remove_all, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_remove_all, std::placeholders::_1)));
		functions_.insert_or_assign(function_names::irisviel_load_databases, meta::replace_return<unknown_object>(std::bind(&impl::irisviel_load_databases, std::placeholders::_1)));
		functions_.insert_or_assign(function_names::irisviel_record_count, std::bind(&impl::irisviel_record_count, std::placeholders::_1));
		functions_.insert_or_assign(function_names::irisviel_contains_key, std::bind(&impl::irisviel_contains_key, std::placeholders::_1));
		functions_.insert_or_assign(function_names::irisviel_try_get_record, std::bind(&impl::irisviel_try_get_record, std::placeholders::_1));
		functions_.insert_or_assign(function_names::irisviel_add_records, std::bind(&impl::irisviel_add_records, std::placeholders::_1));
		functions_.insert_or_assign(function_names::irisviel_update_records, std::bind(&impl::irisviel_update_records, std::placeholders::_1));
		functions_.insert_or_assign(function_names::irisviel_remove_records, std::bind(&impl::irisviel_remove_records, std::placeholders::_1));
		functions_.insert_or_assign(function_names::irisviel_search, std::bind(&impl::irisviel_search, std::placeholders::_1));
		functions_.insert_or_assign(function_names::irisviel_search_nf, std::bind(&impl::irisviel_search_nf, std::placeholders::_1));
	}


	param_string vision_service_impl::impl::name() const
	{
		return u8"Glasssix Vision Service";
	}

	param_string vision_service_impl::impl::version() const
	{
		return u8"1.0.0";
	}

	param_vector<param_string> vision_service_impl::impl::available_functions() const
	{
		auto result = make_param_vector<param_string>();

		for (const auto& [key, value] : functions_)
		{
			result.push_back(key);
		}

		return result;
	}

	param_hash_map<guid, param_string> vision_service_impl::impl::existing_instances() const
	{
		auto result = make_param_hash_map<guid, param_string>();

		for (const auto& [key, value] : instances_)
		{
			result.add_or_update(key, std::get<param_string>(value));
		}

		return result;
	}

	unknown_object vision_service_impl::impl::execute(const param_string& function_name, const param_hash_map<param_string, unknown_object>& params) const
	{
		auto iter = functions_.find(function_name);
		return iter != functions_.end() ? iter->second(params) : throw abi_key_not_found{ function_name };
	}

	// vision_service
	// =========================================================================================

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
