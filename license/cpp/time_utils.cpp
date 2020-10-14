#include "time_utils.hpp"

#ifdef _WIN32
#define gmtime_safe(a, b) gmtime_s(a, b)
#else
#define gmtime_safe(a, b) gmtime_r(b, a)
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
		return time_point{ duration{ get_timestamp() * 10000000LL } };
	}

	std::time_t utc_unix_timestamp_clock::to_time_t(const utc_unix_timestamp_clock::time_point& time) noexcept
	{
		return time.time_since_epoch().count() / 10000000LL;
	}

	utc_unix_timestamp_clock::time_point utc_unix_timestamp_clock::from_time_t(std::time_t timestamp) noexcept
	{
		return time_point{ duration{ timestamp * 10000000LL} };
	}
}
