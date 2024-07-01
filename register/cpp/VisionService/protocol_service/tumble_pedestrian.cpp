#pragma once
#include "../service_register.hpp"
#include <functional>
#include <unordered_map>
#include <iostream>
//
#include <tumble_pedestrian/detect_code.hpp>

namespace glasssix::exposing::nessus::Service
{
	class S_Tumble_Pedestrian :public Service { //Plugin
		static unknown_object tumble_pedestrian_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));

			return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<tumble_pedestrian::detect_code>(models_directory, device));
		}

		static 	unknown_object tumble_pedestrian_version(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = vision_service_impl::impl::get_instance<tumble_pedestrian::detect_code>(params);
			return box(instance.version());
		}

		static 	unknown_object tumble_pedestrian_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = vision_service_impl::impl::get_instance<tumble_pedestrian::detect_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto channels = unbox<std::int32_t>(params.get_value(u8"channels"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));

			auto roi_x = unbox<std::int32_t>(params.get_value(u8"roi_x"));
			auto roi_y = unbox<std::int32_t>(params.get_value(u8"roi_y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));
			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();
			auto pedestrain_info_abi = params.get_value(u8"person_list").as<exposing::param_vector<pedestrian::box_info>>();
			return instance.detect(image, channels, height, width, roi_x, roi_y, roi_width, roi_height, params_map_abi, pedestrain_info_abi);
		}
		static constexpr utf8_string_view MODULE_{ u8"tumble_pedestrian" };
		static constexpr utf8_string_view NEW_{ u8"tumble_pedestrian.new" };
		static constexpr utf8_string_view DELETE_{ u8"tumble_pedestrian.delete" };
		static constexpr utf8_string_view VERSION_{ u8"tumble_pedestrian.version" };
		static constexpr utf8_string_view detect_{ u8"tumble_pedestrian.detect" };
	public:
		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const override {
			service_map.try_emplace(NEW_, &tumble_pedestrian_new);
			service_map.try_emplace(DELETE_, DELETE_FUNC);
			service_map.try_emplace(VERSION_, &tumble_pedestrian_version);
			service_map.try_emplace(detect_, &tumble_pedestrian_detect);
		}
	};

	REGISTE_SERVICE(S_Tumble_Pedestrian) //Plugin
}