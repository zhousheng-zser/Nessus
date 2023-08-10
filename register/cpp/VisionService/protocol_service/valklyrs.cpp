#pragma once
#include "../service_register.hpp"
#include <functional>
#include <unordered_map>
#include <iostream>
//
#include <valklyrs/yolov5s_net.hpp>

namespace glasssix::exposing::nessus::Service
{
	class S_valklyrs :public Service { //Plugin
		static unknown_object valklyrs_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<valklyrs::yolov5s_net>(models_directory + u8"/yolov5s.racy", models_directory + u8"/vehicle_attri.racy", models_directory + u8"/person_attri.racy", device));
		}

		static unknown_object valklyrs_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = vision_service_impl::impl::get_instance<valklyrs::yolov5s_net>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			return instance.detect(image, channels, height, width, order);
		}
		static constexpr utf8_string_view MODULE_{ u8"valklyrs" };
		static constexpr utf8_string_view NEW_{ u8"valklyrs.new" };
		static constexpr utf8_string_view DELETE_{ u8"valklyrs.delete" };
		static constexpr utf8_string_view VERSION_{ u8"valklyrs.version" };
		static constexpr utf8_string_view detect_{ u8"valklyrs.detect" };
	public:
		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const override {
			service_map.try_emplace(NEW_, &valklyrs_new);
			service_map.try_emplace(DELETE_, DELETE_FUNC);

			service_map.try_emplace(detect_, &valklyrs_detect);
		}
	};


	REGISTE_SERVICE(S_valklyrs) //Plugin
}

