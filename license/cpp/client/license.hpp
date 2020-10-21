#pragma once

#ifdef _WIN32
#ifdef EXPORT_NESSUS_LICENSE
#undef EXPORT_NESSUS_LICENSE
#define EXPORT_NESSUS_LICENSE __declspec(dllexport)
#else
#define EXPORT_NESSUS_LICENSE __declspec(dllimport)
#endif
#else
#undef EXPORT_NESSUS_LICENSE
#define EXPORT_NESSUS_LICENSE __attribute__((visibility("default")))
#endif

#include <cstdint>

using request_license_async_callback_type = void(*)(void* context, bool success, const char* message);
using license_deadline_callback_type = void(*)(void* context, const char* message, std::int64_t remaining_seconds);
using evaluate_license_callback_type = void(*)(void* context, bool success, const char* message, std::int64_t remaining_seconds);

/// <summary>
/// Initializes the license system.
/// </summary>
/// <param name="license_key">The license key provided by the service provider</param>
extern "C" EXPORT_NESSUS_LICENSE void init_license_system(const char* license_key);

/// <summary>
/// Evaluates whether a license is valid.
/// </summary>
/// <param name="callback">The callback with a result</param>
/// <param name="context">The user-defined context</param>
extern "C" EXPORT_NESSUS_LICENSE void evaluate_license(evaluate_license_callback_type callback, void* context = nullptr);

/// <summary>
/// Requests the license server asynchronously for a new license.
/// </summary>
/// <param name="callback">The callback with a result</param>
/// <param name="context">The user-defined context</param>
extern "C" EXPORT_NESSUS_LICENSE void request_license_async(request_license_async_callback_type callback, void* context = nullptr);

/// <summary>
/// Sets a license deadline callback.
/// </summary>
/// <param name="callback"></param>
/// <param name="context"></param>
/// <returns></returns>
/// <remarks>The deadline callback triggers every five minutes when the remaining time is less than five days</remarks>
extern "C" EXPORT_NESSUS_LICENSE void set_license_deadline_callback(license_deadline_callback_type callback, void* context = nullptr);
