#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <string_view>

#include <abi/param_span.hpp>

namespace glasssix::crypto
{
	std::string base64_encode(exposing::param_span<const std::uint8_t> buffer);
	std::vector<std::uint8_t> base64_decode(std::string_view str);
}
