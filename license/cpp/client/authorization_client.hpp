#pragma once

#include "protocol_messages.hpp"

#include <memory>
#include <exception>
#include <functional>
#include <string_view>

namespace glasssix::license
{
	/// <summary>
	/// A SSL socket client for authorization.
	/// </summary>
	class authorization_client
	{
	public:
		class impl;

		authorization_client();
		virtual ~authorization_client();
		void connect(std::string_view server, std::string_view port) const;
		void close() const;
		void request_authorization(const authorization_request_message& message) const;
		void on_connect(const std::function<void()>& handler) const;
		void on_async_error(const std::function<void(const std::error_code&)>& handler) const;
		void on_authorization(const std::function<void(const authorization_response_message&)>& handler) const;
	private:
		std::unique_ptr<impl> impl_;
	};
}
