#include "parser_c.hpp"
#include "parser.hpp"
#include "json.h"
#include "parser_exception.hpp"

#include <memory.hpp>

#ifndef G6_DISABLE_LICENSE
#include <vulcanus/license.hpp>
#endif

static glasssix::exposing::nessus::parser parser_object;
bool parser_module_ready = []
{
	if (glasssix::exposing::get_component_loader().add_module_by_name("parser"))
	{
		try
		{
			parser_object = glasssix::exposing::make_exported_interface<glasssix::exposing::nessus::parser>();
		}
		catch (...)
		{
			return false;
		}
		return true;
	}
	else
		return false;
}();

#ifdef __cplusplus
extern "C" {
#endif

	PARSER_C_EXPORT char* parser_init_plugin(const char* config_file_path, const char* license_key)
	{
		Json::FastWriter writer;
		Json::Value value;
#ifndef G6_DISABLE_LICENSE

		init_license_system(license_key);
		
		try
		{
			glasssix::license::check_last_license_error();
		}
		catch (const std::exception& ex)
		{
			auto inner = std::string ("{\"status\":{\"message\":\"") + ex.what() + "\",\"code\":-99}}";
			auto what = glasssix::memory::heap_alloc_elements<char>(inner.size() + 1);

			return (inner.copy(what, inner.size()), what);
		}
#endif

		glasssix::exposing::param_string _config_file_path(config_file_path);
		try
		{
			parser_object.init_plugin(_config_file_path);
			value["status"]["code"] = 0;
			value["status"]["message"] = "OK";
		}
		catch (const glasssix::exposing::abi_error& ex)
		{
			value["status"]["code"] = Json::Int(ex.result());
			value["status"]["message"] = ex.what_to_narrow();
		}
		catch (const glasssix::exposing::nessus::parser_exception& ex)
		{
			value["status"]["code"] = Json::Int(ex.what_code());
			value["status"]["message"] = ex.what();
		}
		catch (const Json::Exception& ex)
		{
			value["status"]["code"] = Json::Int(glasssix::exposing::nessus::parser_exception::parser_exception_code::JSON_EXCEPTION);
			value["status"]["message"] = ex.what();
		}
		catch (const std::exception& ex)
		{
			value["status"]["code"] = Json::Int(glasssix::exposing::nessus::parser_exception::parser_exception_code::UNKNOWN_EXCEPTION);
			value["status"]["message"] = ex.what();
		}

		std::string result_str = writer.write(value);
		std::size_t size = result_str.size() + 1;
		auto result = glasssix::memory::heap_alloc_elements<char>(size);
		std::memcpy(result, result_str.data(), size * sizeof(char));

		return result;
	}


	PARSER_C_EXPORT char* parser_create_instance(const char* qualified_name, const char* str_param)
	{
		Json::FastWriter writer;
		Json::Value value;

		try
		{
			glasssix::exposing::guid instance_id = parser_object.create_instance(qualified_name, str_param);
			value["status"]["code"] = 0;
			value["status"]["message"] = "OK";
			value["instance_id"] = glasssix::exposing::to_string(instance_id);
		}
		catch (const glasssix::exposing::abi_error& ex)
		{
			value["status"]["code"] = Json::Int(ex.result());
			value["status"]["message"] = ex.what_to_narrow();
		}
		catch (const glasssix::exposing::nessus::parser_exception& ex)
		{
			value["status"]["code"] = Json::Int(ex.what_code());
			value["status"]["message"] = ex.what();
		}
		catch (const Json::Exception& ex)
		{
			value["status"]["code"] = Json::Int(glasssix::exposing::nessus::parser_exception::parser_exception_code::JSON_EXCEPTION);
			value["status"]["message"] = ex.what();
		}
		catch (const std::exception& ex)
		{
			value["status"]["code"] = Json::Int(glasssix::exposing::nessus::parser_exception::parser_exception_code::UNKNOWN_EXCEPTION);
			value["status"]["message"] = ex.what();
		}

		std::string result_str = writer.write(value);
		std::size_t size = result_str.size() + 1;
		auto result = glasssix::memory::heap_alloc_elements<char>(size);
		std::memcpy(result, result_str.data(), size * sizeof(char));

		return result;
	}


	PARSER_C_EXPORT char* parser_execute(const char* instance_id, const char* str_param,
		const char* input_data, const int input_data_len, char* output_data, const int output_data_len)
	{
		Json::FastWriter writer;
		Json::Value value;
		try
		{
			glasssix::exposing::param_string _str_param(str_param ? str_param : u8"");
			glasssix::exposing::param_span<std::uint8_t> input_data_span(reinterpret_cast<std::uint8_t*>(const_cast<char*>(input_data)), static_cast<size_t>(input_data_len));
			glasssix::exposing::param_span<std::uint8_t> output_data_span(reinterpret_cast<std::uint8_t*>(output_data), static_cast<size_t>(output_data_len));

			glasssix::exposing::param_string result_str = parser_object.execute(glasssix::exposing::guid(std::string(instance_id)), _str_param, input_data_span, output_data_span);
			value["status"]["code"] = 0;
			value["status"]["message"] = "OK";
			value["result"] = glasssix::exposing::to_narrow_string(result_str);
		}
		catch (const glasssix::exposing::abi_error& ex)
		{
			value["status"]["code"] = Json::Int(ex.result());
			value["status"]["message"] = ex.what_to_narrow();
		}
		catch (const glasssix::exposing::nessus::parser_exception& ex)
		{
			value["status"]["code"] = Json::Int(ex.what_code());
			value["status"]["message"] = ex.what();
		}
		catch (const Json::Exception& ex)
		{
			value["status"]["code"] = Json::Int(glasssix::exposing::nessus::parser_exception::parser_exception_code::JSON_EXCEPTION);
			value["status"]["message"] = ex.what();
		}
		catch (const std::exception& ex)
		{
			value["status"]["code"] = Json::Int(glasssix::exposing::nessus::parser_exception::parser_exception_code::UNKNOWN_EXCEPTION);
			value["status"]["message"] = ex.what();
		}

		std::string status_str = writer.write(value);
		std::size_t size = status_str.size() + 1;
		auto status = glasssix::memory::heap_alloc_elements<char>(size);
		std::memcpy(status, status_str.data(), size * sizeof(char));

		return status;
	}

	PARSER_C_EXPORT void parser_release_instance(const char* instance_id)
	{
		parser_object.release_instance(glasssix::exposing::guid(std::string(instance_id)));
	}

	PARSER_C_EXPORT void parser_free(void* ptr)
	{
		glasssix::memory::heap_free(ptr);
	}

#ifdef __cplusplus
}
#endif

//PARSER_C_EXPORT parser_handle 