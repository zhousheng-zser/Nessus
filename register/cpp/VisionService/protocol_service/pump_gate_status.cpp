// #pragma once
// #include "../service_register.hpp"
// #include <functional>
// #include <unordered_map>
// #include <iostream>
// //
// #include <pump_gate_status/gate_status.hpp>

// namespace glasssix::exposing::nessus::Service
// {
// 	class S_pumpgatestatuss :public Service { //Plugin
// 		static unknown_object pumpgatestatuss_new(const param_hash_map<param_string, unknown_object>& params)
// 		{
// 			auto device = unbox<std::int32_t>(params.get_value(u8"device"));
// 			auto models_directory = unbox<param_string>(params.get_value(u8"models_directory"));

//             auto yellow_hsv_lower = unbox<std::int32_t>(params.get_value(u8"yellow_hsv_lower"));
//             auto yellow_hsv_upper = unbox<std::int32_t>(params.get_value(u8"yellow_hsv_upper"));
//             auto gray_hsv_lower = unbox<std::int32_t>(params.get_value(u8"gray_hsv_lower"));
//             auto gray_hsv_upper = unbox<std::int32_t>(params.get_value(u8"gray_hsv_upper"));

// 			auto hsvparam = exposing::make_param_vector<int>(); 

// 			hsvparam.push_back(yellow_hsv_lower);
// 			hsvparam.push_back(yellow_hsv_upper);
// 			hsvparam.push_back(gray_hsv_lower);
// 			hsvparam.push_back(gray_hsv_upper);

// 			return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<pump_gate_status::gate_status>(hsvparam));
// 		}

// 		static unknown_object pumpgatestatuss_detect(const param_hash_map<param_string, unknown_object>& params)
// 		{
// 			constexpr std::int32_t channels = 3;
// 			auto instance = vision_service_impl::impl::get_instance<pump_gate_status::gate_status>(params);
// 			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
// 			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
// 			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
// 			auto roi_x = unbox<std::int32_t>(params.get_value(u8"roi_x"));
// 			auto roi_y = unbox<std::int32_t>(params.get_value(u8"roi_y"));
// 			auto roi_width = unbox<std::int32_t>(params.get_value(u8"roi_width"));
// 			auto roi_height = unbox<std::int32_t>(params.get_value(u8"roi_height"));
// 			auto rois = params.get_value(u8"rois").as<exposing::param_vector<int>>();
// 			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

// 			return box(instance.detect(image, channels, height, width, roi_x, roi_y, roi_width, roi_height, rois, params_map_abi));
// 		}

// 		static unknown_object pumpgatestatuss_version(const param_hash_map<param_string, unknown_object>& params)
// 		{
// 			auto instance = vision_service_impl::impl::get_instance<pumpgatestatuss::detect_code>(params);
// 			return box(instance.version());
// 		}

// 		static constexpr utf8_string_view MODULE_{ u8"pumpgatestatuss" };
// 		static constexpr utf8_string_view NEW_{ u8"pumpgatestatuss.new" };
// 		static constexpr utf8_string_view DELETE_{ u8"pumpgatestatuss.delete" };
// 		static constexpr utf8_string_view VERSION_{ u8"pumpgatestatuss.version" };
// 		static constexpr utf8_string_view detect_{ u8"pumpgatestatuss.detect" };
// 	public:
// 		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const override {
// 			service_map.try_emplace(NEW_, &pumpgatestatuss_new);
// 			service_map.try_emplace(DELETE_, DELETE_FUNC);
// 			service_map.try_emplace(VERSION_, &pumpgatestatuss_version);

// 			service_map.try_emplace(detect_, &pumpgatestatuss_detect);
// 		}
// 	};

// 	REGISTE_SERVICE(S_pumpgatestatuss) //Plugin
// }


#pragma once
#include "../service_register.hpp"
#include <functional>
#include <unordered_map>
#include <iostream>
//
#include <pump_gate_status/gate_status.hpp>

namespace glasssix::exposing::nessus::Service
{
	class S_pump_gate_status :public Service { //Plugin
		static unknown_object pump_gate_status_new(const param_hash_map<param_string, unknown_object>& params)
		{


			std::int32_t model_type;
			param_string racy_path;
			std::int32_t device=-1;
			bool use_int8;
			return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<pump_gate_status::gate_status>(device));
			// return vision_service_impl::impl::add_instance(MODULE_, make_exported_interface<pump_gate_status::gate_status>(model_type, racy_path, device, use_int8  ));
		}

		static unknown_object pump_gate_status_detect(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = vision_service_impl::impl::get_instance<pump_gate_status::gate_status>(params);

			// auto yellow_hsv_lower = unbox<std::int32_t>(params.get_value(u8"yellow_hsv_lower"));
			// auto yellow_hsv_upper = unbox<std::int32_t>(params.get_value(u8"yellow_hsv_upper"));
			// auto gray_hsv_lower = unbox<std::int32_t>(params.get_value(u8"gray_hsv_lower"));
			// auto gray_hsv_upper = unbox<std::int32_t>(params.get_value(u8"gray_hsv_upper"));

			// auto aligned_faces = unbox<param_span<std::uint8_t>>(params.get_value(u8"aligned_faces"));
			// auto num = unbox<std::int32_t>(params.get_value(u8"num"));
			// auto order = unbox<std::int32_t>(params.get_value(u8"order"));

			// return instance.detect(aligned_faces, num, order);

			auto image = unbox<param_span<std::uint8_t>>(params.get_value(u8"image"));
			auto channels = unbox<std::int32_t>(params.get_value(u8"channels"));
			auto height = unbox<std::int32_t>(params.get_value(u8"height"));
			auto width = unbox<std::int32_t>(params.get_value(u8"width"));
			auto yellow_hsv_lower = unbox<std::int32_t>(params.get_value(u8"yellow_hsv_lower"));
			auto yellow_hsv_upper = unbox<std::int32_t>(params.get_value(u8"yellow_hsv_upper"));
			auto gray_hsv_lower = unbox<std::int32_t>(params.get_value(u8"gray_hsv_lower"));
			auto gray_hsv_upper = unbox<std::int32_t>(params.get_value(u8"gray_hsv_upper"));

			auto rois = params.get_value(u8"rois").as<exposing::param_vector<int>>();
			auto params_map_abi = params.get_value(u8"params").as<exposing::param_hash_map<exposing::param_string, float>>();

			return box(instance.detect(image, channels, height, width, yellow_hsv_lower, yellow_hsv_upper, gray_hsv_lower, gray_hsv_upper, rois, params_map_abi));
		}

		static unknown_object pump_gate_status_version(const param_hash_map<param_string, unknown_object>& params)
		{
			auto instance = vision_service_impl::impl::get_instance<pump_gate_status::gate_status>(params);
			return box(instance.version());
		}

		static constexpr utf8_string_view MODULE_{ u8"pump_gate_status" };
		static constexpr utf8_string_view NEW_{ u8"pump_gate_status.new" };
		static constexpr utf8_string_view DELETE_{ u8"pump_gate_status.delete" };
		static constexpr utf8_string_view VERSION_{ u8"pump_gate_status.version" };
		static constexpr utf8_string_view detect_{ u8"pump_gate_status.detect" };
	public:
		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const override {
			service_map.try_emplace(NEW_, &pump_gate_status_new);
			service_map.try_emplace(DELETE_, DELETE_FUNC);
			service_map.try_emplace(VERSION_, &pump_gate_status_version);

			service_map.try_emplace(detect_, &pump_gate_status_detect);
		}
	};

	REGISTE_SERVICE(S_pump_gate_status) //Plugin
}