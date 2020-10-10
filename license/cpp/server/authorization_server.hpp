#pragma once

#include "protocol_messages.hpp"

#include <memory>
#include <cstdint>
#include <functional>
#include <string_view>

namespace glasssix::license
{
	/// <summary>
	/// A SSL socket server for authorization.
	/// </summary>
	class authorization_server
	{
	public:
		class impl;

		authorization_server();
		virtual ~authorization_server();
		void listen(std::uint16_t port) const;
		void close() const;
		void on_request_authorization(const std::function<authorization_response_message(const authorization_request_message&)>& handler) const;
	private:
		std::unique_ptr<impl> impl_;
	};
}
