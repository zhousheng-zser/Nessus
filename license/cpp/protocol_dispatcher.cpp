#include "protocol_dispatcher.hpp"

#include <vector>
#include <unordered_map>

namespace glasssix::license
{
	class protocol_dispatcher::impl
	{
	public:
		impl(std::initializer_list<std::pair<const message_type, dispatcher_type>> dispatcher_table) : dispatcher_table_ { dispatcher_table }
		{
		}

		~impl()
		{
		}

		void parse(exposing::param_span<const std::uint8_t> payload, const std::shared_ptr<void>& context, const std::function<void(std::string_view)>& invalid_handler) const
		{
			// Validates the header size.
			if (payload.size() < protocol_header::header_size)
			{
				return invalid_handler("The payload size cannot be smaller than header size.");
			}

			protocol_header::buffer_type header_buffer;
			auto header = (std::copy(payload.begin(), payload.begin() + protocol_header::header_size, header_buffer.begin()), protocol_header::parse(header_buffer));

			if (!header)
			{
				return invalid_handler("Invalid header.");
			}

			// Validates the data size.
			if (payload.size() - header_buffer.size() < header.size)
			{
				return invalid_handler("The data size is too small.");
			}

			parse_content(context, header, payload.sub_span(header_buffer.size(), header.size));
		}
	private:
		void parse_content(const std::shared_ptr<void>& context, const protocol_header& header, exposing::param_span<const std::uint8_t> content) const
		{
			// Parses the message buffer and omits this item if any error occurs.
			if (auto json = nlohmann::json::from_msgpack(content, true, false); !json.is_discarded())
			{
				if (auto iter = dispatcher_table_.find(header.type); iter != dispatcher_table_.end())
				{
					iter->second(context, json);
				}
			}
		}

		std::unordered_map<message_type, dispatcher_type> dispatcher_table_;
	};

	protocol_dispatcher::protocol_dispatcher(std::initializer_list<std::pair<const message_type, dispatcher_type>> dispatcher_table) : impl_{ std::make_unique<impl>(std::move(dispatcher_table)) }
	{
	}

	protocol_dispatcher::~protocol_dispatcher()
	{
	}

	void protocol_dispatcher::parse(exposing::param_span<const std::uint8_t> payload, const std::shared_ptr<void>& context, const std::function<void(std::string_view)>& invalid_handler) const
	{
		impl_->parse(payload, context, invalid_handler);
	}
}
