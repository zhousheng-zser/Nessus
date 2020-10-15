#pragma once

#ifdef _MSC_VER
#ifdef EXPORT_NESSUS_LICENSE
#undef EXPORT_NESSUS_LICENSE
#define EXPORT_NESSUS_LICENSE __declspec(dllexport)
#else
#define EXPORT_NESSUS_LICENSE __declspec(dllimport)
#endif
#else
#undef EXPORT_NESSUS_LICENSE
#define EXPORT_NESSUS_LICENSE
#endif

#include <cstdint>

namespace glasssix::license
{
	using request_license_async_callback_type = void(*)(void* context, bool success, const char* message);
	using license_deadline_callback_type = void(*)(void* context, const char* message, std::int64_t remaining_seconds);
	using evaluate_license_callback_type = void(*)(void* context, bool success, const char* message, std::int64_t remaining_seconds);

	extern "C" EXPORT_NESSUS_LICENSE void evaluate_license(evaluate_license_callback_type callback, void* context = nullptr);
	extern "C" EXPORT_NESSUS_LICENSE void request_license_async(request_license_async_callback_type callback, void* context = nullptr);
	extern "C" EXPORT_NESSUS_LICENSE void init_license_system(const char* license_key, license_deadline_callback_type callback = nullptr, void* context = nullptr);
}
