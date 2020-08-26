#ifndef _PARSER_INTERNAL_HPP_
#define _PARSER_INTERNAL_HPP_

#include "singleton.hpp"

#include <string>
#include <string_view>

//#if defined(_MSC_VER)
//#if defined(PARSER_EXPORT)
//#define PARSER_EXPORT_API __declspec(dllexport)
//#else
//#define PARSER_EXPORT_API __declspec(dllimport)
//#endif
//#else
//#define PARSER_EXPORT_API 
//#endif // if defined(_MSC_VER)

namespace glasssix::exposing::nessus
{
	class [[deprecated("Use the abi interface 'make_export_interface<parser>' instead.")]] parser_internal : public singleton<parser_internal>
	{
	public:
		class impl;
		friend singleton<parser_internal>;

		virtual ~parser_internal();

		std::string parse(std::string_view topic, std::string_view jsonstr);
		std::string query_all_instance();
		std::string support_protocol();
		std::string init_plugin(std::string_view config_file_path);
	private:
		parser_internal();

		impl* impl_;
	};
}

#endif
