#pragma once
#include "../service_register.hpp"
#include <functional>
#include <unordered_map>
#include <iostream>
//
#include <facelandmarks/land_code.hpp>

namespace glasssix::exposing::nessus::Service
{
	class S_Facelandmarks :public Service { //Plugin
		static unknown_object facelandmarks_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<facelandmarks::land_code>(models_directory, device));
		}

		static 	unknown_object facelandmarks_version(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = vision_service_impl::impl::get_instance<facelandmarks::land_code>(params);
			return box(instance.version());
		}

		static unknown_object facelandmarks_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = vision_service_impl::impl::get_instance<facelandmarks::land_code>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			return instance.detect(image, channels, height, width);
		}

		static constexpr utf8_string_view MODULE_{ u8"facelandmarks" };
		static constexpr utf8_string_view NEW_{ u8"facelandmarks.new" };
		static constexpr utf8_string_view DELETE_{ u8"facelandmarks.delete" };
		static constexpr utf8_string_view VERSION_{ u8"facelandmarks.version" };
		static constexpr utf8_string_view detect_{ u8"facelandmarks.detect" };
	public:
		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const override {
			service_map.try_emplace(NEW_, &facelandmarks_new);
			service_map.try_emplace(DELETE_, DELETE_FUNC);
			service_map.try_emplace(VERSION_, &facelandmarks_version);

			service_map.try_emplace(detect_, &facelandmarks_detect);
		}
	};

	REGISTE_SERVICE(S_Facelandmarks) //Plugin
}