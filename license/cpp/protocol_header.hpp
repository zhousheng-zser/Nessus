#pragma once

#include <array>
#include <string>
#include <cstdint>
#include <cstddef>
#include <string_view>

namespace glasssix::license
{
	enum class message_type : std::uint32_t
	{
		authorization
	};

	/// <summary>
	/// The protocol fixed header.
	/// </summary>
	struct protocol_header
	{
		static constexpr std::size_t hash_size = 28;
		static constexpr std::string_view header_text{ "!!!GLASSSIX_LICENSING!!!" };
		static constexpr std::size_t header_size = header_text.size() + sizeof(std::uint32_t) + sizeof(message_type) + sizeof(std::uint32_t) + hash_size;
		static constexpr std::uint32_t client_version = 0x0001'0000U;

		using buffer_type = std::array<std::uint8_t, header_size>;

		std::string header;
		std::uint32_t version;
		message_type type;
		std::uint32_t size;
		std::array<std::uint8_t, hash_size> hash;

		protocol_header(message_type type, std::uint32_t size);
		protocol_header(std::string_view header, std::uint32_t version, message_type type, std::uint32_t size, const std::array<std::uint8_t, hash_size>& hash);
		explicit operator bool() const noexcept;
		buffer_type compute_hash_and_dump();
		static protocol_header parse(const buffer_type& buffer);
	};
}
