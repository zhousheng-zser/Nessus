#include "number_utils.hpp"

#include <cstddef>
#include <string_view>

namespace glasssix
{
	std::string buffer_to_hex_string(exposing::param_span<const std::uint8_t> buffer)
	{
		std::string result(buffer.size() * exposing::meta::hexadecimal_character_size_v<std::uint8_t>, '\0');

		for (std::size_t i = 0, j = 0; i < result.size(); )
		{
			for (auto item : exposing::meta::details::to_hexadecimal_character(buffer.data()[j++]))
			{
				result[i++] = item;
			}
		}

		return result;
	}
}
