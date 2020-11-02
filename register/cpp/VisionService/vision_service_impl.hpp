#pragma once

#include "vision_service.hpp"
#include "PluginRegister/plugin_interface.hpp"

#include <memory>

#include <abi/consumer.hpp>

namespace glasssix::exposing::nessus
{
	class vision_service_impl : public implements<vision_service_impl, vision_service>, public make_plugin_qualified_name
	{
	public:
		class impl;

		vision_service_impl();
		~vision_service_impl();
		param_string name() const;
		param_string version() const;
		param_vector<param_string> available_functions() const;
		param_hash_map<guid, param_string> existing_instances() const;
		unknown_object execute(const param_string& function_name, const param_hash_map<param_string, unknown_object>& params) const;
	private:
		std::unique_ptr<impl> impl_;
	};
}
