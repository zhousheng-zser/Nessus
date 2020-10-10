#include "glaucus.hpp"
#include "time_utils.hpp"
#include "meta_string.hpp"
#include "aes_provider.hpp"
#include "crypto_error.hpp"
#include "io.hpp"

#include <array>
#include <random>
#include <string>
#include <cstddef>
#include <stdexcept>
#include <algorithm>
#include <type_traits>

#include <abi/sha3.hpp>
#include <abi/meta.hpp>
#include <filesystem.hpp>
#include <hash_utils.hpp>

namespace glasssix::crypto
{
	namespace meta = exposing::meta;
	namespace hashing = exposing::hashing;

	namespace
	{
		constexpr std::time_t minutes_of_day = 1440;
		constexpr std::size_t user_portrait_size = 512;
		constexpr std::size_t sha3_512_hash_size = hashing::sha3::details::hash_context<hashing::sha3::sha3_type::sha3_512>::final_hash_size;
		constexpr meta_string client_data_salt{ "Don't part with your illusions when they are gone you may still exist, but you have ceased to live." };
		constexpr meta_string user_portrait_salt{ "The reasonable man adapts himself to the world; the unreasonable one persists in trying to adapt the world to himself." };

		/// <summary>
		/// Deserializes a raw buffer into a user portrait.
		/// </summary>
		/// <param name="buffer">The buffer</param>
		/// <returns>The portrait</returns>
		std::array<std::uint64_t, user_portrait_size> make_user_portrait(exposing::param_span<const std::uint8_t> buffer)
		{
			std::array<std::uint64_t, user_portrait_size> result;

			for (std::size_t i = 0; i < result.size(); i++)
			{
				auto bytes = meta::apply_index_sequence<sizeof(std::uint64_t)>([&](auto... indexes)
					{
						return std::array{ buffer.data()[i * sizeof(std::uint64_t) + indexes]... };
					});

				result[i] = meta::make_number<std::uint64_t>(bytes);
			}

			return result;
		}

		/// <summary>
		/// Serializes a user portrait into raw bytes with salt appended.
		/// </summary>
		/// <param name="user_portrait">The user portrait</param>
		/// <returns>The raw bytes</returns>
		auto make_user_portarit_bytes(const std::array<std::uint64_t, user_portrait_size>& user_portrait)
		{
			std::array<std::uint8_t, user_portrait_size * sizeof(std::uint64_t) + user_portrait_salt.size()> result;
			auto iter = std::transform(user_portrait.begin(), user_portrait.end(), result.begin(), [&](std::uint64_t inner) { return meta::to_array(inner); });
			auto salt = client_data_salt.decrypt();

			return (std::copy(reinterpret_cast<const std::uint8_t*>(salt.data()), reinterpret_cast<const std::uint8_t*>(salt.data()) + salt.size(), iter), result);
		}

		/// <summary>
		/// Gets obfuscated factors.
		/// </summary>
		/// <param name="machine_id">The machine ID</param>
		/// <param name="timestamp">The timestamp</param>
		/// <returns>The result</returns>
		auto get_obfuscated_factors(exposing::param_span<const std::uint8_t> machine_id, std::time_t timestamp)
		{
			auto origin_timestamp = meta::to_array(static_cast<std::uint64_t>(timestamp));
			auto reverse_timestamp = meta::to_array(static_cast<std::uint64_t>(timestamp), false);
			std::vector<std::uint8_t> reverse_machine_id{ machine_id.rbegin(), machine_id.rend() };

			for (std::size_t i = 0; i < origin_timestamp.size(); i++)
			{
				origin_timestamp[i] ^= reverse_timestamp[i];
			}

			for (std::size_t i = 0; i < reverse_machine_id.size(); i++)
			{
				reverse_machine_id[i] ^= machine_id.data()[i];
			}

			// A helper function to create factors.
			auto factor_generator = [&](std::size_t number)
			{
				return meta::apply_index_sequence<sha3_512_hash_size / sizeof(std::uint64_t)>([&](auto... indexes)
					{
						auto seed = static_cast<int>(timestamp % minutes_of_day);

						auto timestamp_hash = hashing::sha3::hash_sha3_512(origin_timestamp);
						auto number_hash = hashing::sha3::hash_sha3_512(meta::to_array(number));
						auto machine_id_hash = hashing::sha3::hash_sha3_512(reverse_machine_id.data(), reverse_machine_id.size());
						auto helper = [&](auto&& hash, std::size_t index)
						{
							auto item = meta::apply_index_sequence<sizeof(std::uint64_t)>([&](auto... sub_indexes)
								{
									return std::array{ std::forward<decltype(hash)>(hash)[index * sizeof(std::uint64_t) + sub_indexes]... };
								});

							return meta::make_number<std::uint64_t>(item);
						};

						auto result = (meta::rotl(utils::hash_all(helper(number_hash, indexes), helper(timestamp_hash, indexes), helper(machine_id_hash, indexes)), seed) ^ ...);

						return result;
					});
			};

			// Finalizes the result.
			std::array<std::uint64_t, user_portrait_size> result;

			for (std::size_t i = result.size() - 1; i != 0; i--)
			{
				result[i] = factor_generator(i);
			}

			return result;
		}
	}

	class glaucus::impl
	{
	public:
		impl() : engine_{ std::random_device{}() }, user_portrait_{}
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
				throw crypto_error{ "The user portrait file does not exist." };
			}

			auto last_write_time = to_time_t(fs::last_write_time(path, code));

			if (code)
			{
				throw crypto_error{ code.message() };
			}

			auto buffer = io::read_all_bytes(path);

			if (!buffer)
			{
				throw crypto_error{ "Failed to open the user portrait file." };
			}

			if (buffer->size() != user_portrait_size)
			{
				throw crypto_error{ "Illegal user portrait file." };
			}

			// Initializes cryptography.
			std::reverse(buffer->begin(), buffer->end());
			init_user_portrait_cryptography(machine_id, last_write_time);
			user_portrait_ = make_user_portrait(user_portrait_encrypter_.decrypt(*buffer));
			init_client_data_cryptography(machine_id, last_write_time);
		}

		void save(std::string_view path)
		{
			auto timestamp = get_timestamp();
			auto ciphertext = (user_portrait_encrypter_.set_iv(meta::to_array(timestamp)), user_portrait_encrypter_.encrypt(user_portrait_));

			std::reverse(ciphertext.begin(), ciphertext.end());

			if (!io::write_all_bytes(path, ciphertext))
			{
				throw crypto_error{ "Failed to create the user portrait file." };
			}

			if (std::error_code code; (fs::last_write_time(std::string{ path }, from_time_t<fs::file_time_type>(timestamp), code), code))
			{
				throw crypto_error{ "Failed to update the file time." };
			}
		}

		void set_client_data_timestamp(std::time_t timestamp)
		{
			client_data_encrypter_.set_iv(meta::to_array(timestamp));
		}

		void generate(exposing::param_span<const std::uint8_t> machine_id, std::time_t timestamp)
		{
			std::array<std::uint64_t, user_portrait_size> user_portrait;

			for (std::size_t i = 0; i < user_portrait.size(); i++)
			{
				user_portrait[i] = distribution_(engine_);
			}

			user_portrait_ = user_portrait;
			init_client_data_cryptography(machine_id, timestamp);
		}

		std::vector<std::uint8_t> user_portrait() const
		{
			return std::vector<std::uint8_t>(user_portrait_.begin(), user_portrait_.end());
		}

		std::vector<std::uint8_t> forward(exposing::param_span<const std::uint8_t> buffer)
		{
			return client_data_encrypter_.encrypt(buffer);
		}

		std::vector<std::uint8_t> countermarch(exposing::param_span<const std::uint8_t> buffer)
		{
			return client_data_encrypter_.decrypt(buffer);
		}
	private:
		void init_user_portrait_cryptography(exposing::param_span<const std::uint8_t> machine_id, std::time_t timestamp)
		{
			std::vector<std::uint8_t> buffer;
			auto factors = get_obfuscated_factors(machine_id, timestamp);
			std::array<std::uint8_t, sizeof(factors) + user_portrait_salt.size()> salty_buffer;
			auto iter = std::copy(factors.rbegin(), factors.rend(), salty_buffer.begin());
			auto salt = user_portrait_salt.decrypt();

			std::copy(reinterpret_cast<const std::uint8_t*>(salt.data()), reinterpret_cast<const std::uint8_t*>(salt.data()) + salt.size(), iter);

			client_data_encrypter_.set_iv(meta::to_array(timestamp));
			client_data_encrypter_.set_key(salty_buffer);
		}

		void init_client_data_cryptography(exposing::param_span<const std::uint8_t> machine_id, std::time_t timestamp)
		{
			std::array<std::uint64_t, user_portrait_size> buffer;
			auto factors = get_obfuscated_factors(machine_id, timestamp);

			std::transform(user_portrait_.begin(), user_portrait_.end(), buffer.begin(), [&, index = std::size_t{}](std::uint64_t inner) mutable { return utils::hash_all(inner, factors[index++]); });
			client_data_encrypter_.set_key(make_user_portarit_bytes(buffer));
		}

		std::default_random_engine engine_;
		aes_provider client_data_encrypter_;
		aes_provider user_portrait_encrypter_;
		std::uniform_int_distribution<std::uint64_t> distribution_;
		std::array<std::uint64_t, user_portrait_size> user_portrait_;
	};

	glaucus::glaucus() : impl_{ std::make_unique<impl>() }
	{
	}

	glaucus::~glaucus()
	{
	}

	void glaucus::load(std::string_view path, exposing::param_span<std::uint8_t> machine_id)
	{
		impl_->load(path, machine_id);
	}

	void glaucus::save(std::string_view path)
	{
		impl_->save(path);
	}

	void glaucus::set_client_data_timestamp(std::time_t timestamp)
	{
		impl_->set_client_data_timestamp(timestamp);
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
