#pragma once

#include <abi/consumer.hpp>
#include <algo_plugin_interface.hpp>

namespace glasssix::exposing::nessus
{
	struct plugin_manager;
}

namespace glasssix::exposing::impl
{
	template<> struct abi<nessus::plugin_manager>
	{
		using identity_type = type_identity_interface;
		static constexpr guid id{ "DDEBC855-5201-4390-A438-DF1AE2F4A6DE" };

		struct type : abi_unknown_object
		{
			virtual std::int32_t G6_ABI_CALL create_algo_instance(abi_in_t<param_string> qualified_name, abi_in_t<param_string> str_params, abi_out_t<guid> result) = 0;
			virtual std::int32_t G6_ABI_CALL lookup(abi_in_t<guid> id, abi_out_t<nessus::algo_plugin_interface> result) noexcept = 0;
			virtual std::int32_t G6_ABI_CALL execute(abi_in_t<guid> id, abi_in_t<param_hash_map<param_string, unknown_object>> input_params_map, abi_out_t<param_string> result) = 0;
			virtual std::int32_t G6_ABI_CALL release_algo_instance(abi_in_t<guid> instance_id) noexcept = 0;
		};
	};

	template<typename Derived>
	struct interface_vtable<Derived, nessus::plugin_manager> : interface_vtable_base<Derived, nessus::plugin_manager>
	{
		virtual std::int32_t G6_ABI_CALL create_algo_instance(abi_in_t<param_string> qualified_name, abi_in_t<param_string> str_params, abi_out_t<guid> result) override
		{
			return abi_safe_call([&] { *result = detach_abi(this->self().create_algo_instance(create_from_abi<param_string>(qualified_name), create_from_abi<param_string>(str_params))); });
		}

		virtual std::int32_t G6_ABI_CALL lookup(abi_in_t<guid> id, abi_out_t<nessus::algo_plugin_interface> result) noexcept override
		{
			return abi_safe_call([&] { *result = detach_abi(this->self().lookup(create_from_abi<guid>(id))); });
		}

		virtual std::int32_t G6_ABI_CALL execute(abi_in_t<guid> id, abi_in_t<param_hash_map<param_string, unknown_object>> input_params_map, abi_out_t<param_string> result) override
		{
			return abi_safe_call([&] { *result = detach_abi(this->self().execute(create_from_abi<guid>(id), create_from_abi<param_hash_map<param_string, unknown_object>>(input_params_map))); });
		}

		virtual std::int32_t G6_ABI_CALL release_algo_instance(abi_in_t<guid> instance_id) noexcept override
		{
			return abi_safe_call([&] { this->self().release_algo_instance(create_from_abi<guid>(instance_id)); });
		}
	};

	template<> struct abi_adapter<nessus::plugin_manager>
	{
		template<typename Derived>
		struct type : enable_self_abi_awareness<Derived, nessus::plugin_manager>
		{
			guid create_algo_instance(const param_string& qualified_name, const param_string& str_params)
			{
				guid result;

				return (check_abi_result(this->self_abi().create_algo_instance(get_abi(qualified_name), get_abi(str_params), put_abi(result))), result);
			}

			nessus::algo_plugin_interface lookup(const guid& id) const
			{
				nessus::algo_plugin_interface result{ nullptr };

				return (check_abi_result(this->self_abi().lookup(get_abi(id), put_abi(result))), result);
			}

			param_string execute(const guid& id, const param_hash_map<param_string, unknown_object>& input_params_map) const
			{
				param_string result{ nullptr };

				return (check_abi_result(this->self_abi().execute(get_abi(id), get_abi(input_params_map), put_abi(result))), result);
			}

			void release_algo_instance(const guid& instance_id)
			{
				check_abi_result(this->self_abi().release_algo_instance(get_abi(instance_id)));
			}
		};
	};
}

namespace glasssix::exposing::nessus
{
	struct plugin_manager : inherits<plugin_manager>
	{
		using inherits::inherits;
	};
}
