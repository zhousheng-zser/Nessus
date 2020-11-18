#ifndef _PARSER_IMPL_HPP_
#define _PARSER_IMPL_HPP_
#include "parser.hpp"
#include <abi/implements.hpp>

namespace glasssix::exposing::nessus
{
	inline constexpr utf8_string_view parser_qualified_name{ u8"glasssix.nessus.parser" };

	/// <summary>
	/// An agent of the standard parser.
	/// </summary>
	class parser_impl : public implements<parser_impl, parser>, public make_external_qualified_name<parser_qualified_name>
	{
	public:
		class impl;

		parser_impl();
		~parser_impl();
		param_string parse(const param_string& topic, const param_string& str_param, param_span<std::uint8_t> data);
		param_string query_all_instance();
		param_string support_protocol();
		param_string init_plugin(const param_string& config_file_path);

	private:
		impl* impl_;
	};
}
#endif