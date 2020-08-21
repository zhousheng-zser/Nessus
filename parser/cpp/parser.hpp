#ifndef _PARSER_HPP_
#define _PARSER_HPP_

#include "singleton.hpp"

#include <string>
#include <string_view>

namespace glasssix::exposing::nessus
{
	class parser : public singleton<parser>
	{
	public:
		class impl;
		friend singleton<parser>;

		virtual ~parser();

		std::string parse(std::string_view topic, std::string_view jsonstr);
		std::string query_all_instance();
		std::string support_protocol();
		std::string init_plugin(std::string_view config_file_path);
	private:
		parser();

		impl* impl_;
	};
}

#endif
