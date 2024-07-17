#pragma once
#include "../service_register.hpp"
#include <functional>
#include <unordered_map>
#include <iostream>
//
#include <subway_anomaly/detect_code.hpp>

namespace glasssix::exposing::nessus::Service
{
	class S_Subway_Anomaly :public Service { //Plugin
		static unknown_object subway_anomaly_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<subway_anomaly::detect_code>(models_directory, device));
		}

		static 	unknown_object subway_anomaly_version(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = vision_service_impl::impl::get_instance<subway_anomaly::detect_code>(params);
			return box(instance.version());
		}

		static unknown_object subway_anomaly_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = vision_service_impl::impl::get_instance<subway_anomaly::detect_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));

			auto roi_x = unbox<std::int32_t>(params.get_value(u8"roi_x"));
			auto roi_y = unbox<std::int32_t>(params.get_value(u8"roi_y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));
			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();
			return instance.detect(image, channels, height, width, roi_x, roi_y, roi_width, roi_height, params_map_abi);
		}

		static constexpr utf8_string_view MODULE_{ u8"subway_anomaly" };
		static constexpr utf8_string_view NEW_{ u8"subway_anomaly.new" };
		static constexpr utf8_string_view DELETE_{ u8"subway_anomaly.delete" };
		static constexpr utf8_string_view VERSION_{ u8"subway_anomaly.version" };
		static constexpr utf8_string_view detect_{ u8"subway_anomaly.detect" };
	public:
		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const override {
			service_map.try_emplace(NEW_, &subway_anomaly_new);
			service_map.try_emplace(DELETE_, DELETE_FUNC);
			service_map.try_emplace(VERSION_, &subway_anomaly_version);

			service_map.try_emplace(detect_, &subway_anomaly_detect);
		}
	};

	REGISTE_SERVICE(S_Subway_Anomaly) //Plugin
}