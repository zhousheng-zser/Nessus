#ifndef _VALVE_CODE_BOX_INFO_HPP_
#define _VALVE_CODE_BOX_INFO_HPP_

#include <abi/consumer.hpp>

namespace glasssix::valve
{
    struct box_info;
}

namespace glasssix::exposing::impl
{
    template <>
    struct abi<valve::box_info>
    {
        using identity_type = type_identity_interface;

        static constexpr guid id{ "{9525C196-6320-4CBF-B5CC-51A391A3BF5D}" };

        struct type : abi_unknown_object
        {
            virtual std::int32_t G6_ABI_CALL location(abi_out_t<param_vector<float>> result) noexcept = 0;
            virtual std::int32_t G6_ABI_CALL name(abi_out_t<param_string> result) noexcept = 0;
            virtual std::int32_t G6_ABI_CALL task(abi_out_t<int> result) noexcept = 0;
            virtual std::int32_t G6_ABI_CALL status(abi_out_t<int> result) noexcept = 0;
            virtual std::int32_t G6_ABI_CALL score(abi_out_t<float> result) noexcept = 0;
        };
    };

    template <typename Derived>
    struct interface_vtable<Derived, valve::box_info> : interface_vtable_base<Derived, valve::box_info>
    {
        virtual std::int32_t G6_ABI_CALL location(abi_out_t<param_vector<int>> result) noexcept override
        {
            return abi_safe_call([&]
                                 { *result = detach_abi(this->self().location()); });
        }

        virtual std::int32_t G6_ABI_CALL name(abi_out_t<param_string> result) noexcept override
        {
            return abi_safe_call([&]
                                 { *result = detach_abi(this->self().name()); });
        }

        virtual std::int32_t G6_ABI_CALL task(abi_out_t<int> result) noexcept override
        {
            return abi_safe_call([&]
                { *result = detach_abi(this->self().task()); });
        }

        virtual std::int32_t G6_ABI_CALL status(abi_out_t<int> result) noexcept override
        {
            return abi_safe_call([&]
                { *result = detach_abi(this->self().status()); });
        }

        virtual std::int32_t G6_ABI_CALL score(abi_out_t<float> result) noexcept override
        {
            return abi_safe_call([&]
                { *result = detach_abi(this->self().score()); });
        }
    };

    template <>
    struct abi_adapter<valve::box_info>
    {
        template <typename Derived>
        struct type : enable_self_abi_awareness<Derived, valve::box_info>
        {

            param_vector<int> location() const
            {
                param_vector<int> result;

                return (check_abi_result(this->self_abi().location(put_abi(result))), result);
            }

            param_string name() const
            {
                param_string result;

                return (check_abi_result(this->self_abi().name(put_abi(result))), result);
            }

            int task() const
            {
                int result;

                return (check_abi_result(this->self_abi().task(put_abi(result))), result);
            }

            int status() const
            {
                int result;

                return (check_abi_result(this->self_abi().status(put_abi(result))), result);
            }

            float score() const
            {
                float result;

                return (check_abi_result(this->self_abi().score(put_abi(result))), result);
            }
        };
    };
}

namespace glasssix::valve
{
    struct box_info : exposing::inherits<box_info>
    {
        using inherits::inherits;
    };
}
#endif