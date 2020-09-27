#include "license_info.hpp"

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
}
