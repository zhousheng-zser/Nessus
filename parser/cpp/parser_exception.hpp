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
				enum class parser_exception_code 
				{
					NO_EXCEPTION = 0,
					INTERNAL_FUNCTION_FAILURE = -1,
					INVALID_ARGUMENT = -4,
					INVALID_OPERATION = -7,
					JSON_EXCEPTION = -98,
					UNKNOWN_EXCEPTION = -99
				};
				parser_exception() : code_(parser_exception_code::UNKNOWN_EXCEPTION), message_("Unknown Exception.") {}
				parser_exception(parser_exception_code code, std::string &str) :code_(code), message_(str) {}
				parser_exception(parser_exception_code code, const char *str) :code_(code), message_(str) {}
				~parser_exception() throw () {
				}

				virtual const char* what() const throw () {
					return message_.c_str();
				}
				parser_exception_code what_code() const throw () {
					return code_;
				}

			private:
				parser_exception_code code_;
				std::string message_;
			};
		}
	}
}

#endif
