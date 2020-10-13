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
#else
#error "Unsupported platform."
#endif

#include <mutex>
#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <string_view>

#include <logger.hpp>
#include <abi/sha3.hpp>
#include <fmt/format.h>
#include <delegate.hpp>
#include <filesystem.hpp>
#include <nlohmann/json.hpp>
#include <abi/platform_encoding.hpp>

#define NOGDI
#include <ShlObj.h>

namespace glasssix::license
{
	namespace hashing = exposing::hashing;
	namespace platform_encoding = exposing::platform_encoding;

	namespace
	{
		constexpr std::int64_t watchdog_period = 1000 * 60 * 5;
		constexpr std::int64_t watchdog_deferred_time = watchdog_period;

		constexpr crypto::meta_string license_folder{ "glasssix" };
		constexpr crypto::meta_string license_file{ "product_keeper.dat" };
		constexpr crypto::meta_string portrait_file{ "user_portrait.dat" };

#ifdef __ANDROID__
		fs::path get_app_data_directory()
		{
			return utils::get_application_files_directory();
		}

		std::vector<std::uint8_t> get_machine_id()
		{
			auto device_id = jni::get_android_device_id();
			auto hash = hashing::sha3::hash_sha3_512(reinterpret_cast<const std::uint8_t*>(device_id.c_str()), device_id.size());

			return std::vector<std::uint8_t>{ hash.begin(), hash.end() };
		}
#elif defined(_WIN32)
		fs::path get_app_data_directory()
		{
			if (wchar_t* buffer = nullptr; SUCCEEDED(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, &buffer)))
			{
				std::unique_ptr<void, decltype(&CoTaskMemFree)> buffer_scope{ buffer, &CoTaskMemFree };

				return platform_encoding::win32::wide_to_narrow(buffer);
			}

			return fs::path{};
		}

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
#else
#error "Unsupported platform."
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
						client_.request_authorization(authorization_request_message
							{
								config_->product_id,
								*machine_id_,
								get_timestamp()
							});
					}));

				client_.on_authorization(forward_exceptions([this](const authorization_response_message& message)
					{
						if (!message)
						{
							return on_authorization(false, message.status);
						}

						glaucus_.load(*machine_id_, message.user_portrait, message.server_timestamp);
						glaucus_.set_client_data_timestamp(message.server_timestamp);
						glaucus_.save(portrait_path_.string());

						if (!io::write_all_bytes(license_path_.string(), message.license))
						{
							throw license_error{ "Failed to create a license file." };
						}

						if (std::error_code code; (fs::last_write_time(license_path_, from_time_t<fs::file_time_type>(message.server_timestamp), code), code))
						{
							throw license_error{ fmt::format(FMT_STRING("Failed to update the timestamp: {}"), code.message()) };
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

				std::error_code code;
				auto timestamp = to_time_t(fs::last_write_time(license_path_, code));

				if (code)
				{
					throw license_error{ fmt::format(FMT_STRING("Failed to get the timestamp: {}"), code.message()) };
				}

				auto bytes = io::read_all_bytes(license_path_.string());

				if (!bytes)
				{
					throw license_error{ "Failed to read current license information." };
				}

				// Sets the timestamp for encryption and decryption.
				glaucus_.set_client_data_timestamp(timestamp);

				return timestamp;
			}

			void update_timestamp(const license_info& info)
			{
				auto duplicate{ info };
				auto timestamp = get_timestamp();

				duplicate.last_running_time = timestamp;
				glaucus_.set_client_data_timestamp(timestamp);

				if (!io::write_all_bytes(license_path_.string(), glaucus_.forward(duplicate.to_buffer())))
				{
					throw license_error{ "Failed to update the license information." };
				}

				if (std::error_code code; (fs::last_write_time(license_path_, from_time_t<fs::file_time_type>(timestamp), code), code))
				{
					throw license_error{ fmt::format(FMT_STRING("Failed to update the license timestamp: {}"), code.message()) };
				}
			}

			fs::path license_path_;
			fs::path portrait_path_;
			crypto::glaucus glaucus_;
			authorization_client client_;
			std::optional<license_config> config_;
			std::optional<std::vector<std::uint8_t>> machine_id_;
		};

		std::mutex mutex_license;
		interruptable_timer watchdog_timer;
		std::unique_ptr<license> global_license_;
	}

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
						global_license_ = std::make_unique<license>(license_key);
						watchdog_timer.start(watchdog_period, watchdog_deferred_time, []
							{
								evaluate_license([](void* context, bool valid, const char* message, std::int64_t remaining_seconds)
									{
										if (!valid)
										{
											LOG(FATAL) << fmt::format(FMT_STRING("The application has detected a fatal license exception: {}"), message);
										}
									});
							});
					}
					catch (const std::exception& ex)
					{
						LOG(FATAL) << ex.what();
					}
				});
		}
	}

	EXPORT_NESSUS_LICENSE void request_license_async(void* context)
	{
		if (global_license_)
		{
			std::scoped_lock lock{ mutex_license };

			global_license_->request_new();
		}
	}

	EXPORT_NESSUS_LICENSE void evaluate_license(evaluate_license_callback_type callback, void* context)
	{
		if (!global_license_ || callback == nullptr)
		{
			return;
		}

		try
		{
			std::int64_t remaining_seconds = [] { std::scoped_lock lock{ mutex_license }; return global_license_->evaluate().count(); }();
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

	EXPORT_NESSUS_LICENSE void set_request_license_async_callback(request_license_async_callback_type callback, void* context)
	{
		if (global_license_ && callback)
		{
			std::scoped_lock lock{ mutex_license };

			global_license_->on_authorization += [=](bool success, std::string_view message) { callback(context, success, message.data()); };
		}
	}
}
