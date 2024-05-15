#pragma once

#ifdef _WIN32
#ifdef PARSER_C_EXPORT
#undef PARSER_C_EXPORT
#define PARSER_C_EXPORT __declspec(dllexport)
#else
#define PARSER_C_EXPORT __declspec(dllimport)
#endif
#elif defined(__GNUC__)
#ifdef PARSER_C_EXPORT
#undef PARSER_C_EXPORT
#define PARSER_C_EXPORT __attribute__((visibility("default")))
#else
#define PARSER_C_EXPORT 
#endif
#endif

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif

	/// <summary>
	/// Initializes all plugins.
	/// </summary>
	/// <param name="instance">The instance of a protocol parser</param>
	/// <param name="config_file_path">The config file path</param>
	/// <param name="len">The size of the config file path</param>
	/// <param name="license_key">The license key provided by the service provider</param>
	/// <returns>The result interpreted as JSON</returns>
	/// <remarks>The return value must be disposed by calling praser_free function.</remarks>
	PARSER_C_EXPORT char* parser_init_plugin(const char* config_file_path, const char* license_key);

	/// <summary>
	/// Dispatches a protocol.
	/// </summary>
	/// <param name="instance">The instance of a protocol parser</param>
	/// <param name="topic">The protocol name</param>
	/// <param name="jstr_param">The parameters interpreted as JSON</param>
	/// <param name="data">The optional binary data</param>
	/// <param name="data_len">The size of the optional binary data</param>
	/// <param name="external">The optional binary external</param>
	/// <param name="external_size">The size of the optional binary data</param>
	/// <returns>The result interpreted as JSON</returns>
	/// <remarks>The return value must be disposed by calling praser_free function.</remarks>
	PARSER_C_EXPORT char* parser_create_instance(const char* qualified_name, const char* str_param);


	PARSER_C_EXPORT char* parser_execute(const char* instance_id, const char* str_param,
		const char* input_data, const int input_data_len, char* output_data, const int output_data_len);

	PARSER_C_EXPORT void parser_release_instance(const char* instance_id);

	/// <summary>
	/// Disposes a buffer.
	/// </summary>
	/// <param name="ptr">The buffer</param>
	PARSER_C_EXPORT void parser_free(void* ptr);

#ifdef __cplusplus
}
#endif
