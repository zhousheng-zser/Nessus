#include "utils.hpp"
#include "entry.hpp"
#include "local_ref.hpp"
#include "reflection_cache.hpp"

#include <array>
#include <regex>
#include <memory>
#include <cstddef>
#include <fstream>
#include <iterator>
#include <optional>

#include <abi/sha3.hpp>
#include <abi/meta.hpp>

namespace glasssix::jni
{
	namespace meta = exposing::meta;
	namespace hashing = exposing::hashing;

	namespace
	{
		auto& internal_caches = get_license_reflection_caches();

		/// <summary>
		/// Gets the context of the main activity.
		/// </summary>
		/// <param name="env">The JNI environment</param>
		/// <returns>The context</returns>
		jobject get_main_activity_context(JNIEnv* env)
		{
			auto current_activity_thread = env->CallStaticObjectMethod(internal_caches.class_activity_thread.get(), internal_caches.method_activity_thread_current_activity_thread);

			return env->CallObjectMethod(current_activity_thread, internal_caches.method_activity_thread_get_application);
		}

		/// <summary>
		/// Gets the CPU serial number of ARM hardware.
		/// </summary>
		/// <returns>The CPU serial number</returns>
		std::string get_cpu_serial_number()
		{
			thread_local std::regex pattern{ R"(Serial\s*?\:\s*?(\S+?)$)", std::regex_constants::icase | std::regex_constants::ECMAScript };

			if (std::ifstream stream{ "/proc/cpuinfo", std::ios::in | std::ios::binary })
			{
				std::smatch matches;
				std::string buffer_(std::istreambuf_iterator<char>{ stream }, std::istreambuf_iterator<char>{});

				return std::regex_search(buffer_, matches, pattern) ? matches[1].str() : std::string();
			}

			return std::string{};
		}

		/// <summary>
		/// Gets the Android ID.
		/// </summary>
		/// <returns>The Android ID</returns>
		std::string get_android_id()
		{
			auto env = reflection_cache::instance().get_thread_env();

			if (env == nullptr)
			{
				return std::string{};
			}

			auto context = get_main_activity_context(env);

			if (context == nullptr)
			{
				return std::string{};
			}

			local_ref content_resolver{ env->CallObjectMethod(context, internal_caches.method_context_get_content_resolver) };
			auto android_id = static_cast<jstring>(env->CallStaticObjectMethod(internal_caches.class_settings_secure.get(), internal_caches.method_settings_secure_get_string, content_resolver.get(), env->NewStringUTF("android_id")));

			return to_string(android_id);
		}
	}

	std::string get_android_device_id()
	{
		std::string input;
		auto hash_code = hashing::sha3::hash_sha3_512(reinterpret_cast<const std::uint8_t*>(input.append(get_android_id()).append(get_cpu_serial_number()).c_str()), input.size());
		auto hash_chars = meta::to_char_array(hash_code);

		return std::string{ hash_chars.begin(), hash_chars.end() };
	}

	std::string get_application_files_directory()
	{
		auto env = reflection_cache::instance().get_thread_env();

		if (env == nullptr)
		{
			return std::string{};
		}

		auto context = get_main_activity_context(env);

		if (context == nullptr)
		{
			return std::string{};
		}

		auto files = env->CallObjectMethod(context, internal_caches.method_context_get_files_dir);
		auto directory = static_cast<jstring>(env->CallObjectMethod(files, internal_caches.method_file_get_absolute_path));

		return to_string(directory);
	}

	std::string to_string(jstring str)
	{
		auto env = reflection_cache::instance().get_thread_env();

		return env && str ? std::string{ env->GetStringUTFChars(str, nullptr), static_cast<std::size_t>(env->GetStringUTFLength(str)) } : std::string{};
	}
}
