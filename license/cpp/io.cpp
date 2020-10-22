#include "io.hpp"

#include <string>
#include <fstream>

namespace glasssix::io
{
	std::optional<std::vector<std::uint8_t>> read_all_bytes(std::string_view path)
	{
		if (std::ifstream stream{ std::string{ path }, std::ios::in | std::ios::binary }; stream)
		{
			return std::vector<std::uint8_t>{ std::istreambuf_iterator<char>{ stream }, std::istreambuf_iterator<char>{} };
		}

		return std::nullopt;
	}

	bool write_all_bytes(std::string_view path, exposing::param_span<const std::uint8_t> buffer)
	{
		if (std::ofstream stream{ std::string{ path }, std::ios::out | std::ios::binary | std::ios::trunc }; stream)
		{
			return !stream.write(reinterpret_cast<const char*>(buffer.data()), buffer.size()).fail();
		}

		return false;
	}
}
