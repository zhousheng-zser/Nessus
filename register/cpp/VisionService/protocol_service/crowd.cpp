#pragma once
#include "../service_register.hpp"
#include <functional>
#include <unordered_map>
#include <iostream>
//
#include <crowd/detect_code.hpp>

namespace glasssix::exposing::nessus::Service
{
	class S_Crowd :public Service { //Plugin
		static unknown_object crowd_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<crowd::detect_code>(models_directory, device));
		}

		static 	unknown_object crowd_version(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = vision_service_impl::impl::get_instance<crowd::detect_code>(params);
			return box(instance.version());
		}
		static unknown_object crowd_remove_library(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = vision_service_impl::impl::get_instance<crowd::detect_code>(params);

			auto id = unbox<std::int32_t>(params.get_value(u8"id"));
			return box(instance.remove_library(id));
		}

		static unknown_object crowd_detect(const param_hash_map<param_string, unknown_object>& params)
		{

			constexpr std::int32_t channels = 3;
			auto instance = vision_service_impl::impl::get_instance<crowd::detect_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));

			auto roi_x = unbox<std::int32_t>(params.get_value(u8"roi_x"));
			auto roi_y = unbox<std::int32_t>(params.get_value(u8"roi_y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));
			auto min_cluster_size = unbox<std::int32_t>(params.get_value(u8"min_cluster_size"));

			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(image, channels, height, width, roi_x, roi_y, roi_width, roi_height, min_cluster_size, params_map_abi);
		}

		static constexpr utf8_string_view MODULE_{ u8"crowd" };
		static constexpr utf8_string_view NEW_{ u8"crowd.new" };
		static constexpr utf8_string_view DELETE_{ u8"crowd.delete" };
		static constexpr utf8_string_view VERSION_{ u8"crowd.version" };
		static constexpr utf8_string_view remove_library{ u8"crowd.remove_library" };
		static constexpr utf8_string_view detect_{ u8"crowd.detect" };
	public:
		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const override {
			service_map.try_emplace(NEW_, &crowd_new);
			service_map.try_emplace(DELETE_, DELETE_FUNC);
			service_map.try_emplace(VERSION_, &crowd_version);
			service_map.try_emplace(remove_library, &crowd_remove_library);
			service_map.try_emplace(detect_, &crowd_detect);
		}
	};

	REGISTE_SERVICE(S_Crowd) //Plugin
}
