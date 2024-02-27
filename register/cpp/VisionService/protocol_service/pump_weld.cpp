#pragma once
#include "../service_register.hpp"
#include <functional>
#include <unordered_map>
#include <iostream>
//
#include <pump_weld/detect_code.hpp>
#include <pump_weld/box_info.hpp>

namespace glasssix::exposing::nessus::Service
{
	class S_Pump_weld :public Service { //Plugin
		static unknown_object pump_weld_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<glasssix::pump_weld::detect_code>(models_directory, device));
		}

		static unknown_object pump_weld_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = vision_service_impl::impl::get_instance<pump_weld::detect_code>(params);

			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto batch = unbox<std::int32_t>(params.get_value(u8"batch"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(image, batch, height, width, params_map_abi);
		}

		static unknown_object pump_weld_version(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = vision_service_impl::impl::get_instance<pump_weld::detect_code>(params);
			return box(instance.version());
		}
		static constexpr utf8_string_view MODULE_{ u8"pump_weld" };
		static constexpr utf8_string_view NEW_{ u8"pump_weld.new" };
		static constexpr utf8_string_view DELETE_{ u8"pump_weld.delete" };
		static constexpr utf8_string_view VERSION_{ u8"pump_weld.version" };
		static constexpr utf8_string_view detect_{ u8"pump_weld.detect" };
	public:
		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const override {
			service_map.try_emplace(NEW_, &pump_weld_new);
			service_map.try_emplace(DELETE_, DELETE_FUNC);
			service_map.try_emplace(VERSION_, &pump_weld_version);

			service_map.try_emplace(detect_, &pump_weld_detect);
		}
	};

	REGISTE_SERVICE(S_Pump_weld) //Plugin
}