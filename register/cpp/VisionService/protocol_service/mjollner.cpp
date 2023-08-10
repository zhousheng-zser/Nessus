#pragma once
#include "../service_register.hpp"
#include <functional>
#include <unordered_map>
#include <iostream>
//
#include <mjollner/ocr_net.hpp>

namespace glasssix::exposing::nessus::Service
{
	class S_Mjollner :public Service { //Plugin
		static unknown_object mjollner_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<mjollner::ocr_net>(models_directory + u8"/det_db_resnet18.racy", models_directory + u8"/rec_crnn_resnet34.racy", models_directory + u8"/ppocr_keys_v1.txt", device));
		}
		
		static unknown_object mjollner_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = vision_service_impl::impl::get_instance<mjollner::ocr_net>(params);
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

		static constexpr utf8_string_view MODULE_{ u8"mjollner" };
		static constexpr utf8_string_view NEW_{ u8"mjollner.new" };
		static constexpr utf8_string_view DELETE_{ u8"mjollner.delete" };
		static constexpr utf8_string_view VERSION_{ u8"mjollner.version" };
		static constexpr utf8_string_view detect_{ u8"mjollner.detect" };
	public:
		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const override {
			service_map.try_emplace(NEW_, &mjollner_new);
			service_map.try_emplace(DELETE_, DELETE_FUNC);

			service_map.try_emplace(detect_, &mjollner_detect);
		}
	};

	REGISTE_SERVICE(S_Mjollner) //Plugin
}

