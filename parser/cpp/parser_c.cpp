#include "parser_c.hpp"
#include "parser.hpp"

#include <memory.hpp>

#ifndef G6_DISABLE_LICENSE
#include <vulcanus/license.hpp>
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

	PARSER_C_EXPORT char* parser_init_plugin(void* instance, const char* config_file_path, const char* license_key)
	{
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

		glasssix::exposing::nessus::parser parser_object{ glasssix::exposing::take_over_abi_from_void_ptr(reinterpret_cast<void*>(instance)) };

		glasssix::exposing::param_string config_file_path_(config_file_path);
		auto status_ = parser_object.init_plugin(config_file_path_);
		std::size_t size = status_.size() + 1;
		auto status = glasssix::memory::heap_alloc_elements<char>(size);
		std::memcpy(status, status_.data(), size * sizeof(char));

		glasssix::exposing::detach_abi(parser_object);

		return status;
	}

	PARSER_C_EXPORT char* parser_parse(void* instance, const char* topic, const char* jstr_param, char* data, size_t data_len, void* external, size_t external_size)
	{
		glasssix::exposing::nessus::parser parser_object{ glasssix::exposing::take_over_abi_from_void_ptr(reinterpret_cast<void*>(instance)) };

		glasssix::exposing::param_string topic_(topic);
		glasssix::exposing::param_string str_param_(jstr_param ? jstr_param : u8"");
		glasssix::exposing::param_span<std::uint8_t> data_(reinterpret_cast<std::uint8_t*>(data), data_len);
		glasssix::exposing::param_span<std::uint8_t> external_(reinterpret_cast<std::uint8_t*>(external), external_size);

		glasssix::exposing::param_string result_ = parser_object.parse(topic_, str_param_, data_, external_);
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