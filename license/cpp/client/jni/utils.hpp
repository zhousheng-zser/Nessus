#pragma once

#include <string>
#include <type_traits>

#include <jni.h>

namespace glasssix::jni
{
	/// <summary>
	/// Gets the android device ID.
	/// </summary>
	/// <returns>The device ID</returns>
	std::string get_android_device_id();

	/// <summary>
	/// Gets the data directory of the current application.
	/// </summary>
	/// <returns>The data directory</returns>
	std::string get_application_files_directory();

	/// <summary>
	/// Converts a jstring to a string.
	/// </summary>
	/// <param name="str">The jstring</param>
	/// <returns>The string</returns>
	std::string to_string(jstring str);

	template<typename JObject>
	inline constexpr bool is_derived_from_jobject_v = std::conjunction_v<std::is_pointer<JObject>, std::is_base_of<std::remove_pointer_t<jobject>, std::remove_pointer_t<JObject>>>;

	template<typename JObject>
	constexpr auto jobject_as(jobject obj) noexcept -> std::enable_if_t<is_derived_from_jobject_v<JObject>, JObject>
	{
		return obj ? static_cast<JObject>(obj) : nullptr;
	}
}
