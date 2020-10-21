#pragma once

#ifdef _WIN32
#ifdef PARSER_C_EXPORT
#undef PARSER_C_EXPORT
#define PARSER_C_EXPORT __declspec(dllexport)
#else
#define PARSER_C_EXPORT __declspec(dllimport)
#endif
#else
#undef PARSER_C_EXPORT
#define PARSER_C_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

	PARSER_C_EXPORT void* parser_new_instance();
	PARSER_C_EXPORT void parser_release_instance(void* instance);
	PARSER_C_EXPORT char* parser_init_plugin(void* instance, char* config_file_path, int len, char* license_key);
	PARSER_C_EXPORT char* parser_parse(void* instance, char* protocol, int protocol_len, char* jstr, int jstr_len, char* data, int data_len);
	PARSER_C_EXPORT void parser_free(void* ptr);

#ifdef __cplusplus
}
#endif
