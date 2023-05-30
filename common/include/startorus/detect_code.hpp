#ifndef _STARTORUS_CODE_OCR_MAIN_HPP_
#define _STARTORUS_CODE_OCR_MAIN_HPP_

#include "box_info.hpp"
#include <abi/consumer.hpp>

namespace glasssix::startorus
{
    struct detect_code;
    struct Object;
}

namespace glasssix::exposing::impl
{
    template <>
    struct abi<startorus::detect_code>
    {
        using identity_type = type_identity_interface;

        static constexpr guid id{ "{5EB2656F-0121-4398-92FD-9B6E002204BF}" };

        struct type : abi_unknown_object
        {
            virtual std::int32_t G6_ABI_CALL init(
                abi_in_t<param_string> model_directory,
                abi_in_t<std::int32_t> factory_type,
                std::int32_t device,
                abi_in_t<exposing::param_hash_map<exposing::param_string, float>> param_map_abi) noexcept = 0;

            virtual std::int32_t G6_ABI_CALL detect(
                abi_in_t<param_span<std::uint8_t>> bitmap,
                std::int32_t channels,
                std::int32_t height,
                std::int32_t width,
                std::int32_t order,
                abi_in_t<param_vector<int>> rois_abi,
                abi_out_t<param_vector<startorus::box_info>> result) noexcept = 0;

            virtual std::int32_t G6_ABI_CALL version(abi_out_t<param_string> result) noexcept = 0;
        };
    };

    template <typename Derived>
    struct interface_vtable<Derived, startorus::detect_code> : interface_vtable_base<Derived, startorus::detect_code>
    {
        virtual std::int32_t G6_ABI_CALL init(
            abi_in_t<param_string> model_directory,
            abi_in_t<std::int32_t> factory_type,
            std::int32_t device,
            abi_in_t<exposing::param_hash_map<exposing::param_string, float>> param_map_abi) noexcept override
        {
            return abi_safe_call([&]
                {this->self().init(
                    create_from_abi<param_string>(model_directory),
                    create_from_abi<std::int32_t>(factory_type),
                    device,
                    create_from_abi<exposing::param_hash_map<exposing::param_string, float>>(param_map_abi));
                });
        }

        virtual std::int32_t G6_ABI_CALL detect(
            abi_in_t<param_span<std::uint8_t>> bitmap,
            std::int32_t channels,
            std::int32_t height,
            std::int32_t width,
            std::int32_t order,
            abi_in_t<param_vector<int>> rois_abi,
            abi_out_t<param_vector<startorus::box_info>> result) noexcept override
        {
            return abi_safe_call([&]
                {
                    *result = detach_abi(this->self().detect(
                        create_from_abi<param_span<std::uint8_t>>(bitmap),
                        channels,
                        height,
                        width,
                        order,
                        create_from_abi<param_vector<int>>(rois_abi)
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
    struct abi_adapter<startorus::detect_code>
    {
        template <typename Derived>
        struct type : enable_self_abi_awareness<Derived, startorus::detect_code>
        {
            void init(
                const param_string& model_directory,
                std::int32_t factory_type,
                std::int32_t device,
                const exposing::param_hash_map<exposing::param_string, float>& param_map_abi) const
            {
                check_abi_result(this->self_abi().init(
                    get_abi(model_directory),
                    get_abi(factory_type),
                    get_abi(device),
                    get_abi(param_map_abi)));
            }

            param_vector<startorus::box_info> detect(
                param_span<std::uint8_t> bitmap,
                std::int32_t channels,
                std::int32_t height,
                std::int32_t width,
                std::int32_t order,
                param_vector<int> rois_abi) const
            {
                param_vector<startorus::box_info> result{ nullptr };
                return (check_abi_result(this->self_abi().detect(
                    get_abi(bitmap),
                    channels,
                    height,
                    width,
                    order,
                    get_abi(rois_abi),
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

namespace glasssix::startorus
{
    struct detect_code : exposing::inherits<detect_code>
    {
        using inherits::inherits;
    };
}

#endif