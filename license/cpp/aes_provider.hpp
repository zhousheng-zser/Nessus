#pragma once

#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <string_view>

#include <abi/param_span.hpp>

namespace glasssix::crypto
{
	/// <summary>
	/// A standard AES cryptographic provider.
	/// </summary>
	class aes_provider
	{
	public:
		class impl;

		static constexpr std::size_t iv_size = 16;
		static constexpr std::size_t block_size = 16;

		aes_provider();
		virtual ~aes_provider();

		/// <summary>
		/// Sets a new AES key.
		/// </summary>
		/// <param name="key">The key</param>
		void set_key(exposing::param_span<const std::uint8_t> key);

		/// <summary>
		/// Sets a new IV.
		/// </summary>
		/// <param name="iv">The IV</param>
		void set_iv(const std::array<std::uint8_t, iv_size>& iv);

		/// <summary>
		/// Sets a new IV.
		/// </summary>
		/// <param name="iv">The IV</param>
		void set_iv(exposing::param_span<const std::uint8_t> iv);

		/// <summary>
		/// Encrypts a plaintext.
		/// </summary>
		/// <param name="plaintext">The plaintext to encrypt</param>
		/// <returns>The ciphertext</returns>
		std::vector<std::uint8_t> encrypt(exposing::param_span<const std::uint8_t> plaintext);

		/// <summary>
		/// Decrypts a ciphertext.
		/// </summary>
		/// <param name="ciphertext">The ciphertext to deceypt</param>
		/// <returns>The plaintext</returns>
		std::vector<std::uint8_t> decrypt(exposing::param_span<const std::uint8_t> ciphertext);
	private:
		std::unique_ptr<impl> impl_;
	};
}
