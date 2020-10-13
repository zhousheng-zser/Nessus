#include "time_utils.hpp"

#ifdef _WIN32
#define gmtime_safe gmtime_s
#else
#define gmtime_safe gmtime_r
#endif

namespace glasssix
{
	std::time_t get_timestamp()
	{
		std::tm time;
		auto local_timestamp = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

		return (gmtime_safe(&time, &local_timestamp), std::mktime(&time));
	}

	utc_unix_timestamp_clock::time_point utc_unix_timestamp_clock::now() noexcept
	{
		return time_point{ duration{ get_timestamp() * _XTIME_TICKS_PER_TIME_T } };
	}

	std::time_t utc_unix_timestamp_clock::to_time_t(const utc_unix_timestamp_clock::time_point& time) noexcept
	{
		return time.time_since_epoch().count() / _XTIME_TICKS_PER_TIME_T;
	}

	utc_unix_timestamp_clock::time_point utc_unix_timestamp_clock::from_time_t(std::time_t timestamp) noexcept
	{
		return time_point{ duration{ timestamp * _XTIME_TICKS_PER_TIME_T} };
	}
}
