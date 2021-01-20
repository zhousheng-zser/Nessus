#include "authorization_server.hpp"
#include "glaucus.hpp"
#include "time_utils.hpp"
#include "number_utils.hpp"
#include "license_info.hpp"
#include "license_database.hpp"

#include <cmath>
#include <chrono>
#include <cstdint>
#include <iostream>

#include <logger.hpp>
#include <abi/guid.hpp>
#include <fmt/format.h>
#include <nlohmann/json.hpp>
#include <websocketpp/error.hpp>

using namespace glasssix::license;
namespace crypto = glasssix::crypto;
namespace exposing = glasssix::exposing;

namespace
{
	constexpr std::uint16_t listen_port = 16623;
	constexpr std::time_t permissible_error = 600;

	authorization_response_message create_response_message(const authorization_request_message& message, const license_record& record)
	{
		thread_local crypto::glaucus glaucus;
		auto timestamp = glasssix::get_local_timestamp();
		auto license = nlohmann::json::to_msgpack(license_info{ message.machine_id, record.expiration_time, timestamp, timestamp });

		glaucus.generate(message.machine_id, timestamp);

		return authorization_response_message{ "OK", glaucus.forward(license), glaucus.user_portrait(), timestamp };
	}
}

int main()
{
	try
	{
		authorization_server server;

		server.on_request_authorization([](const authorization_request_message& message)
			{
				thread_local license_database database{ "host=127.0.0.1 port=5432 user=postgres password=Glasssix+1S dbname=postgres connect_timeout=10" };

				LOG_ND(INFO) << fmt::format(FMT_STRING("Client [License ID: {}][Machine ID: {}] is requesting a new license."), message.product_id, glasssix::buffer_to_hex_string(message.machine_id));

				// Checks the client timestamp.
				if (std::abs(message.client_timestamp - glasssix::get_local_timestamp()) >= permissible_error)
				{
					return authorization_response_message{ "The gap between the client time and the server time is too large." };
				}

				auto license = database.get_license(exposing::guid{ message.product_id });

				if (!license)
				{
					return authorization_response_message{ "The license does not exist." };
				}

				if (license->expiration_time <= glasssix::get_local_timestamp())
				{
					return authorization_response_message{ "The license has already expired." };
				}

				if (license->authorized_device_count >= license->allowed_device_count)
				{
					return authorization_response_message{ "The count of the authorized devices has exceeded the limit of the license." };
				}
				
				LOG_ND(INFO) << fmt::format(FMT_STRING("License information [ID: {}][Organization: {}]"), exposing::to_string(license->id), license->organization);

				// Increases the authorized device count if it is a new deivce.
				if (database.get_exact_authorized_devices(license->id, message.machine_id).empty())
				{
					license->authorized_device_count++;
					database.add_or_update_license(*license);
				}
				
				database.add_or_update_authorized_device(authorized_device_record{ license->id, message.machine_id, glasssix::get_local_timestamp() });

				return create_response_message(message, *license);
			});

		server.listen(listen_port);

		for (std::string buffer;;)
		{
			std::cin >> buffer;

			if (buffer == "q" || buffer == "Q")
			{
				break;
			}
		}

		server.close();
	}
	catch (const std::exception& ex)
	{
		LOG_ND(ERROR) << ex.what();
	}
}
