#include "license.hpp"
#include "io.hpp"
#include "base64.hpp"
#include "glaucus.hpp"
#include "time_utils.hpp"
#include "meta_string.hpp"
#include "license_info.hpp"
#include "license_error.hpp"
#include "license_config.hpp"
#include "interruptable_timer.hpp"
#include "authorization_client.hpp"

#ifdef __ANDROID__
#include "jni/utils.hpp"
#elif defined(_WIN32)
#include "smbios.hpp"

#define NOGDI
#define NOMINMAX
#include <ShlObj.h>
#elif defined(__linux__)
#include "smbios.hpp"
#endif

#include <mutex>
#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <cstdint>
#include <optional>
#include <exception>
#include <functional>
#include <string_view>

#include <logger.hpp>
#include <abi/sha3.hpp>
#include <fmt/format.h>
#include <delegate.hpp>
#include <filesystem.hpp>
#include <nlohmann/json.hpp>
#include <abi/param_string.hpp>

namespace glasssix::license
{
	namespace hashing = exposing::hashing;
	namespace platform_encoding = exposing::platform_encoding;

	namespace
	{
		constexpr std::int64_t deadline_seconds = 3600 * 24 * 5;
		constexpr std::int64_t watchdog_period = 1000 * 60 * 5;
		constexpr std::int64_t watchdog_deferred_time = 30 * 1000;

		constexpr crypto::meta_string license_folder{ "glasssix" };
		constexpr crypto::meta_string license_file{ "product_keeper.dat" };
		constexpr crypto::meta_string portrait_file{ "user_portrait.dat" };

#ifdef _WIN32
		fs::path get_app_data_directory()
		{
			if (wchar_t* buffer = nullptr; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, &buffer)))
			{
				std::unique_ptr<void, decltype(&CoTaskMemFree)> buffer_scope{ buffer, &CoTaskMemFree };

				return platform_encoding::win32::wide_to_narrow(buffer);
			}

			return fs::path{};
		}
#elif defined(__ANDROID__)
		fs::path get_app_data_directory()
		{
			return jni::get_application_files_directory();
		}
#elif defined(__linux__)
		fs::path get_app_data_directory()
		{
			return "/usr/local/etc";
		}
#endif

#ifdef __ANDROID__
		std::optional<std::vector<std::uint8_t>> get_machine_id()
		{
			auto device_id = jni::get_android_device_id();
			auto hash = hashing::sha3::hash_sha3_512(reinterpret_cast<const std::uint8_t*>(device_id.c_str()), device_id.size());

			return std::vector<std::uint8_t>{ hash.begin(), hash.end() };
		}
#elif defined(_WIN32) || defined(__linux__)
		std::optional<std::vector<std::uint8_t>> get_machine_id()
		{
			try
			{
				auto info = smbios::read_smbios_info();

				if (!info)
				{
					return std::nullopt;
				}

				std::string buffer;

				buffer.append(std::to_string(info->processor.processor_id))
					.append(info->board.serial_number)
					.append(info->system.serial_number)
					.append(info->system.uuid);

				auto hash = hashing::sha3::hash_sha3_512(reinterpret_cast<const std::uint8_t*>(buffer.c_str()), buffer.size());

				return std::vector<std::uint8_t>{ hash.begin(), hash.end() };
			}
			catch (const std::exception& ex)
			{
				return std::nullopt;
			}
		}
#endif

		/// <summary>
		/// A internal license manager.
		/// </summary>
		class license
		{
		public:
			delegate<void, bool, std::string_view> on_authorization;

			license(std::string_view license_key) : config_{ license_config::from_license_key(license_key) }, machine_id_{ get_machine_id() }
			{
				if (!machine_id_ || machine_id_->empty())
				{
					throw license_error{ "Failed to get the machine ID." };
				}

				if (!config_ || !*config_)
				{
					throw license_error{ "Invalid license key." };
				}

				auto app_data_directory = get_app_data_directory();

				if (app_data_directory.empty())
				{
					throw license_error{ "Failed to get the application data directory." };
				}

				auto license_directory = app_data_directory / license_folder.decrypt_as_string();

				license_path_ = license_directory / license_file.decrypt_as_string();
				portrait_path_ = license_directory / portrait_file.decrypt_as_string();

				// create_directoires returns false with a successful error code if the directory already exists.
				if (std::error_code code; !fs::create_directories(license_directory, code) && code)
				{
					throw license_error{ fmt::format(FMT_STRING("Failed to create the working directory: {}"), code.message()) };
				}

				init_authorization_client();
			}

			/// <summary>
			/// Evaluates the current license.
			/// </summary>
			/// <returns>The remaining time in seconds</returns>
			std::chrono::seconds evaluate()
			{
				std::scoped_lock lock{ mutex_ };

				auto timestamp = bind_license_file_timestamp();
				auto bytes = io::read_all_bytes(license_path_.string());

				if (!bytes)
				{
					throw license_error{ "Failed to read current license information." };
				}

				auto info = license_info::from_buffer(glaucus_.countermarch(*bytes));

				if (!info)
				{
					throw license_error{ "Failed to parse the current license information." };
				}

				if (!info->valid(timestamp, *machine_id_))
				{
					throw license_error{ "Invalid or expired license." };
				}

				update_timestamp(*info);

				return info->remaining_seconds();
			}

			void request_new()
			{
				std::scoped_lock lock{ mutex_ };

				client_.connect(config_->to_websocket_uri());
			}
		private:
			template<typename Callable>
			auto forward_exceptions(Callable&& callable)
			{
				return [=](auto&&... args)
				{
					try
					{
						callable(std::forward<decltype(args)>(args)...);
					}
					catch (const std::exception& ex)
					{
						on_authorization(false, fmt::format(FMT_STRING("An error occurred when reuqesting the server: {}"), ex.what()));
					}
				};
			}

			void init_authorization_client()
			{
				// Implements a Websocket state machine.
				client_.on_connect(forward_exceptions([this]
					{
						std::scoped_lock lock{ mutex_ };

						client_.request_authorization(authorization_request_message
							{
								config_->license_id,
								*machine_id_,
								get_local_timestamp()
							});
					}));

				client_.on_authorization(forward_exceptions([this](const authorization_response_message& message)
					{
						if (!message)
						{
							return on_authorization(false, message.status);
						}

						std::scoped_lock lock{ mutex_ };

						glaucus_.load(*machine_id_, message.user_portrait, message.server_timestamp);
						glaucus_.set_client_data_timestamp(message.server_timestamp);
						glaucus_.save(portrait_path_.string());

						if (!io::write_all_bytes(license_path_.string(), message.license))
						{
							throw license_error{ "Failed to create a license file." };
						}

						if (!set_file_last_write_timestamp(license_path_.string(), message.server_timestamp))
						{
							throw license_error{ "Failed to update the timestamp: {}" };
						}

						on_authorization(true, message.status);
					}));

				client_.on_async_error([this](const std::error_code& code)
					{
						forward_exceptions([&] { throw license_error{ code.message() }; })();
					});
			}

			std::time_t bind_license_file_timestamp()
			{
				glaucus_.load(portrait_path_.string(), *machine_id_);

				auto timestamp = get_file_last_write_timestamp(license_path_.string());

				if (!timestamp)
				{
					throw license_error{ "Failed to get the timestamp." };
				}

				auto bytes = io::read_all_bytes(license_path_.string());

				if (!bytes)
				{
					throw license_error{ "Failed to read current license information." };
				}

				// Sets the timestamp for encryption and decryption.
				glaucus_.set_client_data_timestamp(*timestamp);

				return *timestamp;
			}

			void update_timestamp(const license_info& info)
			{
				auto duplicate{ info };
				auto timestamp = get_local_timestamp();

				duplicate.last_running_time = timestamp;
				glaucus_.set_client_data_timestamp(timestamp);

				if (!io::write_all_bytes(license_path_.string(), glaucus_.forward(duplicate.to_buffer())))
				{
					throw license_error{ "Failed to update the license information." };
				}

				if (!set_file_last_write_timestamp(license_path_.string(), timestamp))
				{
					throw license_error{ "Failed to update the license timestamp." };
				}
			}

			std::mutex mutex_;
			fs::path license_path_;
			fs::path portrait_path_;
			crypto::glaucus glaucus_;
			authorization_client client_;
			std::optional<license_config> config_;
			std::optional<std::vector<std::uint8_t>> machine_id_;
		};

		interruptable_timer watchdog_timer;
		std::unique_ptr<license> global_license;
		std::shared_ptr<std::exception_ptr> last_exception;
		std::shared_ptr<std::thread> watchdog_initialization_thread;
		delegate<void, std::string_view, std::int64_t> deadline_callback;
		const exposing::abi_not_initialized initial_exception{ u8"The license has not been initialized correctly. Please initialize the SDK before any invocations." };
	}

	namespace
	{
		/// <summary>
		/// Clears the last exception and marks it as a success.
		/// </summary>
		void clear_last_exception()
		{
			std::atomic_store_explicit(&last_exception, std::make_shared<std::exception_ptr>(), std::memory_order_release);
		}

		/// <summary>
		/// Assigns the last exception.
		/// </summary>
		/// <typeparam name="T">The exception type</typeparam>
		/// <param name="ex">The exception</param>
		template<typename T>
		void set_last_exception(T&& ex) try
		{
			throw std::forward<T>(ex);
		}
		catch (...)
		{
			return std::atomic_store_explicit(&last_exception, std::make_shared<std::exception_ptr>(std::current_exception()), std::memory_order_release);
		}

		/// <summary>
		/// Initializes the watchdog timer.
		/// </summary>
		void init_watchdog_timer()
		{
			set_last_exception(initial_exception);
			watchdog_initialization_thread.reset(new std::thread
				{
					[]
					{
						watchdog_timer.start(watchdog_period, watchdog_deferred_time, []
							{
								if (!global_license)
								{
									return set_last_exception(initial_exception);
								}

								evaluate_license([](void* context, bool valid, const char* message, std::int64_t remaining_seconds)
									{
										// Notifies the consumer nearing expiration.
										if (remaining_seconds < deadline_seconds)
										{
											deadline_callback(message, remaining_seconds);
										}

										if (valid)
										{
											return clear_last_exception();
										}

										set_last_exception(exposing::abi_failure{ exposing::format(FMT_STRING(u8"Failed in license evaluation. {}"), message) });
									});

								request_license_async([](void* context, bool success, const char* message)
									{
										LOG_ND(INFO) << message;
									});
							});
					}
				},
				[](std::thread* inner)
					{
						if (inner->joinable())
						{
							inner->join();
						}
					});
		}

		auto dummy_initializer = []
		{
			return (init_watchdog_timer(), 0);
		}();
	}
}

using namespace glasssix;
using namespace glasssix::license;

EXPORT_NESSUS_LICENSE void init_license_system(const char* license_key)
{
	if (license_key)
	{
		static std::once_flag flag;

		std::call_once(flag, [&]
			{
				try
				{
					// In case of expensive initialzation on Android, we defer the first tick of the watchdog to wait for the initialization to accomplish.
					watchdog_initialization_thread.reset();
					global_license = std::make_unique<glasssix::license::license>(license_key);
					clear_last_exception();
				}
				catch (const std::exception& ex)
				{
					set_last_exception(exposing::abi_not_initialized{ exposing::format(FMT_STRING(u8"Failed to initialize the license system. {}"), ex.what()) });
				}
			});
	}
}

EXPORT_NESSUS_LICENSE void evaluate_license(evaluate_license_callback_type callback, void* context)
{
	if (!global_license || callback == nullptr)
	{
		return;
	}

	try
	{
		watchdog_initialization_thread.reset();

		std::int64_t remaining_seconds = global_license->evaluate().count();
		std::int64_t seconds = remaining_seconds % 60;
		std::int64_t minutes = remaining_seconds / 60 % 60;
		std::int64_t hours = remaining_seconds / 3600 % 24;
		std::int64_t days = remaining_seconds / 86400;

		callback(context, true, fmt::format(FMT_STRING("License remaining time: {}d {:02}:{:02}:{:02}."), days, hours, minutes, seconds).c_str(), remaining_seconds);
	}
	catch (const std::exception& ex)
	{
		callback(context, false, ex.what(), 0);
	}
}

EXPORT_NESSUS_LICENSE void request_license_async(request_license_async_callback_type callback, void* context)
{
	struct token_wrapper
	{
		std::shared_ptr<delegate_token> token;
	};

	if (global_license)
	{
		watchdog_initialization_thread.reset();

		auto wrapper = std::make_shared<token_wrapper>();
		wrapper->token = global_license->on_authorization.add_listener_auto_removal([=, wrapper = wrapper](bool success, std::string_view message) mutable { wrapper->token.reset(); callback(context, success, message.data()); });
		global_license->request_new();
	}
}

EXPORT_NESSUS_LICENSE void set_license_deadline_callback(license_deadline_callback_type callback, void* context)
{
	if (callback)
	{
		watchdog_initialization_thread.reset();
		deadline_callback += [=](std::string_view message, std::int64_t remaining_seconds) { callback(context, message.data(), remaining_seconds); };
	}
}

EXPORT_NESSUS_LICENSE std::int32_t get_last_license_error_code() noexcept
{
	auto ex = std::atomic_load_explicit(&last_exception, std::memory_order::memory_order_acquire);

	return glasssix::exposing::abi_safe_call([&]
		{
			if (ex && *ex)
			{
				std::rethrow_exception(*ex);
			}
		});
}
