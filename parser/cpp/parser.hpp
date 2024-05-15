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
			virtual std::int32_t G6_ABI_CALL create_instance(abi_in_t<param_string> qualified_name, abi_in_t<param_string> str_param, abi_out_t<guid> result) = 0;
			virtual std::int32_t G6_ABI_CALL execute(abi_in_t<guid> instance_id, abi_in_t<param_string> str_param, 
				abi_in_t<param_span<std::uint8_t>> input_data, abi_in_t<param_span<std::uint8_t>> reserve_output_data, abi_out_t<param_string> result) = 0;
			virtual std::int32_t G6_ABI_CALL release_instance(abi_in_t<guid> instance_id) noexcept = 0;
			virtual std::int32_t G6_ABI_CALL init_plugin(abi_in_t<param_string> config_file_path) = 0;
		};
	};

	template<typename Derived>
	struct interface_vtable<Derived, nessus::parser> : interface_vtable_base<Derived, nessus::parser>
	{
		virtual std::int32_t G6_ABI_CALL create_instance(abi_in_t<param_string> qualified_name, abi_in_t<param_string> str_param, abi_out_t<guid> result) override
		{
			return abi_safe_call([&] { *result = detach_abi(this->self().create_instance(create_from_abi<param_string>(qualified_name), create_from_abi<param_string>(str_param))); });
		}

		virtual std::int32_t G6_ABI_CALL execute(abi_in_t<guid> instance_id, abi_in_t<param_string> str_param,
			abi_in_t<param_span<std::uint8_t>> input_data, abi_in_t<param_span<std::uint8_t>> reserve_output_data, abi_out_t<param_string> result) override
		{
			return abi_safe_call([&] { *result = detach_abi(this->self().execute(create_from_abi<guid>(instance_id), create_from_abi<param_string>(str_param), 
				create_from_abi<param_span<std::uint8_t>>(input_data), create_from_abi<param_span<std::uint8_t>>(reserve_output_data))); });
		}

		virtual std::int32_t G6_ABI_CALL release_instance(abi_in_t<guid> instance_id) noexcept override
		{
			return abi_safe_call([&] { this->self().release_instance(create_from_abi<guid>(instance_id)); });
		}

		virtual std::int32_t G6_ABI_CALL init_plugin(abi_in_t<param_string> config_file_path) override
		{
			return abi_safe_call([&] { this->self().init_plugin(create_from_abi<param_string>(config_file_path)); });
		}
	};

	template<> struct abi_adapter<nessus::parser>
	{
		template<typename Derived>
		struct type : enable_self_abi_awareness<Derived, nessus::parser>
		{
			guid create_instance(const param_string& qualified_name, const param_string& str_param) const
			{
				guid result;
				return (check_abi_result(this->self_abi().create_instance(get_abi(qualified_name), get_abi(str_param), put_abi(result))), result);
			}

			param_string execute(const guid& instance_id, const param_string& str_param, param_span<std::uint8_t> input_data, param_span<std::uint8_t> reserve_output_data) const
			{
				param_string result{ nullptr };
				return (check_abi_result(this->self_abi().execute(get_abi(instance_id), get_abi(str_param), get_abi(input_data), get_abi(reserve_output_data), put_abi(result))), result);
			}

			void release_instance(const guid& instance_id)
			{
				check_abi_result(this->self_abi().release_instance(get_abi(instance_id)));
			}

			void init_plugin(const param_string& config_file_path) const
			{
				check_abi_result(this->self_abi().init_plugin(get_abi(config_file_path)));
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