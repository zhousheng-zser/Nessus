#pragma once

#include "protocol_header.hpp"

#include <memory>
#include <cstdint>
#include <functional>
#include <initializer_list>

#include <nlohmann/json.hpp>
#include <abi/param_span.hpp>

namespace glasssix::license
{
	/// <summary>
	/// A helper class to parse a payload and dispatch the pending requests.
	/// </summary>
	class protocol_dispatcher
	{
	public:
		class impl;
		using dispatcher_type = std::function<void(const std::shared_ptr<void>&, const nlohmann::json&)>;

		protocol_dispatcher(std::initializer_list<std::pair<const message_type, dispatcher_type>> dispatcher_table);
		virtual ~protocol_dispatcher();
		void parse(exposing::param_span<const std::uint8_t> payload, const std::shared_ptr<void>& context, const std::function<void(std::string_view)>& invalid_handler) const;
	private:
		std::unique_ptr<impl> impl_;
	};
}
