#include "aes_provider.hpp"

#include <limits>
#include <cstddef>
#include <algorithm>

#include <abi/meta.hpp>
#include <abi/sha3.hpp>
#include <openssl/aes.h>

namespace glasssix::crypto
{
	namespace meta = exposing::meta;
	namespace hashing = exposing::hashing;

	class aes_provider::impl
	{
	public:
		void set_key_with_iv(exposing::param_span<const std::uint8_t> key, const std::array<std::uint8_t, iv_size>& iv)
		{
			// Assigns the IV and the hashed key into internal buffers.
			auto key_hash = hashing::sha3::hash_sha3_512(key.data(), key.size());

			initialization_vector_ = iv;
			AES_set_encrypt_key(key_hash.data(), static_cast<int>(key_hash.size()) * meta::byte_bits, &encyption_key_);
			AES_set_decrypt_key(key_hash.data(), static_cast<int>(key_hash.size()) * meta::byte_bits, &decryption_key_);
		}

		void set_key_with_iv(exposing::param_span<const std::uint8_t> key, exposing::param_span<const std::uint8_t> iv)
		{
			auto real_iv = hashing::sha3::hash_sha3_224(iv.data(), iv.size());

			set_key_with_iv(key, meta::sub_array<0, iv_size>::get(real_iv));
		}

		std::vector<std::uint8_t> encrypt(exposing::param_span<const std::uint8_t> plaintext)
		{
			// Makes the plain text aligned by 16 bytes.
			auto subsential_iv{ initialization_vector_ };
			std::size_t padding_bytes = plaintext.size() % AES_BLOCK_SIZE > 0 ? (AES_BLOCK_SIZE - plaintext.size() % AES_BLOCK_SIZE) : 0;
			std::vector<std::uint8_t> ciphertext(plaintext.size() + padding_bytes + 1);

			// Encrypts the plain text.
			AES_cbc_encrypt(plaintext.data(), ciphertext.data(), plaintext.size(), &encyption_key_, subsential_iv.data(), AES_ENCRYPT);
			ciphertext.back() = static_cast<std::uint8_t>(padding_bytes);

			return ciphertext;
		}

		std::vector<std::uint8_t> decrypt(exposing::param_span<const std::uint8_t> ciphertext)
		{
			// The last byte indicates how many bytes are padded.
			if (ciphertext.size() <= 1)
			{
				return std::vector<std::uint8_t>();
			}

			auto subsential_iv{ initialization_vector_ };
			std::size_t padding_bytes = *ciphertext.rbegin();
			std::vector<std::uint8_t> plaintext(ciphertext.size() - 1);

			// Removes padding as required.
			AES_cbc_encrypt(ciphertext.data(), plaintext.data(), ciphertext.size() - 1, &decryption_key_, subsential_iv.data(), AES_DECRYPT);

			return (plaintext.resize(plaintext.size() - padding_bytes), plaintext);
		}
	private:
		AES_KEY encyption_key_;
		AES_KEY decryption_key_;
		std::array<std::uint8_t, iv_size> initialization_vector_;
	};

	aes_provider::aes_provider() : impl_{ std::make_unique<impl>() }
	{
	}

	aes_provider::~aes_provider()
	{
	}

	void aes_provider::set_key_with_iv(exposing::param_span<const std::uint8_t> key, const std::array<std::uint8_t, iv_size>& iv)
	{
		impl_->set_key_with_iv(key, iv);
	}

	void aes_provider::set_key_with_iv(exposing::param_span<const std::uint8_t> key, exposing::param_span<const std::uint8_t> iv)
	{
		impl_->set_key_with_iv(key, iv);
	}

	std::vector<std::uint8_t> aes_provider::encrypt(exposing::param_span<const std::uint8_t> plaintext)
	{
		return impl_->encrypt(plaintext);
	}

	std::vector<std::uint8_t> aes_provider::decrypt(exposing::param_span<const std::uint8_t> ciphertext)
	{
		return impl_->decrypt(ciphertext);
	}
}
