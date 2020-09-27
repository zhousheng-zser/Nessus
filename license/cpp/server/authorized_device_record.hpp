#pragma once

#include <ctime>
#include <string>
#include <vector>
#include <cstdint>

#include <abi/guid.hpp>

namespace glasssix::license
{
	/// <summary>
	/// An authorized device record in the database.
	/// </summary>
	struct authorized_device_record
	{
		exposing::guid license_id;
		std::vector<std::uint8_t> machine_id;
		std::time_t last_authorization_time;
	};
}
