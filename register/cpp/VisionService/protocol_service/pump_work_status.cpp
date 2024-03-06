#pragma once
#include "../service_register.hpp"
#include <functional>
#include <unordered_map>
#include <iostream>

//
#include <pump_work_status/work_status.hpp>

namespace glasssix::exposing::nessus::Service
{
	class S_pump_work_status :public Service { //Plugin
		
		static unknown_object pump_work_status_new(const param_hash_map<param_string, unknown_object>& params)
		{
			std::int32_t model_type;
			param_string racy_path;
			std::int32_t device=-1;
			bool use_int8;
			return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<pump_work_status::work_status>(device));
			// return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<pump_work_status::work_status>(model_type, racy_path, device, use_int8  ));
		}

		static unknown_object pump_work_status_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = vision_service_impl::impl::get_instance<pump_work_status::work_status>(params);

			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto channels = unbox<std::int32_t>(params.get_value(u8"channels"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));

			auto rois = params.get_value(u8"rois").as<exposing::param_vector<int>>();
			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return box(instance.status(image, channels, height, width, rois, params_map_abi));
		}

		static unknown_object pump_work_status_version(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = vision_service_impl::impl::get_instance<pump_work_status::work_status>(params);
			return box(instance.version());
		}

		static constexpr utf8_string_view MODULE_{ u8"pump_work_status" };
		static constexpr utf8_string_view NEW_{ u8"pump_work_status.new" };
		static constexpr utf8_string_view DELETE_{ u8"pump_work_status.delete" };
		static constexpr utf8_string_view VERSION_{ u8"pump_work_status.version" };
		static constexpr utf8_string_view status_{ u8"pump_work_status.status" };
	public:
		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const override {
			service_map.try_emplace(NEW_, &pump_work_status_new);
			service_map.try_emplace(DELETE_, DELETE_FUNC);
			service_map.try_emplace(VERSION_, &pump_work_status_version);

			service_map.try_emplace(status_, &pump_work_status_detect);
		}
	};

	REGISTE_SERVICE(S_pump_work_status) //Plugin
}