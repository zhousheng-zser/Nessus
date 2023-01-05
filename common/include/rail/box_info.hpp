#ifndef _RAIL_BOX_INFO_HPP_
#define _RAIL_BOX_INFO_HPP_

#include <abi/consumer.hpp>

namespace glasssix::rail
{
    struct box_info;
}

namespace glasssix::exposing::impl
{
    template<>
    struct abi<rail::box_info>
    {
        using identity_type = type_identity_interface;

        static constexpr guid id{ "{4E399FFA-49E0-41B4-B72E-A4EFC3204349}" };

        struct type : abi_unknown_object
        {
            virtual std::int32_t G6_ABI_CALL x1(abi_out_t<int> result) noexcept = 0;
            virtual std::int32_t G6_ABI_CALL y1(abi_out_t<int> result) noexcept = 0;
            virtual std::int32_t G6_ABI_CALL x2(abi_out_t<int> result) noexcept = 0;
            virtual std::int32_t G6_ABI_CALL y2(abi_out_t<int> result) noexcept = 0;
			virtual std::int32_t G6_ABI_CALL category(abi_out_t<int> result) noexcept = 0;

            virtual std::int32_t G6_ABI_CALL set_x1(abi_in_t<int> input) noexcept = 0;
            virtual std::int32_t G6_ABI_CALL set_y1(abi_in_t<int> input) noexcept = 0;
            virtual std::int32_t G6_ABI_CALL set_x2(abi_in_t<int> input) noexcept = 0;
            virtual std::int32_t G6_ABI_CALL set_y2(abi_in_t<int> input) noexcept = 0;
			virtual std::int32_t G6_ABI_CALL set_category(abi_in_t<int> input) noexcept = 0;
        };
    };

    template <typename Derived>
    struct interface_vtable<Derived, rail::box_info> : interface_vtable_base<Derived, rail::box_info>
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
        virtual std::int32_t G6_ABI_CALL category(abi_out_t<int> result) noexcept override
        {
            return abi_safe_call([&]
                                 { *result = detach_abi(this->self().category()); });
        }

        virtual std::int32_t G6_ABI_CALL set_x1(abi_in_t<int> input) noexcept override
        {
            return abi_safe_call([&]
                { this->self().set_x1(create_from_abi<int>(input)); });
        }
        virtual std::int32_t G6_ABI_CALL set_y1(abi_in_t<int> input) noexcept override
        {
            return abi_safe_call([&]
                { this->self().set_y1(create_from_abi<int>(input)); });
        }
        virtual std::int32_t G6_ABI_CALL set_x2(abi_in_t<int> input) noexcept override
        {
            return abi_safe_call([&]
                { this->self().set_x2(create_from_abi<int>(input)); });
        }
        virtual std::int32_t G6_ABI_CALL set_y2(abi_in_t<int> input) noexcept override
        {
            return abi_safe_call([&]
                { this->self().set_y2(create_from_abi<int>(input)); });
        }
        virtual std::int32_t G6_ABI_CALL set_category(abi_in_t<int> input) noexcept override
        {
            return abi_safe_call([&]
                                 { this->self().set_category(create_from_abi<int>(input)); });
        }
    };

    template <>
    struct abi_adapter<rail::box_info>
    {
        template <typename Derived>
        struct type : enable_self_abi_awareness<Derived, rail::box_info>
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
			int category() const
            {
                int result = 0;

                return (check_abi_result(this->self_abi().category(put_abi(result))), result);
            }

            void set_x1(int input)
            {
                check_abi_result(this->self_abi().set_x1(get_abi(input)));
            }
            void set_y1(int input)
            {
                check_abi_result(this->self_abi().set_y1(get_abi(input)));
            }
            void set_x2(int input)
            {
                check_abi_result(this->self_abi().set_x2(get_abi(input)));
            }
            void set_y2(int input)
            {
                check_abi_result(this->self_abi().set_y2(get_abi(input)));
            }
            void set_category(int input)
            {
                check_abi_result(this->self_abi().set_category(get_abi(input)));
            }
        };
    };
}

namespace glasssix::rail
{
    struct box_info : exposing::inherits<box_info>
    {
        using inherits::inherits;
    };
}
#endif