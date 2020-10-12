#include "base64.hpp"

#include <openssl/evp.h>

namespace glasssix::crypto
{
	std::string base64_encode(exposing::param_span<const std::uint8_t> buffer)
	{
		if (buffer.empty())
		{
			return std::string{};
		}

		std::size_t size = 4 * ((buffer.size() + 2) / 3);
		std::string result(size, '\0');
		
		if (std::size_t real_size = static_cast<std::size_t>(EVP_EncodeBlock(reinterpret_cast<std::uint8_t*>(result.data()), buffer.data(), static_cast<int>(buffer.size()))); real_size != size)
		{
			result.clear();
			result.shrink_to_fit();
		}

		return result;
	}

	std::vector<std::uint8_t> base64_decode(std::string_view str)
	{
		if (str.empty())
		{
			return std::vector<std::uint8_t>{};
		}

		std::size_t size = 3 * str.size() / 4;
		std::vector<std::uint8_t> result(size);

		if (std::size_t real_size = static_cast<std::size_t>(EVP_DecodeBlock(result.data(), reinterpret_cast<const std::uint8_t*>(str.data()), static_cast<int>(str.size()))); real_size != size)
		{
			result.clear();
			result.shrink_to_fit();
		}

		return result;
	}
}
