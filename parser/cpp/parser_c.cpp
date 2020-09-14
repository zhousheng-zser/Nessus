#include "parser.hpp"

#include <memory.hpp>

#define PARSER_C_EXPORT __declspec(dllexport)
#define parser_handle void*

glasssix::exposing::nessus::parser parser_singleton = []
{
	glasssix::exposing::get_component_loader().add_module_by_name("parser");
	return glasssix::exposing::make_exported_interface<glasssix::exposing::nessus::parser>();
}();

#ifdef __cplusplus
extern "C" {
#endif

	char* parser_init_plugin(char* config_file_path, int len)
	{
		glasssix::exposing::param_string config_file_path_(config_file_path, len);
		auto status_ = parser_singleton.init_plugin(config_file_path_);
		std::size_t size = status_.size() + 1;
		auto status = glasssix::memory::heap_alloc_elements<char>(size);
		std::memcpy(status, status_.data(), size * sizeof(char));
		return status;
	}

	char* parser_parse(char* protocol, int protocol_len, char* jstr, int jstr_len)
	{
		glasssix::exposing::param_string protocol_(protocol, protocol_len);
		glasssix::exposing::param_string jstr_(jstr, jstr_len);

		glasssix::exposing::param_string result_ = parser_singleton.parse(protocol_, jstr_);
		std::size_t size = result_.size() + 1;
		auto result = glasssix::memory::heap_alloc_elements<char>(size);
		std::memcpy(result, result_.data(), size * sizeof(char));
		return result;
	}

	void parser_free(void* ptr)
	{
		glasssix::memory::heap_free(ptr);
	}

#ifdef __cplusplus
}
#endif

//PARSER_C_EXPORT parser_handle 