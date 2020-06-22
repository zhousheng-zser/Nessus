#include "plugin_manager_impl.hpp"

namespace glasssix::exposing::nessus
{
	void plugin_manager_concrete_impl::load_from_file(const param_string& path)
	{
		if (auto factory = component_loader::instance().add_module_with_factory(path); factory && factory.get_qualified_names().contains(plugin_qualified_name))
		{
			create_plugin(factory);
		}
	}

	void plugin_manager_concrete_impl::load_from_directory(const param_string& path)
	{
		if (auto factories = component_loader::instance().add_modules_with_factories_in_directory(path); factories && !factories.empty())
		{
			for (const auto& item : factories)
			{
				create_plugin(item);
			}
		}
	}

	plugin_interface plugin_manager_concrete_impl::lookup(const param_string& plugin_name)
	{
		std::lock_guard<std::mutex> lock{ lock_ };
		auto iter = plugins_.find(plugin_name);

		return iter != plugins_.end() ? iter->second : nullptr;
	}

	unknown_object plugin_manager_concrete_impl::execute(const param_string& plugin_name, const param_string& function_name, const param_vector<unknown_object>& params)
	{
		auto plugin = lookup(plugin_name);
		
		return plugin ? plugin.execute(function_name, params) : throw abi_key_not_found{};
	}

	void plugin_manager_concrete_impl::create_plugin(const class_factory& item)
	{
		auto plugin = item.create_instance(plugin_qualified_name).as<plugin_interface>();
		{
			std::lock_guard<std::mutex> lock{ lock_ };

			plugins_.insert_or_assign(plugin.name(), plugin);
		}
	}

	void plugin_manager_impl::load_from_file(const param_string& path)
	{
		plugin_manager_concrete_impl::instance().load_from_file(path);
	}

	void plugin_manager_impl::load_from_directory(const param_string& path)
	{
		plugin_manager_concrete_impl::instance().load_from_directory(path);
	}

	plugin_interface plugin_manager_impl::lookup(const param_string& plugin_name)
	{
		return plugin_manager_concrete_impl::instance().lookup(plugin_name);
	}

	unknown_object plugin_manager_impl::execute(const param_string& plugin_name, const param_string& function_name, const param_vector<unknown_object>& params)
	{
		return plugin_manager_concrete_impl::instance().execute(plugin_name, function_name, params);
	}
}
