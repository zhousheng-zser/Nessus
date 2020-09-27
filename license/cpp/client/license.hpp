#pragma once

#ifdef _MSC_VER
#ifdef EXPORT_NESSUS_LICENSE
#undef 
#define EXPORT_NESSUS_LICENSE __declspec(dllexport)
#else
#define EXPORT_NESSUS_LICENSE __declspec(dllimport)
#endif
#elif defined(__linux__)
#define EXPORT_NESSUS_LICENSE
#endif

namespace glasssix::license
{
	extern "C" EXPORT_NESSUS_LICENSE void check_license_async(void(*callback)(bool, const char*));
	extern "C" EXPORT_NESSUS_LICENSE void request_license_async(void(*callback)(bool, const char*));
}
