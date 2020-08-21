#include "plugin_manager_impl.hpp"

#include <mutex>
#include <iostream>
#include <unordered_map>

namespace glasssix::exposing::nessus
{
	/// <summary>
	/// An implementation of the standard plugin manager.
	/// </summary>
	class plugin_manager_concrete_impl : public singleton<plugin_manager_concrete_impl>
	{
	public:
		friend singleton<plugin_manager_concrete_impl>;

		void load_from_file(const param_string& path)
		{
			if (auto factory = component_loader::instance().add_module_with_factory(path); factory && factory.qualified_names().contains(plugin_qualified_name))
			{
				create_plugin(factory);
			}
		}

		void load_from_directory(const param_string& path)
		{
			if (auto factories = component_loader::instance().add_modules_with_factories_in_directory(path); factories && !factories.empty())
			{
				for (const auto& item : factories)
				{
					create_plugin(item.value());
				}
			}
		}

		plugin_interface lookup(const param_string& plugin_name)
		{
			std::lock_guard<std::mutex> lock{ lock_ };
			auto iter = plugins_.find(plugin_name);

			return iter != plugins_.end() ? iter->second : nullptr;
		}

		unknown_object execute(const param_string& plugin_name, const param_string& function_name, const param_hash_map<param_string, unknown_object>& params)
		{
			auto plugin = lookup(plugin_name);

			return plugin ? plugin.execute(function_name, params) : throw abi_key_not_found{};
		}
	private:
		void create_plugin(const class_factory& item)
		{
			if (auto plugin = item.create_by_name(plugin_qualified_name).try_as<plugin_interface>())
			{
				std::lock_guard<std::mutex> lock{ lock_ };

				plugins_.insert_or_assign(plugin.name(), plugin);
			}
		}

		std::mutex lock_;
		std::unordered_map<param_string, plugin_interface> plugins_;
	};

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

	unknown_object plugin_manager_impl::execute(const param_string& plugin_name, const param_string& function_name, const param_hash_map<param_string, unknown_object>& params)
	{
		return plugin_manager_concrete_impl::instance().execute(plugin_name, function_name, params);
	}
}
