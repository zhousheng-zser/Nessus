#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>

namespace glasssix::crypto
{
	struct base64
	{
		static std::string encode(const std::uint8_t* buffer, std::size_t size);
		static std::vector<std::uint8_t> decode(const std::string& encoded_string);
	};
}
