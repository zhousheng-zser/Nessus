#pragma once

#include <string>
#include <cstdint>
#include <optional>
#include <string_view>

#include <nlohmann/json.hpp>

namespace glasssix::license
{
	/// <summary>
	/// A client config.
	/// </summary>
	struct license_config
	{
		std::string server;
		std::uint16_t port;
		std::string product_id;

		explicit operator bool() const noexcept;
		std::string to_license_key() const;
		std::string to_websocket_uri();
		static std::optional<license_config> from_license_key(std::string_view license_key);
	};

	void to_json(nlohmann::json& json, const license_config& value);
	void from_json(const nlohmann::json& json, license_config& value);
}
