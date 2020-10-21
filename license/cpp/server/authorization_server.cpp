#include "authorization_server.hpp"
#include "license_database.hpp"
#include "protocol_header.hpp"
#include "protocol_dispatcher.hpp"

#include <array>
#include <mutex>
#include <thread>
#include <vector>
#include <algorithm>
#include <functional>
#include <condition_variable>

#define NOGDI

#include <asio.hpp>
#include <logger.hpp>
#include <delegate.hpp>
#include <nlohmann/json.hpp>
#include <abi/param_span.hpp>
#include <websocketpp/server.hpp>
#include <websocketpp/config/asio.hpp>

namespace glasssix::license
{
	namespace
	{
		/// <summary>
		/// An IO context running on a thread pool.
		/// </summary>
		struct thread_pool_io_context
		{
		public:
			thread_pool_io_context() : work_guard_{ asio::make_work_guard(context_) }
			{
				for (std::size_t i = 0, size = std::thread::hardware_concurrency(); i < size; i++)
				{
					worker_threads_.emplace_back([this] { context_.run(); });
				}
			}

			~thread_pool_io_context()
			{
				work_guard_.reset();
				
				for (auto& item : worker_threads_)
				{
					if (item.joinable())
					{
						item.join();
					}
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
				return context_;
			}
		private:
			asio::io_context context_;
			std::vector<std::thread> worker_threads_;
			asio::executor_work_guard<asio::io_context::executor_type> work_guard_;
		};
	}
	
	class authorization_server::impl
	{
	public:
		using server_type = websocketpp::server<websocketpp::config::asio>;
		delegate<authorization_response_message, const authorization_request_message&> on_request_authorization;

		impl() :
			dispatcher_
			{
				{ message_type::authorization, std::bind(&impl::raise_request_authorization, this, std::placeholders::_1, std::placeholders::_2) }
			}
		{
			server_.init_asio(&io_context_.get());
			server_.set_access_channels(websocketpp::log::alevel::all);
			server_.clear_access_channels(websocketpp::log::alevel::frame_payload);
			server_.set_message_handler(std::bind(&impl::on_message, this, std::placeholders::_1, std::placeholders::_2));
			server_.set_open_handler([this](const websocketpp::connection_hdl& handle)
				{
					std::scoped_lock lock{ mutex_ };

					connections_.emplace_back(handle);
				});

			server_.set_close_handler([this](const websocketpp::connection_hdl& handle)
				{
					{
						std::scoped_lock lock{ mutex_ };

						if (auto iter = std::find_if(connections_.begin(), connections_.end(), [&](const websocketpp::connection_hdl& inner) { return inner.lock() == handle.lock(); }); iter != connections_.end())
						{
							connections_.erase(iter);
						}
					}

					condition_erase_.notify_all();
				});
		}

		~impl()
		{
			close();
		}

		void listen(std::uint16_t port)
		{
			close();
			server_.listen(port);
			server_.start_accept();
		}

		void close()
		{
			std::error_code code;
			
			if (server_.is_listening())
			{
				server_.stop_listening(code);
			}

			{
				std::unique_lock lock{ mutex_ };

				// Closes all active connections.
				for (const auto& item : connections_)
				{
					if (auto connection = server_.get_con_from_hdl(item, code))
					{
						connection->close(websocketpp::close::status::going_away, "Shutdown");
					}
				}

				// Waits for all connections to be closed.
				condition_erase_.wait(lock, [this] { return connections_.empty(); });
			}
		}
	private:
		void raise_request_authorization(const std::shared_ptr<void>& context, const nlohmann::json& json)
		{
			auto request = json.get<authorization_request_message>();
			auto response = on_request_authorization(request);
			auto buffer = create_message_buffer(message_type::authorization, response);
			auto connection = std::static_pointer_cast<server_type::connection_ptr::element_type>(context);

			// Sends the response message to the client.
			connection->send(buffer.data(), buffer.size());
		}

		void on_message(const websocketpp::connection_hdl& handle, const server_type::message_ptr& message)
		{
			try
			{
				// Only processes bianry data.
				if (message->get_opcode() == websocketpp::frame::opcode::BINARY)
				{
					auto connection = server_.get_con_from_hdl(handle);
					auto payload = message->get_payload();

					dispatcher_.parse(exposing::param_span<const std::uint8_t>{ reinterpret_cast<const std::uint8_t*>(payload.c_str()), payload.size() }, connection, [&](std::string_view message)
						{
							connection->close(websocketpp::close::status::invalid_payload, std::string{ message });
						});
				}
			}
			catch (const std::exception& ex)
			{
				LOG(ERROR) << ex.what();
			}
		}

		std::mutex mutex_;
		server_type server_;
		protocol_dispatcher dispatcher_;
		thread_pool_io_context io_context_;
		std::condition_variable condition_erase_;
		std::vector<websocketpp::connection_hdl> connections_;
	};

	authorization_server::authorization_server() : impl_{ std::make_unique<impl>() }
	{
	}

	authorization_server::~authorization_server()
	{
	}

	void authorization_server::listen(std::uint16_t port) const
	{
		impl_->listen(port);
	}

	void authorization_server::close() const
	{
		impl_->close();
	}

	void authorization_server::on_request_authorization(const std::function<authorization_response_message(const authorization_request_message&)>& handler) const
	{
		impl_->on_request_authorization += handler;
	}
}
