#include "time_utils.hpp"

namespace glasssix
{
	std::time_t get_local_timestamp()
	{
		return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	}
}
