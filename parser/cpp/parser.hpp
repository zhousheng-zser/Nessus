#pragma once
#ifndef _PARSER_HPP_
#define _PARSER_HPP_

#include <abi/consumer.hpp>

namespace glasssix::exposing::nessus
{
	struct parser;
}

namespace glasssix::exposing::impl
{
	template<> struct abi<nessus::parser>
	{
		using identity_type = type_identity_interface;
		static constexpr guid id{ "A9B635E2-0217-4343-9215-7EF1E0920DDD" };

		struct type : abi_unknown_object
		{
			virtual std::int32_t G6_ABI_CALL parse(abi_in_t<param_string> protocol, abi_in_t<param_string> jsonstr, abi_out_t<param_string> result) noexcept = 0;
			virtual std::int32_t G6_ABI_CALL query_all_instance(abi_out_t<param_string> result) noexcept = 0;
			virtual std::int32_t G6_ABI_CALL support_protocol(abi_out_t<param_string> result) noexcept = 0;
			virtual std::int32_t G6_ABI_CALL init_plugin(abi_in_t<param_string> config_file_path, abi_out_t<param_string> result) noexcept = 0;
		};
	};

	template<typename Derived>
	struct interface_vtable<Derived, nessus::parser> : interface_vtable_base<Derived, nessus::parser>
	{
		virtual std::int32_t G6_ABI_CALL parse(abi_in_t<param_string> protocol, abi_in_t<param_string> jsonstr, abi_out_t<param_string> result) noexcept override
		{
			return abi_safe_call([&] { *result = detach_abi(this->self().parse(create_from_abi<param_string>(protocol), create_from_abi<param_string>(jsonstr))); });
		}

		virtual std::int32_t G6_ABI_CALL query_all_instance(abi_out_t<param_string> result) noexcept override
		{
			return abi_safe_call([&] { *result = detach_abi(this->self().query_all_instance()); });
		}

		virtual std::int32_t G6_ABI_CALL support_protocol(abi_out_t<param_string> result) noexcept override
		{
			return abi_safe_call([&] { *result = detach_abi(this->self().support_protocol()); });
		}

		virtual std::int32_t G6_ABI_CALL init_plugin(abi_in_t<param_string> config_file_path, abi_out_t<param_string> result) noexcept override
		{
			return abi_safe_call([&] { *result = detach_abi(this->self().init_plugin(create_from_abi<param_string>(config_file_path))); });
		}
	};

	template<> struct abi_adapter<nessus::parser>
	{
		template<typename Derived>
		struct type : enable_self_abi_awareness<Derived, nessus::parser>
		{
			param_string parse(const param_string& protocol, const param_string& jsonstr) const
			{
				param_string result{ nullptr };
				return (check_abi_result(this->self_abi().parse(get_abi(protocol), get_abi(jsonstr), put_abi(result))), result);
			}

			param_string query_all_instance() const
			{
				param_string result{ nullptr };
				return (check_abi_result(this->self_abi().query_all_instance(put_abi(result))), result);
			}

			param_string support_protocol() const
			{
				param_string result{ nullptr };
				return (check_abi_result(this->self_abi().support_protocol(put_abi(result))), result);
			}

			param_string init_plugin(const param_string& config_file_path) const
			{
				param_string result{ nullptr };
				return (check_abi_result(this->self_abi().init_plugin(get_abi(config_file_path), put_abi(result))), result);
			}
		};
	};
}

namespace glasssix::exposing::nessus
{
	struct parser : inherits<parser>
	{
		using inherits::inherits;
	};
}

#endif