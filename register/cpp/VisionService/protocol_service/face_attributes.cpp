#pragma once
#include "../service_register.hpp"
#include <functional>
#include <unordered_map>
#include <iostream>
// #include "PluginRegister/plugin_interface.hpp"
// #include "../../../../parser/cpp/message_protocol_jsoncpp.hpp"
// #include "../../../../common/include/faceattributes/face_attributes_detector.hpp"
#include "faceattributes/face_attributes_detector.hpp"

namespace glasssix::exposing::nessus::Service
{
	class S_FaceAttributes :public Service { //Plugin

		static unknown_object face_attributes_new(const param_hash_map<param_string, unknown_object>& params)
		{
			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

			return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<face_attributes::face_attributes_detector>(models_directory, device));
		}

		static unknown_object face_attributes_version(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance =  vision_service_impl::impl::get_instance<face_attributes::face_attributes_detector>(params);
			return box(instance.version());
		}

		static unknown_object face_attributes_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			constexpr std::int32_t channels = 3;
			auto instance = vision_service_impl::impl::get_instance<face_attributes::face_attributes_detector>(params);
			auto bitmap = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto order = unbox<std::int32_t>(params.get_value(u8"order"));
			auto faces = params.get_value(u8"facerect_list").as<param_vector<face_info>>();

			return instance.detect(faces, bitmap, channels, height, width, order);
		}

		static constexpr utf8_string_view MODULE_{ u8"face_attributes" };
		static constexpr utf8_string_view NEW_{ u8"face_attributes.new" };
		static constexpr utf8_string_view DELETE_{ u8"face_attributes.delete" };
		static constexpr utf8_string_view VERSION_{ u8"face_attributes.version" };
		static constexpr utf8_string_view detect_{ u8"face_attributes.detect" };
	public:
		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const override {
			service_map.try_emplace(NEW_, &face_attributes_new);
			service_map.try_emplace(DELETE_, DELETE_FUNC);
			service_map.try_emplace(VERSION_, &face_attributes_version);

			service_map.try_emplace(detect_, &face_attributes_detect);
		}
	};

	REGISTE_SERVICE(S_FaceAttributes) //Plugin
}
  
