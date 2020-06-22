#include "plugin_manager.hpp"
#include "abi/implements.hpp"
#include "plugin_interface.hpp"
#include "singleton.hpp"

#include <mutex>
#include <unordered_map>

namespace glasssix::exposing::nessus
{
	inline constexpr utf8_string_view plugin_manager_qualified_name{ u8"glasssix.nessus.pluginManager" };

	/// <summary>
	/// An implementation of the standard plugin manager.
	/// </summary>
	class plugin_manager_concrete_impl : public singleton<plugin_manager_concrete_impl>
	{
	public:
		friend singleton<plugin_manager_concrete_impl>;

		void load_from_file(const param_string& path);
		void load_from_directory(const param_string& path);
		plugin_interface lookup(const param_string& plugin_name);
		unknown_object execute(const param_string& plugin_name, const param_string& function_name, const param_vector<unknown_object>& params);
	private:
		void create_plugin(const class_factory& item);

		std::mutex lock_;
		std::unordered_map<param_string, plugin_interface> plugins_;
	};

	/// <summary>
	/// An agent of the standard plugin manager.
	/// </summary>
	class plugin_manager_impl : public implements<plugin_manager_impl, plugin_manager>, public make_external_qualified_name<plugin_manager_qualified_name>
	{
	public:
		void load_from_file(const param_string& path);
		void load_from_directory(const param_string& path);
		plugin_interface lookup(const param_string& plugin_name);
		unknown_object execute(const param_string& plugin_name, const param_string& function_name, const param_vector<unknown_object>& params);
	};
}
