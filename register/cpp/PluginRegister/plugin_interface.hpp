#pragma once

#include <abi/consumer.hpp>

namespace glasssix::exposing::nessus
{
	struct plugin_interface;

	inline constexpr utf8_string_view plugin_qualified_name{ u8"glasssix.nessus.plugin" };
	using make_plugin_qualified_name = make_external_qualified_name<plugin_qualified_name>;
}

namespace glasssix::exposing::impl
{
	template<> struct abi<nessus::plugin_interface>
	{
		using identity_type = type_identity_interface;
		static constexpr guid id{ "415DEBC1-B2D6-4D50-8BB6-1DDD7626AF8B" };

		struct type : abi_unknown_object
		{
			virtual std::int32_t G6_ABI_CALL name(abi_out_t<param_string> result) noexcept = 0;
			virtual std::int32_t G6_ABI_CALL version(abi_out_t<param_string> result) noexcept = 0;
			virtual std::int32_t G6_ABI_CALL get_available_functions(abi_out_t<param_vector<param_string>> result) noexcept = 0;
			virtual std::int32_t G6_ABI_CALL execute(abi_in_t<param_string> function_name, abi_in_t<param_hash_map<param_string, unknown_object>> params, abi_out_t<unknown_object> result) noexcept = 0;
		};
	};

	template<typename Derived>
	struct interface_vtable<Derived, nessus::plugin_interface> : interface_vtable_base<Derived, nessus::plugin_interface>
	{
		virtual std::int32_t G6_ABI_CALL name(abi_out_t<param_string> result) noexcept override
		{
			return abi_safe_call([&] { *result = detach_abi(this->self().name()); });
		}

		virtual std::int32_t G6_ABI_CALL version(abi_out_t<param_string> result) noexcept override
		{
			return abi_safe_call([&] { *result = detach_abi(this->self().version()); });
		}

		virtual std::int32_t G6_ABI_CALL get_available_functions(abi_out_t<param_vector<param_string>> result) noexcept override
		{
			return abi_safe_call([&] { *result = detach_abi(this->self().get_available_functions()); });
		}

		virtual std::int32_t G6_ABI_CALL execute(abi_in_t<param_string> function_name, abi_in_t<param_hash_map<param_string, unknown_object>> params, abi_out_t<unknown_object> result) noexcept override
		{
			return abi_safe_call([&] { *result = detach_abi(this->self().execute(create_from_abi<param_string>(function_name), create_from_abi<param_hash_map<param_string, unknown_object>>(params))); });
		}
	};

	template<> struct abi_adapter<nessus::plugin_interface>
	{
		template<typename Derived>
		struct type : enable_self_abi_awareness<Derived, nessus::plugin_interface>
		{
			param_string name() const
			{
				param_string result{ nullptr };

				return (check_abi_result(this->self_abi().name(put_abi(result))), result);
			}

			param_string version() const
			{
				param_string result{ nullptr };

				return (check_abi_result(this->self_abi().version(put_abi(result))), result);
			}

			param_vector<param_string> get_available_functions() const
			{
				param_vector<param_string> result{ nullptr };

				return (check_abi_result(this->self_abi().get_available_functions(put_abi(result))), result);
			}

			unknown_object execute(const param_string& function_name, const param_hash_map<param_string, unknown_object>& params) const
			{
				unknown_object result{ nullptr };

				return (check_abi_result(this->self_abi().execute(get_abi(function_name), get_abi(params), put_abi(result))), result);
			}
		};
	};
}

namespace glasssix::exposing::nessus
{
	struct plugin_interface : inherits<plugin_interface>
	{
		using inherits::inherits;
	};
}
