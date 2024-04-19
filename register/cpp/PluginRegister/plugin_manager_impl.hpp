#include <singleton.hpp>
#include <plugin_manager.hpp>
#include <abi/implements.hpp>
#include <algo_plugin_interface.hpp>

namespace glasssix::exposing::nessus
{
	inline constexpr utf8_string_view plugin_manager_qualified_name{ u8"glasssix.nessus.pluginManager" };

	/// <summary>
	/// An agent of the standard plugin manager.
	/// </summary>
	class plugin_manager_impl : public implements<plugin_manager_impl, plugin_manager>, public make_external_qualified_name<plugin_manager_qualified_name>
	{
	public:
		guid create_algo_instance(const param_string& qualified_name, const param_string& str_params);
		algo_plugin_interface lookup(const guid& id);
		param_string execute(const guid& id, const param_hash_map<param_string, unknown_object>& input_params_map);
		void release_algo_instance(const guid& id);
	};
}
