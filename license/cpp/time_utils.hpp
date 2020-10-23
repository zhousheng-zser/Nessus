#pragma once

#include <ctime>
#include <optional>
#include <string_view>

namespace glasssix
{
	/// <summary>
	/// Retrieves the current local timestamp.
	/// </summary>
	/// <returns>The current local timestamp</returns>
	std::time_t get_local_timestamp();

	/// <summary>
	/// Retrieves the last write timestamp of a file.
	/// </summary>
	/// <param name="path">The path</param>
	/// <returns>The timestamp</returns>
	std::optional<std::time_t> get_file_last_write_timestamp(std::string_view path);

	/// <summary>
	/// Sets the last write timestamp of a file.
	/// </summary>
	/// <param name="path">The path</param>
	/// <param name="timestamp">The timestamp</param>
	/// <returns>True if the opeartion succeeds; otherwise false</returns>
	bool set_file_last_write_timestamp(std::string_view path, std::time_t timestamp);
}
