#include "license_info.hpp"
#include "time_utils.hpp"

#include <utility>
#include <algorithm>

namespace glasssix::license
{
	void to_json(nlohmann::json& json, const license_info& value)
	{
		json =
		{
			{ "machine_id", value.machine_id },
			{ "expiration_time", value.expiration_time },
			{ "last_running_time", value.last_running_time },
			{ "authorization_time", value.authorization_time }
		};
	}

	void from_json(const nlohmann::json& json, license_info& value)
	{
		json["machine_id"].get_to(value.machine_id);
		json["expiration_time"].get_to(value.expiration_time);
		json["last_running_time"].get_to(value.last_running_time);
		json["authorization_time"].get_to(value.authorization_time);
	}

	std::vector<std::uint8_t> license_info::to_buffer() const
	{
		return nlohmann::json::to_msgpack(*this);
	}

	std::chrono::seconds license_info::remaining_seconds() const
	{
		auto remaining = std::chrono::system_clock::from_time_t(expiration_time) - std::chrono::system_clock::now();
		auto final_remaining = std::max(remaining, std::chrono::system_clock::duration::zero());

		return std::chrono::duration_cast<std::chrono::seconds>(final_remaining);
	}

	bool license_info::valid(std::time_t timestamp, exposing::param_span<const std::uint8_t> machine_id) const
	{
		auto now_timestamp = get_timestamp();

		return this->machine_id.size() == machine_id.size() &&
			std::equal(this->machine_id.begin(), this->machine_id.end(), machine_id.begin()) &&
			last_running_time < expiration_time&&
			authorization_time < expiration_time&&
			last_running_time >= authorization_time &&
			last_running_time <= now_timestamp &&
			authorization_time <= now_timestamp &&
			expiration_time > now_timestamp &&
			timestamp < expiration_time;

	}

	std::optional<license_info> license_info::from_buffer(exposing::param_span<const std::uint8_t> buffer)
	{
		auto json = nlohmann::json::from_msgpack(buffer, true, false);

		return json.is_discarded() ? std::nullopt : std::optional{ json.get<license_info>() };
	}
}
