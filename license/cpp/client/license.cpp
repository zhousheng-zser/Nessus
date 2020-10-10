#include "license.hpp"
#include "glaucus.hpp"
#include "time_utils.hpp"
#include "meta_string.hpp"
#include "license_info.hpp"
#include "authorization_client.hpp"

#ifdef __ANDROID__
#include "jni/utils.hpp"
#elif defined(_WIN32)
#include "smbios.hpp"
#else
#error "Unsupported platform."
#endif

#include <chrono>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

#include <logger.hpp>
#include <filesystem.hpp>
#include <abi/sha3.hpp>
#include <abi/platform_encoding.hpp>

#define NOGDI
#include <ShlObj.h>

namespace glasssix::license
{
	namespace hashing = exposing::hashing;
	namespace platform_encoding = exposing::platform_encoding;

	namespace
	{
		constexpr crypto::meta_string license_folder{ "glasssix" };
		constexpr crypto::meta_string license_file{ "product_keeper.dat" };

#ifdef __ANDROID__
		fs::path get_app_data_directory()
		{
			return utils::get_application_files_directory();
		}

		std::vector<std::uint8_t> get_machine_id()
		{
			auto device_id = jni::get_android_device_id();
			auto hash = hashing::sha3::hash_sha3_512(reinterpret_cast<const std::uint8_t*>(device_id.c_str()), device_id.size());

			return std::vector<std::uint8_t>(hash.begin(), hash.end());
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

		std::vector<std::uint8_t> get_machine_id()
		{
			try
			{
				if (auto info = smbios::read_smbios_info())
				{
					std::string buffer;

					buffer.append(std::to_string(info->processor.processor_id))
						.append(info->board.serial_number)
						.append(info->system.serial_number)
						.append(info->system.uuid);

					auto hash = hashing::sha3::hash_sha3_512(reinterpret_cast<const std::uint8_t*>(buffer.c_str()), buffer.size());

					return std::vector<std::uint8_t>(hash.begin(), hash.end());
				}
			}
			catch (const std::exception& ex)
			{
				return std::vector<std::uint8_t>{};
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
			license()
			{
				auto app_data_directory = get_app_data_directory();

				if (app_data_directory.empty())
				{
					return;
				}

				license_directory_ = app_data_directory / license_folder.decrypt().data();
				license_path_ = license_directory_ / license_file.decrypt().data();

				if (std::error_code code; !license_directory_.empty())
				{
					fs::create_directories(license_directory_, code);
				}
			}

			void evaluate()
			{

			}
		private:
			fs::path license_path_;
			fs::path license_directory_;
			crypto::glaucus encryptor_;
		};
	}

	EXPORT_NESSUS_LICENSE void evaluate_license(evaluate_license_callback_type callback)
	{
	}

	EXPORT_NESSUS_LICENSE void request_license_async(request_license_async_callback_type callback)
	{
	}
}
