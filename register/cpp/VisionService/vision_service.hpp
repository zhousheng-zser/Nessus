#pragma once

#include "PluginRegister/plugin_interface.hpp"

#include <abi/consumer.hpp>

namespace glasssix::exposing::nessus
{
	struct vision_service;
}

namespace glasssix::exposing::impl
{
	template<> struct abi<nessus::vision_service>
	{
		using identity_type = type_identity_interface;
		static constexpr guid id{ "A99759CE-3D90-4684-BB51-963160CBDF02" };

		struct type : abi_unknown_object
		{
			virtual std::int32_t G6_ABI_CALL existing_instances(abi_out_t<param_hash_map<guid, param_string>> result) noexcept = 0;
		};
	};

	template<typename Derived>
	struct interface_vtable<Derived, nessus::vision_service> : interface_vtable_base<Derived, nessus::vision_service>
	{
		virtual std::int32_t G6_ABI_CALL existing_instances(abi_out_t<param_hash_map<guid, param_string>> result) noexcept override
		{
			return abi_safe_call([&] { *result = detach_abi(this->self().existing_instances()); });
		}
	};

	template<> struct abi_adapter<nessus::vision_service>
	{
		template<typename Derived>
		struct type : enable_self_abi_awareness<Derived, nessus::vision_service>
		{
			param_hash_map<guid, param_string> existing_instances() const
			{
				param_hash_map<guid, param_string> result{ nullptr };

				return (check_abi_result(this->self_abi().existing_instances(put_abi(result))), result);
			}
		};
	};
}

namespace glasssix::exposing::nessus
{
	struct vision_service : inherits<vision_service, plugin_interface>
	{
		using inherits<vision_service, plugin_interface>::inherits;
	};
}
