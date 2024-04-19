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

		algo_plugin_interface lookup(const guid& id)
		{
			std::scoped_lock lock{ mutex_ };
			auto iter = plugin_instances_map_.find(id);

			return iter != plugin_instances_map_.end() ? iter->second : nullptr;
		}

		guid create_algo_instance(const param_string& qualified_name, const param_string& str_params)
		{
			auto instance = get_component_loader().create_by_name(qualified_name).try_as<algo_plugin_interface>();
			if (instance)
			{
				instance.init(str_params);
				auto instance_id = create_guid_from_bytes(meta::to_array(reinterpret_cast<std::size_t>(get_abi(instance))));
				std::scoped_lock lock{ mutex_ };
				plugin_instances_map_.insert_or_assign(instance_id, instance);
				return instance_id;
			}
			else
				throw abi_no_interface{ u8"using '" + qualified_name + "' to create instance failed."};
		}

		param_string execute(const guid& instance_id, const param_hash_map<param_string, unknown_object>& input_params_map)
		{
			auto instance = lookup(instance_id);
			if (instance)
			{
				return instance.execute(input_params_map);
			}
			else
				throw abi_key_not_found{};
		}

		void release_algo_instance(const guid& instance_id)
		{
			std::scoped_lock lock1{ mutex_ };
			plugin_instances_map_.erase(instance_id);
		}

	private:
		std::mutex mutex_;
		std::unordered_map<guid, algo_plugin_interface> plugin_instances_map_;
	};

	guid plugin_manager_impl::create_algo_instance(const param_string& qualified_name, const param_string& str_params)
	{
		return plugin_manager_concrete_impl::instance().create_algo_instance(qualified_name, str_params);
	}

	algo_plugin_interface plugin_manager_impl::lookup(const guid& id)
	{
		return plugin_manager_concrete_impl::instance().lookup(id);
	}

	param_string plugin_manager_impl::execute(const guid& id, const param_hash_map<param_string, unknown_object>& input_params_map)
	{
		return plugin_manager_concrete_impl::instance().execute(id, input_params_map);
	}

	void plugin_manager_impl::release_algo_instance(const guid& instance_id)
	{
		plugin_manager_concrete_impl::instance().release_algo_instance(instance_id);
	}
}
