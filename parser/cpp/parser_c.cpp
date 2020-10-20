#include "parser.hpp"
#include "license.hpp"

#include <memory.hpp>

#ifdef _WIN32
#define PARSER_C_EXPORT __declspec(dllexport)
#else
#define PARSER_C_EXPORT 
#endif

bool parser_module_ready = []
{
	return glasssix::exposing::get_component_loader().add_module_by_name("parser");
}();

#ifdef __cplusplus
extern "C" {
#endif
	PARSER_C_EXPORT void* parser_new_instance()
	{
		auto parser_object = glasssix::exposing::make_exported_interface<glasssix::exposing::nessus::parser>();
		return glasssix::exposing::detach_abi(parser_object);
	}

	PARSER_C_EXPORT void parser_release_instance(void *instance)
	{
		glasssix::exposing::unknown_object parser_objcet{ reinterpret_cast<void*>(instance) };
		instance = nullptr;
	}

	PARSER_C_EXPORT char* parser_init_plugin(void* instance, char* config_file_path, int len, char* license_key)
	{
		glasssix::license::init_license_system(license_key);

		glasssix::exposing::nessus::parser parser_object{ glasssix::exposing::take_over_abi_from_void_ptr(reinterpret_cast<void*>(instance)) };

		glasssix::exposing::param_string config_file_path_(config_file_path, len);
		auto status_ = parser_object.init_plugin(config_file_path_);
		std::size_t size = status_.size() + 1;
		auto status = glasssix::memory::heap_alloc_elements<char>(size);
		std::memcpy(status, status_.data(), size * sizeof(char));

		glasssix::exposing::detach_abi(parser_object);

		return status;
	}

	PARSER_C_EXPORT char* parser_parse(void* instance, char* protocol, int protocol_len, char* jstr, int jstr_len, char* data, int data_len)
	{
		glasssix::exposing::nessus::parser parser_object{ glasssix::exposing::take_over_abi_from_void_ptr(reinterpret_cast<void*>(instance)) };

		glasssix::exposing::param_string protocol_(protocol, protocol_len);
		glasssix::exposing::param_string jstr_(jstr, jstr_len);
		glasssix::exposing::param_span<std::uint8_t> data_(reinterpret_cast<std::uint8_t*>(data), data_len);

		glasssix::exposing::param_string result_ = parser_object.parse(protocol_, jstr_, data_);
		std::size_t size = result_.size() + 1;
		auto result = glasssix::memory::heap_alloc_elements<char>(size);
		std::memcpy(result, result_.data(), size * sizeof(char));

		glasssix::exposing::detach_abi(parser_object);

		return result;
	}

	PARSER_C_EXPORT void parser_free(void* ptr)
	{
		glasssix::memory::heap_free(ptr);
	}

#ifdef __cplusplus
}
#endif

//PARSER_C_EXPORT parser_handle 