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

#include <ctime>

namespace glasssix::license
{
	using evaluate_license_callback_type = void(*)(void* context, bool valid, const char* message, std::time_t remaining_time);
	using request_license_async_callback_type = void(*)(void* context, bool success, const char* message, std::time_t remaining_time);

	extern "C" EXPORT_NESSUS_LICENSE void evaluate_license(evaluate_license_callback_type callback);
	extern "C" EXPORT_NESSUS_LICENSE void request_license_async(request_license_async_callback_type callback);
}
