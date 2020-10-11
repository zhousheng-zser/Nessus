#pragma once

#ifdef _MSC_VER
#ifdef EXPORT_NESSUS_LICENSE
#undef EXPORT_NESSUS_LICENSE
#define EXPORT_NESSUS_LICENSE __declspec(dllexport)
#else
#define EXPORT_NESSUS_LICENSE __declspec(dllimport)
#endif
#elif defined(__linux__)
#define EXPORT_NESSUS_LICENSE
#endif

#include <cstdint>

namespace glasssix::license
{
	using request_license_async_callback_type = void(*)(void* context, bool success, const char* message);
	using evaluate_license_callback_type = void(*)(void* context, bool valid, const char* message, std::int64_t remaining_seconds);

	extern "C" EXPORT_NESSUS_LICENSE void init_license_system(const char* license_key);
	extern "C" EXPORT_NESSUS_LICENSE void request_license_async(const char* license_key, void* context = nullptr);
	extern "C" EXPORT_NESSUS_LICENSE void set_request_license_async_callback(request_license_async_callback_type callback, void* context = nullptr);
	extern "C" EXPORT_NESSUS_LICENSE void evaluate_license(const char* license_key, evaluate_license_callback_type callback, void* context = nullptr);
}
