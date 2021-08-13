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
#endif
#endif

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#else
#include <stddef.h>
#endif

	/// <summary>
	/// Creates a new instance of a protocol praser.
	/// </summary>
	/// <returns>The instance</returns>
	/// <remarks>The instance must be freed by parser_release_instance function</remarks>
	PARSER_C_EXPORT void* parser_new_instance();

	/// <summary>
	/// Disposes a instance of a protocol parser.
	/// </summary>
	/// <param name="instance">The instance</param>
	PARSER_C_EXPORT void parser_release_instance(void* instance);

	/// <summary>
	/// Initializes all plugins.
	/// </summary>
	/// <param name="instance">The instance of a protocol parser</param>
	/// <param name="config_file_path">The config file path</param>
	/// <param name="len">The size of the config file path</param>
	/// <param name="license_key">The license key provided by the service provider</param>
	/// <returns>The result interpreted as JSON</returns>
	/// <remarks>The return value must be disposed by calling praser_free function.</remarks>
	PARSER_C_EXPORT char* parser_init_plugin(void* instance, const char* config_file_path, const char* license_key);

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
	PARSER_C_EXPORT char* parser_parse(void* instance, const char* topic, const char* jstr_param, char* data, size_t data_len, void* external = nullptr, size_t external_size = 0);

	/// <summary>
	/// Disposes a buffer.
	/// </summary>
	/// <param name="ptr">The buffer</param>
	PARSER_C_EXPORT void parser_free(void* ptr);

#ifdef __cplusplus
}
#endif
