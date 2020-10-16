#pragma once

#include <ctime>
#include <string>
#include <cstdint>

#include <abi/guid.hpp>

namespace glasssix::license
{
	/// <summary>
	/// A license record in the database.
	/// </summary>
	struct license_record
	{
		exposing::guid id;
		std::string organization;
		std::int32_t allowed_device_count;
		std::int32_t authorized_device_count;
		std::time_t creation_time;
		std::time_t expiration_time;
	};
}
