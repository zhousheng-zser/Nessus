#pragma once
#include "../service_register.hpp"
#include <functional>
#include <unordered_map>
#include <iostream>
//
#include <plate/ocr_code.hpp>

namespace glasssix::exposing::nessus::Service
{
	class S_Plate :public Service { //Plugin
		static unknown_object plate_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<plate::ocr_code>(models_directory, device));
		}

		static unknown_object plate_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = vision_service_impl::impl::get_instance<plate::ocr_code>(params);
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

		static unknown_object plate_recognize(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = vision_service_impl::impl::get_instance<plate::ocr_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.recognize(image, channels, height, width, order);
		}

		static unknown_object plate_trace_init(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = vision_service_impl::impl::get_instance<plate::ocr_code >(params);
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

		static unknown_object plate_trace_update(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = vision_service_impl::impl::get_instance<plate::ocr_code >(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.trace_update(image, channels, height, width, order);
		}

		static constexpr utf8_string_view MODULE_{ u8"plate" };
		static constexpr utf8_string_view NEW_{ u8"plate.new" };
		static constexpr utf8_string_view DELETE_{ u8"plate.delete" };

		static constexpr utf8_string_view detect_{ u8"plate.detect" };
		static constexpr utf8_string_view trace_init_{ u8"plate.trace_init" };
		static constexpr utf8_string_view trace_update_{ u8"plate.trace_update" };
		static constexpr utf8_string_view recognize_{ u8"plate.recognize" };

	public:
		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const override {
			service_map.try_emplace(NEW_, &plate_new);
			service_map.try_emplace(DELETE_, DELETE_FUNC);

			service_map.try_emplace(detect_, &plate_detect);
			service_map.try_emplace(trace_init_, &plate_trace_init);
			service_map.try_emplace(trace_update_, &plate_trace_update);
			service_map.try_emplace(recognize_, &plate_recognize);
		}
	};

	REGISTE_SERVICE(S_Plate) //Plugin
}