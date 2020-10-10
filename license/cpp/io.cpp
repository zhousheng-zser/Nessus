#include "io.hpp"

#include <string>
#include <fstream>

namespace glasssix::io
{
	std::optional<std::vector<std::uint8_t>> read_all_bytes(std::string_view path)
	{
		if (std::basic_ifstream<std::uint8_t> stream{ std::string{ path }, std::ios::in | std::ios::binary }; stream)
		{
			return std::vector<std::uint8_t>{ std::istreambuf_iterator<std::uint8_t>(stream), std::istreambuf_iterator<std::uint8_t>{} };
		}

		return std::nullopt;
	}

	bool write_all_bytes(std::string_view path, exposing::param_span<const std::uint8_t> buffer)
	{
		if (std::basic_ofstream<std::uint8_t> stream{ std::string{ path }, std::ios::out | std::ios::binary }; stream)
		{
			return !stream.write(buffer.data(), buffer.size()).fail();
		}

		return false;
	}
}
