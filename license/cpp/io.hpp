#pragma once

#include <vector>
#include <cstdint>
#include <optional>
#include <string_view>

#include <abi/param_span.hpp>

namespace glasssix::io
{
	/// <summary>
	/// Reads all bytes from a file.
	/// </summary>
	/// <param name="path">The path</param>
	/// <returns>The bytes</returns>
	std::optional<std::vector<std::uint8_t>> read_all_bytes(std::string_view path);

	/// <summary>
	/// Writes all bytes into a file.
	/// </summary>
	/// <param name="path">The path</param>
	/// <param name="buffer">The buffer</param>
	/// <returns>True if succeeded; otherwise false</returns>
	bool write_all_bytes(std::string_view path, exposing::param_span<const std::uint8_t> buffer);
}
