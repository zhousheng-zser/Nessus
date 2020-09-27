#pragma once

#include <ctime>
#include <vector>
#include <cstdint>

#include <nlohmann/json.hpp>

namespace glasssix::license
{
	/// <summary>
	/// License information.
	/// </summary>
	struct license_info
	{
		std::vector<std::uint8_t> machine_id;
		std::time_t expiration_time;
		std::time_t last_running_time;
		std::time_t authorization_time;
	};

	void to_json(nlohmann::json& json, const license_info& value);
	void from_json(const nlohmann::json& json, license_info& value);
}
