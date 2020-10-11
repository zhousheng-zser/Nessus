#pragma once

#include <ctime>
#include <vector>
#include <chrono>
#include <cstdint>
#include <optional>

#include <nlohmann/json.hpp>
#include <abi/param_span.hpp>

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

		std::vector<std::uint8_t> to_buffer() const;
		std::chrono::seconds remaining_seconds() const;
		bool valid(std::time_t timestamp, exposing::param_span<const std::uint8_t> machine_id) const;
		static std::optional<license_info> from_buffer(exposing::param_span<const std::uint8_t> buffer);
	};

	void to_json(nlohmann::json& json, const license_info& value);
	void from_json(const nlohmann::json& json, license_info& value);
}
