#ifndef _VALVE_CODE_OCR_MAIN_HPP_
#define _VALVE_CODE_OCR_MAIN_HPP_

#include "box_info.hpp"
#include <abi/consumer.hpp>

namespace glasssix::valve
{
    struct detect_code;
    struct Object;
}

namespace glasssix::exposing::impl
{
    template <>
    struct abi<valve::detect_code>
    {
        using identity_type = type_identity_interface;

        static constexpr guid id{ "{67B34138-9CD9-4C7C-94B7-EBA798627113}" };

        struct type : abi_unknown_object
        {
            virtual std::int32_t G6_ABI_CALL init(
                abi_in_t<param_string> model_directory,
                abi_in_t<std::int32_t> factory_type,
                std::int32_t device,
                abi_in_t<exposing::param_hash_map<exposing::param_string, float>> init_param_map_abi) noexcept = 0;

            virtual std::int32_t G6_ABI_CALL detect(
                abi_in_t<param_span<std::uint8_t>> bitmap,
                std::int32_t channels,
                std::int32_t height,
                std::int32_t width,
                std::int32_t order,
                abi_in_t<param_vector<int>> rois_abi,
                abi_in_t<exposing::param_hash_map<exposing::param_string, float>> det_param_map_abi,
                abi_out_t<param_vector<valve::box_info>> result) noexcept = 0;

            virtual std::int32_t G6_ABI_CALL version(abi_out_t<param_string> result) noexcept = 0;
        };
    };

    template <typename Derived>
    struct interface_vtable<Derived, valve::detect_code> : interface_vtable_base<Derived, valve::detect_code>
    {
        virtual std::int32_t G6_ABI_CALL init(
            abi_in_t<param_string> model_directory,
            abi_in_t<std::int32_t> factory_type,
            std::int32_t device,
            abi_in_t<exposing::param_hash_map<exposing::param_string, float>> init_param_map_abi) noexcept override
        {
            return abi_safe_call([&]
                {this->self().init(
                    create_from_abi<param_string>(model_directory),
                    create_from_abi<std::int32_t>(factory_type),
                    device,
                    create_from_abi<exposing::param_hash_map<exposing::param_string, float>>(init_param_map_abi));
                });
        }

        virtual std::int32_t G6_ABI_CALL detect(
            abi_in_t<param_span<std::uint8_t>> bitmap,
            std::int32_t channels,
            std::int32_t height,
            std::int32_t width,
            std::int32_t order,
            abi_in_t<param_vector<int>> rois_abi,
            abi_in_t<exposing::param_hash_map<exposing::param_string, float>> det_param_map_abi,
            abi_out_t<param_vector<valve::box_info>> result) noexcept override
        {
            return abi_safe_call([&]
                {
                    *result = detach_abi(this->self().detect(
                        create_from_abi<param_span<std::uint8_t>>(bitmap),
                        channels,
                        height,
                        width,
                        order,
                        create_from_abi<param_vector<int>>(rois_abi),
                        create_from_abi<exposing::param_hash_map<exposing::param_string, float>>(det_param_map_abi)
                        )
                    );
                }
            );
        }

        virtual std::int32_t G6_ABI_CALL version(abi_out_t<param_string> result) noexcept override
        {
            return abi_safe_call([&]
                { *result = detach_abi(this->self().version()); });
        }
    };

    template <>
    struct abi_adapter<valve::detect_code>
    {
        template <typename Derived>
        struct type : enable_self_abi_awareness<Derived, valve::detect_code>
        {
            void init(
                const param_string& model_directory,
                std::int32_t factory_type,
                std::int32_t device,
                const exposing::param_hash_map<exposing::param_string, float>& init_param_map_abi) const
            {
                check_abi_result(this->self_abi().init(
                    get_abi(model_directory),
                    get_abi(factory_type),
                    get_abi(device),
                    get_abi(init_param_map_abi)));
            }

            param_vector<valve::box_info> detect(
                param_span<std::uint8_t> bitmap,
                std::int32_t channels,
                std::int32_t height,
                std::int32_t width,
                std::int32_t order,
                param_vector<int> rois_abi,
                const exposing::param_hash_map<exposing::param_string, float>& det_param_map_abi) const
            {
                param_vector<valve::box_info> result{ nullptr };
                return (check_abi_result(this->self_abi().detect(
                    get_abi(bitmap),
                    channels,
                    height,
                    width,
                    order,
                    get_abi(rois_abi),
                    get_abi(det_param_map_abi),
                    put_abi(result))), result);
            }

            param_string version() const
            {
                param_string result{ nullptr };

                return (check_abi_result(this->self_abi().version(put_abi(result))), result);
            }
        };
    };
}

namespace glasssix::valve
{
    struct detect_code : exposing::inherits<detect_code>
    {
        using inherits::inherits;
    };
}

#endif