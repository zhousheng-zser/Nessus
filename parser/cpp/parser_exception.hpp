#ifndef _PARSER_EXCEPTION_HPP_
#define _PARSER_EXCEPTION_HPP_

#include <exception>
#include <string>

namespace glasssix
{
	namespace exposing
	{
		namespace nessus
		{
			class parser_exception : public std::exception
			{
			public:
				parser_exception() : message("Error.") {}
				parser_exception(std::string &str) : message(str) {}
				parser_exception(const char *str) : message(str) {}
				~parser_exception() throw () {
				}

				virtual const char* what() const throw () {
					return message.c_str();
				}

			private:
				std::string message;
			};
		}
	}
}

#endif
