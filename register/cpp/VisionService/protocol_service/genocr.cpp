#pragma once
#include "../service_register.hpp"
#include <functional>
#include <unordered_map>
#include <iostream>
//
#include <genocr/txt_code.hpp>

namespace glasssix::exposing::nessus::Service
{
	class S_Genocr :public Service { //Plugin

		static 	unknown_object genocr_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto factory_type = unbox<std::int32_t>(params.get_value(u8"factory_type"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			auto chardic_directory = unbox<param_string>(params.get_value(u8"chardic_directory"));
			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();
			return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<genocr::txt_code>(models_directory, chardic_directory, factory_type, device, params_map_abi));
		}

		static 	unknown_object genocr_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = vision_service_impl::impl::get_instance<genocr::txt_code>(params);
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

		static constexpr utf8_string_view MODULE_{ u8"genocr" };
		static constexpr utf8_string_view NEW_{ u8"genocr.new" };
		static constexpr utf8_string_view DELETE_{ u8"genocr.delete" };
		//static constexpr utf8_string_view VERSION_{ u8"genocr.version" };
		static constexpr utf8_string_view detect_{ u8"genocr.detect" };
	public:
		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const override {
			service_map.try_emplace(NEW_, &genocr_new);
			service_map.try_emplace(DELETE_, DELETE_FUNC);

			service_map.try_emplace(detect_, &genocr_detect);
		}
	};

	REGISTE_SERVICE(S_Genocr) //Plugin
}

