#include "time_utils.hpp"

#include <chrono>

#include <sys/stat.h>

#ifdef _WIN32
#include <sys/utime.h>
#else 
#include <utime.h>
#endif

namespace glasssix
{
	std::time_t get_local_timestamp()
	{
		return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	}

	std::optional<std::time_t> get_file_last_write_timestamp(std::string_view path)
	{
#ifdef _WIN32
		struct _stat64 info;
		
		return _stat64(path.data(), &info) == 0 ? std::optional{ info.st_mtime } : std::nullopt;
#else
		struct stat64 info;

		return stat64(path.data(), &info) == 0 ? std::optional{ info.st_mtime } : std::nullopt;
#endif
	}

	bool set_file_last_write_timestamp(std::string_view path, std::time_t timestamp)
	{
		utimbuf info = { timestamp, timestamp };

		return utime(path.data(), &info) == 0;
	}
}
