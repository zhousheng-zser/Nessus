#include "protocol_messages.hpp"

namespace glasssix::license
{
	void to_json(nlohmann::json& json, const authorization_request_message& value)
	{
		json =
		{
			{ "product_id", value.product_id },
			{ "machine_id", value.machine_id },
			{ "client_timestamp", value.client_timestamp }
		};
	}

	void to_json(nlohmann::json& json, const authorization_response_message& value)
	{
		json =
		{
			{ "status" , value.status },
			{ "license", value.license },
			{ "user_portrait", value.user_portrait },
			{ "server_timestamp", value.server_timestamp }
		};
	}

	void from_json(const nlohmann::json& json, authorization_request_message& value)
	{
		json["product_id"].get_to(value.product_id);
		json["machine_id"].get_to(value.machine_id);
		json["client_timestamp"].get_to(value.client_timestamp);
	}

	void from_json(const nlohmann::json& json, authorization_response_message& value)
	{
		json["status"].get_to(value.status);
		json["user_portrait"].get_to(value.user_portrait);
		json["license"].get_to(value.license);
		json["server_timestamp"].get_to(value.server_timestamp);
	}
}
