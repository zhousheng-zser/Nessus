#pragma once
#include "../service_register.hpp"
#include <functional>
#include <unordered_map>
#include <iostream>
//
#include <trespass/detect_code.hpp>

namespace glasssix::exposing::nessus::Service
{
	class S_Trespass :public Service { //Plugin
		static unknown_object trespass_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));

			return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<trespass::detect_code>(models_directory, device));
		}

		static unknown_object trespass_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = vision_service_impl::impl::get_instance<trespass::detect_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto channels = unbox<std::int32_t>(params.get_value(u8"channels"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));

			return instance.detect(image, channels, height, width);
		}
		static constexpr utf8_string_view MODULE_{ u8"trespass" };
		static constexpr utf8_string_view NEW_{ u8"trespass.new" };
		static constexpr utf8_string_view DELETE_{ u8"trespass.delete" };
		static constexpr utf8_string_view VERSION_{ u8"trespass.version" };
		static constexpr utf8_string_view detect_{ u8"trespass.detect" };
	public:
		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const override {
			service_map.try_emplace(NEW_, &trespass_new);
			service_map.try_emplace(DELETE_, DELETE_FUNC);

			service_map.try_emplace(detect_, &trespass_detect);
		}
	};

	REGISTE_SERVICE(S_Trespass) //Plugin
}
