#pragma once

#include <abi/consumer.hpp>
#include <plugin_interface.hpp>

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
			virtual std::int32_t G6_ABI_CALL load_from_existing_libraries() noexcept = 0;
			virtual std::int32_t G6_ABI_CALL load_from_file(abi_in_t<param_string> path) noexcept = 0;
			virtual std::int32_t G6_ABI_CALL load_from_directory(abi_in_t<param_string> directory) noexcept = 0;
			virtual std::int32_t G6_ABI_CALL lookup(abi_in_t<param_string> plugin_name, abi_out_t<nessus::plugin_interface> result) noexcept = 0;
			virtual std::int32_t G6_ABI_CALL execute(abi_in_t<param_string> plugin_name, abi_in_t<param_string> function_name, abi_in_t<param_hash_map<param_string, unknown_object>> params, abi_out_t<unknown_object> result) noexcept = 0;
		};
	};

	template<typename Derived>
	struct interface_vtable<Derived, nessus::plugin_manager> : interface_vtable_base<Derived, nessus::plugin_manager>
	{
		virtual std::int32_t G6_ABI_CALL load_from_existing_libraries() noexcept override
		{
			return abi_safe_call([&] { this->self().load_from_existing_libraries(); });
		}

		virtual std::int32_t G6_ABI_CALL load_from_file(abi_in_t<param_string> path) noexcept override
		{
			return abi_safe_call([&] { this->self().load_from_file(create_from_abi<param_string>(path)); });
		}

		virtual std::int32_t G6_ABI_CALL load_from_directory(abi_in_t<param_string> directory) noexcept override
		{
			return abi_safe_call([&] { this->self().load_from_directory(create_from_abi<param_string>(directory)); });
		}

		virtual std::int32_t G6_ABI_CALL lookup(abi_in_t<param_string> plugin_name, abi_out_t<nessus::plugin_interface> result) noexcept override
		{
			return abi_safe_call([&] { *result = detach_abi(this->self().lookup(create_from_abi<param_string>(plugin_name))); });
		}

		virtual std::int32_t G6_ABI_CALL execute(abi_in_t<param_string> plugin_name, abi_in_t<param_string> function_name, abi_in_t<param_hash_map<param_string, unknown_object>> params, abi_out_t<unknown_object> result) noexcept override
		{
			return abi_safe_call([&] { *result = detach_abi(this->self().execute(create_from_abi<param_string>(plugin_name), create_from_abi<param_string>(function_name), create_from_abi<param_hash_map<param_string, unknown_object>>(params))); });
		}
	};

	template<> struct abi_adapter<nessus::plugin_manager>
	{
		template<typename Derived>
		struct type : enable_self_abi_awareness<Derived, nessus::plugin_manager>
		{
			void load_from_existing_libraries() const
			{
				check_abi_result(this->self_abi().load_from_existing_libraries());
			}

			void load_from_file(const param_string& path) const
			{
				check_abi_result(this->self_abi().load_from_file(get_abi(path)));
			}

			void load_from_directory(const param_string& path) const
			{
				check_abi_result(this->self_abi().load_from_directory(get_abi(path)));
			}

			nessus::plugin_interface lookup(const param_string& plugin_name) const
			{
				nessus::plugin_interface result{ nullptr };

				return (check_abi_result(this->self_abi().lookup(get_abi(plugin_name), put_abi(result))), result);
			}

			unknown_object execute(const param_string& plugin_name, const param_string& function_name, const param_hash_map<param_string, unknown_object>& params) const
			{
				unknown_object result{ nullptr };

				return (check_abi_result(this->self_abi().execute(get_abi(plugin_name), get_abi(function_name), get_abi(params), put_abi(result))), result);
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
