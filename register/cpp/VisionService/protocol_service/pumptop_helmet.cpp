#pragma once
#include "../service_register.hpp"
#include <functional>
#include <unordered_map>
#include <iostream>
#include "pumptop_helmet/pumptop_helmet_detector.hpp"

namespace glasssix::exposing::nessus::Service
{
	class S_PumptopHelmet :public Service { //Plugin

		static unknown_object pumptop_helmet_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<pumptop_helmet::pumptop_helmet_detector>(models_directory, device));
		}

		static unknown_object pumptop_helmet_version(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance =  vision_service_impl::impl::get_instance<pumptop_helmet::pumptop_helmet_detector>(params);
			return box(instance.version());
		}

		static unknown_object pumptop_helmet_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = vision_service_impl::impl::get_instance<pumptop_helmet::pumptop_helmet_detector>(params);
			auto bitmap = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return instance.detect(bitmap, channels, height, width, params_map_abi);
		}

		static constexpr utf8_string_view MODULE_{ u8"pumptop_helmet" };
		static constexpr utf8_string_view NEW_{ u8"pumptop_helmet.new" };
		static constexpr utf8_string_view DELETE_{ u8"pumptop_helmet.delete" };
		static constexpr utf8_string_view VERSION_{ u8"pumptop_helmet.version" };
		static constexpr utf8_string_view detect_{ u8"pumptop_helmet.detect" };
	public:
		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const override {
			service_map.try_emplace(NEW_, &pumptop_helmet_new);
			service_map.try_emplace(DELETE_, DELETE_FUNC);
			service_map.try_emplace(VERSION_, &pumptop_helmet_version);

			service_map.try_emplace(detect_, &pumptop_helmet_detect);
		}
	};

	REGISTE_SERVICE(S_PumptopHelmet) //Plugin
}

