#include "glaucus.hpp"
#include "time_utils.hpp"
#include "meta_string.hpp"
#include "aes_provider.hpp"
#include "crypto_error.hpp"
#include "io.hpp"
#include "number_utils.hpp"

#include <array>
#include <random>
#include <string>
#include <cstddef>
#include <utility>
#include <stdexcept>
#include <algorithm>
#include <type_traits>
#include <iostream>

#include <abi/sha3.hpp>
#include <abi/meta.hpp>
#include <fmt/format.h>
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
		/// Deserializes a raw buffer into a numeric buffer.
		/// </summary>
		/// <param name="buffer">The buffer</param>
		/// <returns>The portrait</returns>
		std::vector<std::uint64_t> make_user_portrait(exposing::param_span<const std::uint8_t> buffer)
		{
			std::vector<std::uint64_t> result(buffer.size() / sizeof(std::uint64_t));

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
		/// Serializes a numeric buffer into a byte buffer.
		/// </summary>
		/// <typeparam name="Buffer">The buffer type</typeparam>
		/// <param name="buffer">The buffer</param>
		/// <param name="additional_size">The additional size</param>
		/// <returns>The buffer</returns>
		template<typename Buffer>
		std::vector<std::uint8_t> make_byte_buffer(Buffer&& buffer, std::size_t additional_size = 0)
		{
			std::vector<std::uint8_t> result(std::forward<Buffer>(buffer).size() * sizeof(typename std::decay_t<Buffer>::value_type) + additional_size);
			auto iter = result.begin();

			for (const auto& item : std::forward<Buffer>(buffer))
			{
				auto bytes = meta::to_array(item);

				iter = std::copy(bytes.begin(), bytes.end(), iter);
			}

			return result;
		}

		/// <summary>
		/// Serializes a numeric buffer into a byte buffer with salt appended.
		/// </summary>
		/// <typeparam name="Buffer">The buffer type</typeparam>
		/// <typeparam name="MetaString">The salty type</typeparam>
		/// <param name="buffer">The buffer</param>
		/// <param name="salt">The salt</param>
		/// <returns>The salty buffer</returns>
		template<typename Buffer, typename MetaString>
		std::vector<std::uint8_t> make_salty_byte_buffer(Buffer&& buffer, MetaString&& salt)
		{
			static constexpr std::size_t salty_size = std::decay_t<MetaString>::buffer_size;
			auto result = make_byte_buffer(std::forward<Buffer>(buffer), salty_size);
			auto salty_buffer = std::forward<MetaString>(salt).decrypt();

			std::copy(reinterpret_cast<const std::uint8_t*>(salty_buffer.data()), reinterpret_cast<const std::uint8_t*>(salty_buffer.data()) + salty_buffer.size(), result.end() - salty_size);

			return result;
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
			std::vector<std::uint64_t> result(user_portrait_size);

			for (std::size_t i = 0, j = result.size() - 1; i < result.size(); i++, j--)
			{
				result[j] = factor_generator(j);
			}

			return result;
		}
	}

	class glaucus::impl
	{
	public:
		impl() : engine_ { std::random_device{}() }
		{
		}

		~impl()
		{

		}

		void load(exposing::param_span<const std::uint8_t> machine_id, exposing::param_span<const std::uint8_t> user_portrait, std::time_t timestamp)
		{
			set_machine_id(machine_id);
			cipher_user_portrait_.assign(user_portrait.begin(), user_portrait.end());
			init_user_portrait_cryptography(timestamp);
		}

		void load(std::string_view path, exposing::param_span<const std::uint8_t> machine_id)
		{
			std::error_code code;

			if (!fs::exists(path, code))
			{
				throw crypto_error{ "The user portrait file does not exist." };
			}

			auto last_write_time = to_time_t(fs::last_write_time(path, code));

			if (code)
			{
				throw crypto_error{ fmt::format(FMT_STRING("Failed to get the timestamp: {}"), code.message()) };
			}

			auto buffer = io::read_all_bytes(path);

			if (!buffer)
			{
				throw crypto_error{ "Failed to open the user portrait file." };
			}

			if (buffer->size() != aes_provider::get_encrypted_size(user_portrait_size * sizeof(std::uint64_t)))
			{
				throw crypto_error{ "Illegal user portrait file." };
			}

			set_machine_id(machine_id);

			// Initializes cryptography.
			std::reverse(buffer->begin(), buffer->end());
			cipher_user_portrait_ = std::move(*buffer);
			init_user_portrait_cryptography(last_write_time);
		}

		void save(std::string_view path)
		{
			auto timestamp = get_local_timestamp();
			auto old_user_portrait = user_portrait_encrypter_.decrypt(cipher_user_portrait_);
			auto new_user_portrait = (init_user_portrait_cryptography(timestamp), user_portrait_encrypter_.encrypt(old_user_portrait));

			cipher_user_portrait_ = new_user_portrait;
			std::reverse(new_user_portrait.begin(), new_user_portrait.end());

			if (!io::write_all_bytes(path, new_user_portrait))
			{
				throw crypto_error{ "Failed to create the user portrait file." };
			}

			if (std::error_code code; (fs::last_write_time(std::string{ path }, from_time_t<fs::file_time_type>(timestamp), code), code))
			{
				throw crypto_error{ "Failed to update the timestamp." };
			}
		}

		void set_machine_id(exposing::param_span<const std::uint8_t> machine_id)
		{
			machine_id_.assign(machine_id.begin(), machine_id.end());
		}

		void set_client_data_timestamp(std::time_t timestamp)
		{
			client_data_encrypter_.set_iv(meta::to_array(timestamp));
			init_client_data_cryptography(timestamp);
		}

		void generate(exposing::param_span<const std::uint8_t> machine_id, std::time_t timestamp)
		{
			std::vector<std::uint64_t> user_portrait(user_portrait_size);

			for (auto& item : user_portrait)
			{
				item = distribution_(engine_);
			}

			auto buffer = make_byte_buffer(user_portrait);

			set_machine_id(machine_id);
			init_user_portrait_cryptography(timestamp);
			cipher_user_portrait_ = user_portrait_encrypter_.encrypt(buffer);
			init_client_data_cryptography(timestamp);
			set_client_data_timestamp(timestamp);
		}

		std::vector<std::uint8_t> user_portrait()
		{
			return cipher_user_portrait_;
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
		void init_user_portrait_cryptography(std::time_t timestamp)
		{
			std::vector<std::uint8_t> buffer;
			auto factors = get_obfuscated_factors(machine_id_, timestamp);
			auto salty_buffer = make_salty_byte_buffer(factors, user_portrait_salt);

			user_portrait_encrypter_.set_iv(meta::to_array(timestamp));
			user_portrait_encrypter_.set_key(salty_buffer);
		}

		void init_client_data_cryptography(std::time_t timestamp)
		{
			auto factors = get_obfuscated_factors(machine_id_, timestamp);
			auto real_user_portrait = make_user_portrait(user_portrait_encrypter_.decrypt(cipher_user_portrait_));
			std::vector<std::uint64_t> buffer(real_user_portrait.size());

			std::transform(real_user_portrait.begin(), real_user_portrait.end(), buffer.begin(), [&, index = std::size_t{}](std::uint64_t inner) mutable { return utils::hash_all(inner, factors[index++]); });
			client_data_encrypter_.set_key(make_salty_byte_buffer(buffer, client_data_salt));
		}

		std::default_random_engine engine_;
		aes_provider client_data_encrypter_;
		aes_provider user_portrait_encrypter_;
		std::vector<std::uint8_t> machine_id_;
		std::vector<std::uint8_t> cipher_user_portrait_;
		std::uniform_int_distribution<std::uint64_t> distribution_;
	};

	glaucus::glaucus() : impl_{ std::make_unique<impl>() }
	{
	}

	glaucus::~glaucus()
	{
	}

	void glaucus::load(exposing::param_span<const std::uint8_t> machine_id, exposing::param_span<const std::uint8_t> user_portrait, std::time_t timestamp) const
	{
		impl_->load(machine_id, user_portrait, timestamp);
	}

	void glaucus::load(std::string_view path, exposing::param_span<const std::uint8_t> machine_id) const
	{
		impl_->load(path, machine_id);
	}

	void glaucus::save(std::string_view path) const
	{
		impl_->save(path);
	}

	void glaucus::set_client_data_timestamp(std::time_t timestamp) const
	{
		impl_->set_client_data_timestamp(timestamp);
	}

	void glaucus::generate(exposing::param_span<const std::uint8_t> machine_id, std::time_t client_data_timestamp) const
	{
		impl_->generate(machine_id, client_data_timestamp);
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
