#include "license.hpp"
#include "authorization_client.hpp"

#ifdef __ANDROID__
#include "jni/utils.hpp"
#elif defined(_WIN32)
#include "smbios.hpp"
#else
#error "Unsupported platform."
#endif

#include <vector>
#include <cstdint>

#include <logger.hpp>
#include <abi/sha3.hpp>
#include <abi/consumer.hpp>

namespace glasssix::license
{
	namespace hashing = exposing::hashing;

	namespace
	{
		std::vector<std::uint8_t> get_machine_id()
		{
			
#ifdef __ANDROID__
			auto device_id = jni::get_android_device_id();
			auto hash = hashing::sha3::hash_sha3_512(reinterpret_cast<const std::uint8_t*>(device_id.c_str()), device_id.size());

			return std::vector<std::uint8_t>(hash.begin(), hash.end());
#else
			try
			{
				if (auto info = smbios::read_smbios_info())
				{
					info->processor.processor_id;
					info->board.serial_number;
					info->system.serial_number;
					info->system.uuid;
				}
			}
			catch (const std::exception& ex)
			{
				return std::vector<std::uint8_t>();
			}

			return std::vector<std::uint8_t>();
#endif
		}
	}

	EXPORT_NESSUS_LICENSE void check_license_async(void(*callback)(bool, const char*))
	{
	}

	EXPORT_NESSUS_LICENSE void request_license_async(void(*callback)(bool, const char*))
	{
	}
}
