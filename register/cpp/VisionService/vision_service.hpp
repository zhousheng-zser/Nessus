#pragma once

#include "PluginRegister/plugin_interface.hpp"

#include <functional>
#include <unordered_map>

#include <abi/consumer.hpp>
#include <abi/implements.hpp>

namespace glasssix::exposing::nessus
{
	class vision_service : public implements<vision_service, plugin_interface>, public make_plugin_qualified_name
	{
	public:
		vision_service();
		param_string name() const;
		param_string version() const;
		param_vector<param_string> get_available_functions() const;
		unknown_object execute(const param_string& function_name, const param_hash_map<param_string, unknown_object>& params) const;
	private:
		std::unordered_map<param_string, std::function<unknown_object(const param_hash_map<param_string, unknown_object>&)>> functions_;
	};
}
