#include "license_config.hpp"
#include "base64.hpp"

#include <fmt/format.h>

namespace glasssix::license
{
	void to_json(nlohmann::json& json, const license_config& value)
	{
		json =
		{
			{ "server", value.server },
			{ "port", value.port },
			{ "license_id", value.license_id }
		};
	}

	void from_json(const nlohmann::json& json, license_config& value)
	{
		json["server"].get_to(value.server);
		json["port"].get_to(value.port);
		json["license_id"].get_to(value.license_id);
	}

	license_config::operator bool() const noexcept
	{
		return !server.empty() && !license_id.empty() && port != 0;
	}

	std::string license_config::to_license_key() const
	{
		auto buffer = nlohmann::json::to_msgpack(*this);

		return crypto::base64_encode(buffer);
	}

	std::string license_config::to_websocket_uri()
	{
		return fmt::format(FMT_STRING("ws://{}:{}"), server, port);
	}

	std::optional<license_config> license_config::from_license_key(std::string_view license_key)
	{
		auto decoded_data = crypto::base64_decode(license_key);

		if (decoded_data.empty())
		{

			return std::nullopt;
		}

		auto json = nlohmann::json::from_msgpack(decoded_data, true, false);

		return json.is_discarded() ? std::nullopt : std::optional{ json.get<license_config>() };
	}
}
