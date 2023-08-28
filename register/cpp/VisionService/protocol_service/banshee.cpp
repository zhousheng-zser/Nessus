#pragma once
#include "../service_register.hpp"
#include <functional>
#include <unordered_map>
#include <iostream>
#include <banshee/kcf_tracker.hpp>

namespace glasssix::exposing::nessus::Service
{
	class S_Banshee :public Service {

		static unknown_object banshee_new(const param_hash_map<param_string, unknown_object>& params)
		{
			return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<banshee::kcf_tracker>());
		}

		static unknown_object banshee_init(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = vision_service_impl::impl::get_instance<banshee::kcf_tracker>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto x = unbox<std::int32_t>(params.get_value(u8"x"));
			auto y = unbox<std::int32_t>(params.get_value(u8"y"));
			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));

			instance.init_trace(image, width, height, x, y, roi_width, roi_height);
			return unknown_object();
		}

		static unknown_object banshee_update(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = vision_service_impl::impl::get_instance<banshee::kcf_tracker>(params);
			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));

			return instance.update(image, width, height);
		}

		static constexpr utf8_string_view MODULE_{ u8"banshee" };
		static constexpr utf8_string_view NEW_{ u8"banshee.new" };
		static constexpr utf8_string_view DELETE_{ u8"banshee.delete" };
		//static constexpr utf8_string_view VERSION_{ u8"banshee.version" };
		static constexpr utf8_string_view init_{ u8"banshee.init" };
		static constexpr utf8_string_view update_{ u8"banshee.update" };
	public:
		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const override {
			service_map.try_emplace(NEW_, &banshee_new);
			service_map.try_emplace(DELETE_, DELETE_FUNC);

			service_map.try_emplace(init_, &banshee_init);
			service_map.try_emplace(update_, &banshee_update);
		}
	};

	REGISTE_SERVICE(S_Banshee)
}

