#pragma once

#include "protocol_header.hpp"

#include <ctime>
#include <limits>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <cstddef>
#include <utility>
#include <algorithm>

#include <nlohmann/json.hpp>

namespace glasssix::license
{
	/// <summary>
	/// The maximum size of a message.
	/// </summary>
	inline constexpr std::size_t max_message_size = std::numeric_limits<std::int16_t>::max();

	struct authorization_request_message
	{
		std::string product_id;
		std::vector<std::uint8_t> machine_id;
		std::time_t client_timestamp;
	};

	struct authorization_response_message
	{
		std::string status;
		std::vector<std::uint8_t> license;
		std::vector<std::uint8_t> user_portrait;
		std::time_t server_timestamp;

		explicit operator bool() const noexcept
		{
			return status == "OK";
		}
	};

	void to_json(nlohmann::json& json, const authorization_request_message& value);
	void to_json(nlohmann::json& json, const authorization_response_message& value);
	void from_json(const nlohmann::json& json, authorization_request_message& value);
	void from_json(const nlohmann::json& json, authorization_response_message& value);

	template<typename Message>
	std::vector<std::uint8_t> create_message_buffer(message_type type, Message&& message)
	{
		auto json_bytes = nlohmann::json::to_msgpack(nlohmann::json{ std::forward<Message>(message) });
		auto header_bytes = protocol_header{ type, static_cast<std::uint32_t>(json_bytes.size()) }.compute_hash_and_dump();
		std::size_t size = header_bytes.size() + json_bytes.size();

		std::vector<std::uint8_t> result(size);
		auto iter = std::copy(header_bytes.begin(), header_bytes.end(), result.begin());
		
		return (std::copy(json_bytes.begin(), json_bytes.end(), iter), result);
	}


}
