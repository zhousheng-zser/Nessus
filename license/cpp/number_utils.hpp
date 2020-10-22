#pragma once

#include <string>
#include <cstdint>

#include <abi/param_span.hpp>

namespace glasssix
{
	/// <summary>
	/// Converts a buffer to a hexadecimal string.
	/// </summary>
	/// <param name="buffer">The buffer</param>
	/// <returns>The hexadecimal string</returns>
	std::string buffer_to_hex_string(exposing::param_span<const std::uint8_t> buffer);

	/// <summary>
	/// Converts a buffer to hexadecimal string.
	/// </summary>
	/// <typeparam name="T">The element type</typeparam>
	/// <param name="buffer">The buffer</param>
	/// <returns>The hexadecimal string</returns>
	template<typename T>
	std::string buffer_to_hex_string(exposing::param_span<const T> buffer)
	{
		return buffer_to_hex_string(exposing::param_span<const std::uint8_t>{ reinterpret_cast<const std::uint8_t*>(buffer.data()), buffer.size() * sizeof(T) });
	}
}
