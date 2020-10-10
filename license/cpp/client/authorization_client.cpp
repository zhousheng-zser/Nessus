#include "authorization_client.hpp"
#include "protocol_header.hpp"

#include <mutex>
#include <atomic>
#include <thread>
#include <cstdint>
#include <algorithm>
#include <condition_variable>

#define NOGDI

#include <asio.hpp>
#include <logger.hpp>
#include <delegate.hpp>
#include <nlohmann/json.hpp>
#include <websocketpp/client.hpp>
#include <websocketpp/config/asio.hpp>

namespace glasssix::license
{
	using asio::ip::tcp;

	namespace
	{
		/// <summary>
		/// An IO context running on a worker thread.
		/// </summary>
		struct single_threaded_io_context
		{
		public:
			single_threaded_io_context() : work_guard_{ asio::make_work_guard(context_.get_executor()) }, worker_thread_{ [this] { context_.run(); } }
			{
			}

			~single_threaded_io_context()
			{
				context_.stop();
				work_guard_.reset();

				if (worker_thread_.joinable())
				{
					worker_thread_.join();
				}
			}

			operator asio::io_context& () noexcept
			{
				return context_;
			}

			operator const asio::io_context& () const noexcept
			{
				return context_;
			}

			asio::io_context& get() noexcept
			{
				return *this;
			}
		private:
			asio::io_context context_;
			asio::executor_work_guard<asio::io_context::executor_type> work_guard_;
			std::thread worker_thread_;
		};

		single_threaded_io_context internal_io_context;
	}

	class authorization_client::impl
	{
	public:
		using client_type = websocketpp::client<websocketpp::config::asio>;

		delegate<void> on_connect;
		delegate<void, const std::error_code&> on_async_error;
		delegate<void, const authorization_response_message&> on_authorization;

		impl()
		{
			client_.init_asio(&internal_io_context.get());
			client_.set_access_channels(websocketpp::log::alevel::all);
			client_.clear_access_channels(websocketpp::log::alevel::frame_payload);
			client_.set_message_handler(std::bind(&impl::on_message, this, std::placeholders::_1, std::placeholders::_2));
			client_.set_open_handler([this](const websocketpp::connection_hdl& handle)
				{
					{
						std::scoped_lock lock{ mutex_ };
						connection_ = handle;
					}

					on_connect();
				});

			client_.set_close_handler([this](const websocketpp::connection_hdl& handle)
				{
					std::scoped_lock lock{ mutex_ };

					connection_.reset();
					condition_close_.notify_one();
				});
		}

		~impl()
		{
			close();
		}

		void connect(std::string_view uri)
		{
			close();

			std::error_code code;
			auto connection = client_.get_connection(std::string(uri), code);

			if (code)
			{
				throw websocketpp::exception{ fmt::format(FMT_STRING("Failed to create a connection. {}"), code.message()) };
			}

			client_.connect(connection);
		}

		void close()
		{
			std::error_code code;
			std::unique_lock lock{ mutex_ };

			if (connection_.expired())
			{
				connection_.reset();
			}
			else
			{
				client_.close(connection_, websocketpp::close::status::going_away, "Shutdown", code);
			}

			condition_close_.wait(lock, [this] { return connection_.expired(); });
		}

		void request_authorization(const authorization_request_message& message)
		{
			auto connection = [this] { std::scoped_lock{ mutex_ }; return connection_; }();
			auto payload = create_message_buffer(message_type::authorization, message);

			client_.send(connection, payload.data(), payload.size(), websocketpp::frame::opcode::BINARY);
		}
	private:
		static void raise_authorization(impl& obj, const nlohmann::json& json)
		{
			if (auto response = json.get<authorization_response_message>())
			{
				obj.on_authorization(response);
			}
		}

		void parse_content(const protocol_header& header, std::string_view content)
		{
			static constexpr std::array dispatcher_table
			{
				std::pair{ message_type::authorization, &impl::raise_authorization }
			};

			// Parses the message buffer and omits this item if any error occurs.
			if (auto json = nlohmann::json::parse(std::string_view{ reinterpret_cast<const char*>(content.data()), content.size() }, nullptr, false); !json.is_discarded())
			{
				if (auto iter = std::find_if(dispatcher_table.begin(), dispatcher_table.end(), [&](const auto& inner) { return inner.first == header.type; }); iter != dispatcher_table.end())
				{
					iter->second(*this, json);
				}
			}
		}

		void parse_header(const client_type::connection_ptr& connection, std::string_view payload)
		{
			std::error_code code;

			// Validates the header size.
			if (payload.size() < protocol_header::header_size)
			{
				return connection->close(websocketpp::close::status::invalid_payload, "The payload size cannot be smaller than header size.", code);
			}

			protocol_header::buffer_type header_buffer;
			auto header = (std::copy(payload.begin(), payload.begin() + protocol_header::header_size, header_buffer.begin()), protocol_header::parse(header_buffer));

			// Validates the data size.
			if (!header || payload.size() - header_buffer.size() < header.size)
			{
				return connection->close(websocketpp::close::status::invalid_payload, "The data size is too small.", code);
			}

			parse_content(header, payload.substr(header_buffer.size(), header.size));
		}

		void on_message(const websocketpp::connection_hdl& handle, const client_type::message_ptr& message)
		{
			try
			{
				// Only processes bianry data.
				if (message->get_opcode() == websocketpp::frame::opcode::BINARY)
				{
					auto connection = client_.get_con_from_hdl(handle);
					auto payload = message->get_payload();

					parse_header(connection, payload);
				}
			}
			catch (const std::exception& ex)
			{
				LOG(ERROR) << ex.what();
			}
		}

		std::mutex mutex_;
		client_type client_;
		websocketpp::connection_hdl connection_;
		std::condition_variable condition_close_;
	};

	authorization_client::authorization_client() : impl_{ std::make_unique<impl>() }
	{
	}

	authorization_client::~authorization_client()
	{
	}

	void authorization_client::connect(std::string_view uri) const
	{
		impl_->connect(uri);
	}

	void authorization_client::close() const
	{
		impl_->close();
	}

	void authorization_client::request_authorization(const authorization_request_message& message) const
	{
		impl_->request_authorization(message);
	}

	void authorization_client::on_connect(const std::function<void()>& handler) const
	{
		impl_->on_connect += handler;
	}

	void authorization_client::on_async_error(const std::function<void(const std::error_code&)>& handler) const
	{
		impl_->on_async_error += handler;
	}

	void authorization_client::on_authorization(const std::function<void(const authorization_response_message&)>& handler) const
	{
		impl_->on_authorization += handler;
	}
}
