#pragma once
#include "../service_register.hpp"
#include <functional>
#include <unordered_map>
#include <iostream>
//
#include <posture/detect_code.hpp>

namespace glasssix::exposing::nessus::Service
{
	class S_Posture :public Service { //Plugin
		static unknown_object posture_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto model_type = unbox<std::int32_t>(params.get_value(u8"model_type"));

			return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<posture::detect_code>(models_directory, device,model_type));
		}
		
		static unknown_object posture_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = vision_service_impl::impl::get_instance<posture::detect_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto channels = unbox<std::int32_t>(params.get_value(u8"channels"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto roi_x = unbox<std::int32_t>(params.get_value(u8"roi_x"));
			auto roi_y = unbox<std::int32_t>(params.get_value(u8"roi_y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));
			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(image, channels, height, width, roi_x, roi_y, roi_width, roi_height, params_map_abi);
		}

		static unknown_object posture_version(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = vision_service_impl::impl::get_instance<posture::detect_code>(params);
			return box(instance.version());
		}
		static constexpr utf8_string_view MODULE_{ u8"posture" };
		static constexpr utf8_string_view NEW_{ u8"posture.new" };
		static constexpr utf8_string_view DELETE_{ u8"posture.delete" };
		static constexpr utf8_string_view VERSION_{ u8"posture.version" };
		static constexpr utf8_string_view detect_{ u8"posture.detect" };
	public:
		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const override {
			service_map.try_emplace(NEW_, &posture_new);
			service_map.try_emplace(DELETE_, DELETE_FUNC);
			service_map.try_emplace(VERSION_, &posture_version);

			service_map.try_emplace(detect_, &posture_detect);
		}
	};

	REGISTE_SERVICE(S_Posture) //Plugin
}

