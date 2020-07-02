#include <singleton.hpp>
#include <plugin_manager.hpp>
#include <abi/implements.hpp>
#include <plugin_interface.hpp>

namespace glasssix::exposing::nessus
{
	/// <summary>
	/// An agent of the standard plugin manager.
	/// </summary>
	class plugin_manager_impl : public implements<plugin_manager_impl, plugin_manager>, public make_plugin_qualified_name
	{
	public:
		void load_from_file(const param_string& path);
		void load_from_directory(const param_string& path);
		plugin_interface lookup(const param_string& plugin_name);
		unknown_object execute(const param_string& plugin_name, const param_string& function_name, const param_hash_map<param_string, unknown_object>& params);
	};
}
