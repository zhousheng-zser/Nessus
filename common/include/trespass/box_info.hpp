#ifndef _TRESPASS_BOX_INFO_HPP_
#define _TRESPASS_BOX_INFO_HPP_

#include <abi/consumer.hpp>

namespace glasssix::trespass
{
    struct box_info;
}

namespace glasssix::exposing::impl
{
    template<>
    struct abi<trespass::box_info>
    {
        using identity_type = type_identity_interface;

        static constexpr guid id{ "{5614372A-F63E-49F4-A199-0C07FF9BB65B}" };

        struct type : abi_unknown_object
        {
            virtual std::int32_t G6_ABI_CALL x1(abi_out_t<int> result) noexcept = 0;
            virtual std::int32_t G6_ABI_CALL y1(abi_out_t<int> result) noexcept = 0;
            virtual std::int32_t G6_ABI_CALL x2(abi_out_t<int> result) noexcept = 0;
            virtual std::int32_t G6_ABI_CALL y2(abi_out_t<int> result) noexcept = 0;
            virtual std::int32_t G6_ABI_CALL score(abi_out_t<float> result) noexcept = 0;
			virtual std::int32_t G6_ABI_CALL category(abi_out_t<int> result) noexcept = 0;
            virtual std::int32_t G6_ABI_CALL key_points(abi_out_t<param_vector<float>> result) noexcept = 0;
        };
    };

    template <typename Derived>
    struct interface_vtable<Derived, trespass::box_info> : interface_vtable_base<Derived, trespass::box_info>
    {

        virtual std::int32_t G6_ABI_CALL x1(abi_out_t<int> result) noexcept override
        {
            return abi_safe_call([&]
                { *result = detach_abi(this->self().x1()); });
        }

        virtual std::int32_t G6_ABI_CALL y1(abi_out_t<int> result) noexcept override
        {
            return abi_safe_call([&]
                { *result = detach_abi(this->self().y1()); });
        }

        virtual std::int32_t G6_ABI_CALL x2(abi_out_t<int> result) noexcept override
        {
            return abi_safe_call([&]
                { *result = detach_abi(this->self().x2()); });
        }

        virtual std::int32_t G6_ABI_CALL y2(abi_out_t<int> result) noexcept override
        {
            return abi_safe_call([&]
                { *result = detach_abi(this->self().y2()); });
        }
        virtual std::int32_t G6_ABI_CALL score(abi_out_t<float> result) noexcept override
        {
            return abi_safe_call([&]
                { *result = detach_abi(this->self().score()); });
        }
        virtual std::int32_t G6_ABI_CALL category(abi_out_t<int> result) noexcept override
        {
            return abi_safe_call([&]
                { *result = detach_abi(this->self().category()); });
        }

        virtual std::int32_t G6_ABI_CALL key_points(abi_out_t<param_vector<float>> result) noexcept override
        {
            return abi_safe_call([&]
                { *result = detach_abi(this->self().key_points()); });
        }

    };

    template <>
    struct abi_adapter<trespass::box_info>
    {
        template <typename Derived>
        struct type : enable_self_abi_awareness<Derived, trespass::box_info>
        {

            int x1() const
            {
                int result = 0;

                return (check_abi_result(this->self_abi().x1(put_abi(result))), result);
            }
            int y1() const
            {
                int result = 0;

                return (check_abi_result(this->self_abi().y1(put_abi(result))), result);
            }
            int x2() const
            {
                int result = 0;

                return (check_abi_result(this->self_abi().x2(put_abi(result))), result);
            }
            int y2() const
            {
                int result = 0;

                return (check_abi_result(this->self_abi().y2(put_abi(result))), result);
            }
			float score() const
            {
                float result = 0;

                return (check_abi_result(this->self_abi().score(put_abi(result))), result);
            }
			int category() const
            {
                int result = 0;

                return (check_abi_result(this->self_abi().category(put_abi(result))), result);
            }
            param_vector<float> key_points() const
            {
                param_vector<float> result;

                return (check_abi_result(this->self_abi().key_points(put_abi(result))), result);
            }
        };
    };
}

namespace glasssix::trespass
{
    struct box_info : exposing::inherits<box_info>
    {
        using inherits::inherits;
    };
}
#endif