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

		parser_impl();
		~parser_impl();
		guid create_instance(const param_string& qualified_name, const param_string& str_param);
		param_string execute(const guid& instance_id, const param_string& str_param, const param_span<std::uint8_t> img_data, const int height, const int width, const int img_format, bool is_base64, param_span<std::uint8_t> output_data);
		void release_instance(const guid& instance_id);
		void init_plugin(const param_string& config_file_path);
	};
}
#endif