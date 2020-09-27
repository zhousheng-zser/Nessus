#include "glaucus.hpp"
#include "time_utils.hpp"
#include "aes_provider.hpp"

#include <array>
#include <random>
#include <string>
#include <cstddef>
#include <fstream>
#include <algorithm>
#include <stdexcept>

#include <abi/sha3.hpp>
#include <abi/meta.hpp>
#include <filesystem.hpp>

namespace glasssix::crypto
{
	namespace meta = exposing::meta;
	namespace hashing = exposing::hashing;

	namespace
	{
		constexpr std::time_t minutes_of_day = 1440;
		constexpr std::size_t user_portrait_size = 512;
		constexpr std::size_t hash_size = hashing::sha3::details::hash_context<hashing::sha3::sha3_type::sha3_512>::final_hash_size;

		std::array<std::uint32_t, user_portrait_size> make_user_portrait(exposing::param_span<std::uint8_t> buffer)
		{
			return meta::apply_index_sequence<user_portrait_size>([&](auto... indexes)
				{
					return std::array
					{
						meta::make_number<std::uint32_t>(
							{
								buffer.data()[indexes * sizeof(std::uint32_t)],
								buffer.data()[indexes * sizeof(std::uint32_t) + 1],
								buffer.data()[indexes * sizeof(std::uint32_t) + 2],
								buffer.data()[indexes * sizeof(std::uint32_t) + 3]
							})...
					};
				});
		}

		auto make_user_portarit_bytes(const std::array<std::uint32_t, user_portrait_size>& user_portrait)
		{
			std::array<std::uint8_t, user_portrait_size * sizeof(std::uint32_t)> result;
			auto iter = result.begin();

			for (std::size_t i = 0; i < user_portrait_size; i++)
			{
				auto bytes = meta::to_array(user_portrait[i]);

				iter = std::copy(bytes.begin(), bytes.end(), iter);
			}

			return result;
		}
	}

	class glaucus::impl
	{
	public:
		impl() : timestamp_{}, engine_ { std::random_device{}() }
		{
		}

		~impl()
		{

		}

		void load(std::string_view path, exposing::param_span<std::uint8_t> machine_id)
		{
			std::error_code code;

			if (!fs::exists(path, code))
			{
				throw std::runtime_error{ "The user portrait file does not exist." };
			}

			auto last_write_time = fs::last_write_time(path, code);

			if (code)
			{
				throw std::runtime_error{ code.message() };
			}

			if (std::basic_ifstream<std::uint8_t> stream{ std::string(path), std::ios::in | std::ios::binary })
			{
				std::vector<std::uint8_t> buffer(std::istreambuf_iterator<std::uint8_t>{ stream }, std::istreambuf_iterator<std::uint8_t>{});

				if (buffer.size() != user_portrait_size)
				{
					throw std::runtime_error{ "Illegal user portrait file." };
				}
				
				std::reverse(buffer.begin(), buffer.end());
				
				init_cryptography(make_user_portrait(buffer), machine_id, time_utils::to_time_t(last_write_time));
			}
			else
			{
				throw std::runtime_error{ "Failed to open the user portrait file." };
			}
		}

		void save(std::string_view path)
		{
			if (std::basic_ofstream<std::uint8_t> stream{ std::string(path), std::ios::out | std::ios::binary | std::ios::trunc })
			{
				std::vector<std::uint8_t> intermeidate(user_portrait_.rbegin(), user_portrait_.rend());

				stream.write(intermeidate.data(), intermeidate.size());
			}
			else
			{
				throw std::runtime_error{ "Failed to create the user portrait file." };
			}

			if (std::error_code code;  (fs::last_write_time(std::string(path), time_utils::from_time_t<fs::file_time_type>(timestamp_), code), code))
			{
				throw std::runtime_error{ "Failed to write the file time." };
			}
		}

		void generate(exposing::param_span<const std::uint8_t> machine_id, std::time_t timestamp)
		{
			auto user_portrait = meta::apply_index_sequence<user_portrait_size>([this](auto... indexes) { return std::array{ (indexes, distribution_(engine_))... }; });

			init_cryptography(user_portrait, machine_id, timestamp);
		}

		std::vector<std::uint8_t> user_portrait() const
		{
			return std::vector<std::uint8_t>(user_portrait_.begin(), user_portrait_.end());
		}

		std::vector<std::uint8_t> forward(exposing::param_span<const std::uint8_t> buffer)
		{
			return aes_.encrypt(buffer);
		}

		std::vector<std::uint8_t> countermarch(exposing::param_span<const std::uint8_t> buffer)
		{
			return aes_.decrypt(buffer);
		}
	private:
		void init_cryptography(const std::array<std::uint32_t, user_portrait_size>& user_portrait, exposing::param_span<const std::uint8_t> machine_id, std::time_t timestamp)
		{
			auto origin_timestamp = meta::to_array(static_cast<std::uint64_t>(timestamp));
			auto reverse_timestamp = meta::to_array(static_cast<std::uint64_t>(timestamp), false);
			std::vector<std::uint8_t> reverse_machine_id(machine_id.rbegin(), machine_id.rend());

			for (std::size_t i = 0; i < origin_timestamp.size(); i++)
			{
				origin_timestamp[i] ^= reverse_timestamp[i];
			}
			
			for (std::size_t i = 0; i < reverse_machine_id.size(); i++)
			{
				reverse_machine_id[i] ^= machine_id.data()[i];
			}

			std::uint32_t factor = meta::apply_index_sequence<hash_size / sizeof(std::uint32_t)>([&](auto... indexes)
				{
					auto seed = static_cast<int>(timestamp % minutes_of_day);
					auto hash = hashing::sha3::hash_sha3_512(origin_timestamp);
					auto machine_id_hash = hashing::sha3::hash_sha3_512(reverse_machine_id.data(), reverse_machine_id.size());
					auto result = (meta::rotl(meta::make_number<std::uint32_t>(
						{
							static_cast<std::uint8_t>(hash[indexes * sizeof(std::uint32_t)] ^ machine_id_hash[indexes * sizeof(std::uint32_t)]),
							static_cast<std::uint8_t>(hash[indexes * sizeof(std::uint32_t) + 1] ^ machine_id_hash[indexes * sizeof(std::uint32_t) + 1]),
							static_cast<std::uint8_t>(hash[indexes * sizeof(std::uint32_t) + 2] ^ machine_id_hash[indexes * sizeof(std::uint32_t) + 2]),
							static_cast<std::uint8_t>(hash[indexes * sizeof(std::uint32_t) + 3] ^ machine_id_hash[indexes * sizeof(std::uint32_t) + 3])
						}), seed) ^ ...);

					return result;
				});

			auto buffer = meta::apply_index_sequence<user_portrait_size>([&](auto... indexes) { return std::array{ (indexes, (user_portrait[indexes] ^ factor))... }; });

			aes_.set_key_with_iv(make_user_portarit_bytes(buffer), machine_id);
		}

		aes_provider aes_;
		std::time_t timestamp_;
		std::default_random_engine engine_;
		std::uniform_int_distribution<std::uint32_t> distribution_;
		std::array<std::uint32_t, user_portrait_size> user_portrait_;
	};

	glaucus::glaucus() : impl_{ std::make_unique<impl>() }
	{
	}

	glaucus::~glaucus()
	{
	}

	void glaucus::generate(exposing::param_span<const std::uint8_t> machine_id, std::time_t timestamp)
	{
		impl_->generate(machine_id, timestamp);
	}

	std::vector<std::uint8_t> glaucus::user_portrait() const
	{
		return impl_->user_portrait();
	}

	std::vector<std::uint8_t> glaucus::forward(exposing::param_span<const std::uint8_t> buffer) const
	{
		return impl_->forward(buffer);
	}

	std::vector<std::uint8_t> glaucus::countermarch(exposing::param_span<const std::uint8_t> buffer) const
	{
		return impl_->countermarch(buffer);
	}
}
