#include "protocol_header.hpp"

#include <algorithm>

#include <abi/meta.hpp>
#include <abi/sha3.hpp>

namespace glasssix::license
{
	namespace meta = exposing::meta;
	namespace hashing = exposing::hashing;

	protocol_header::protocol_header(message_type type, std::uint32_t size) : header{ header_text }, version{ client_version }, type{ type }, size{ size }, hash{}
	{
	}

	protocol_header::protocol_header(std::string_view header, std::uint32_t version, message_type type, std::uint32_t size, const std::array<std::uint8_t, hash_size>& hash) : header{ header }, version{ version }, type{ type }, size{ size }, hash{ hash }
	{
	}

	protocol_header::operator bool() const noexcept
	{
		return !hash.empty() && header == header_text && hashing::sha3::hash_sha3_224(meta::to_array(size)) == hash;
	}

	protocol_header protocol_header::parse(const buffer_type& buffer)
	{
		static constexpr std::size_t offset_version = header_text.size();
		static constexpr std::size_t offset_type = offset_version + sizeof(version);
		static constexpr std::size_t offset_size = offset_type + sizeof(type);
		static constexpr std::size_t offset_hash = offset_size + sizeof(size);

		return protocol_header
		{
			std::string(reinterpret_cast<const char*>(buffer.data()), header_text.size()),
			meta::make_number<std::uint32_t>(meta::sub_array<offset_version, sizeof(version)>::get(buffer)),
			static_cast<message_type>(meta::make_number<std::uint32_t>(meta::sub_array<offset_type, sizeof(type)>::get(buffer))),
			meta::make_number<std::uint32_t>(meta::sub_array<offset_size, sizeof(size)>::get(buffer)),
			meta::sub_array<offset_hash, hash_size>::get(buffer)
		};
	}

	protocol_header::buffer_type protocol_header::compute_hash_and_dump()
	{
		buffer_type result;

		auto version_bytes = meta::to_array(version);
		auto type_bytes = meta::to_array(static_cast<std::uint32_t>(type));
		auto size_bytes = meta::to_array(size);
		auto iter_begin = result.begin();
		auto iter = std::copy(header.begin(), header.end(), iter_begin);

		iter = std::copy(version_bytes.begin(), version_bytes.end(), iter);
		iter = std::copy(type_bytes.begin(), type_bytes.end(), iter);
		iter = std::copy(size_bytes.begin(), size_bytes.end(), iter);
		hash = hashing::sha3::hash_sha3_224(result.data(), iter - iter_begin);

		return result;
	}
}
