#pragma once

#include <array>
#include <limits>
#include <utility>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace glasssix::crypto
{
	namespace details
	{
		constexpr int digit_to_int32(char c)
		{
			return c - '0';
		};

		inline constexpr int random_seed = digit_to_int32(__TIME__[7]) +
			digit_to_int32(__TIME__[6]) * 10 +
			digit_to_int32(__TIME__[4]) * 60 +
			digit_to_int32(__TIME__[3]) * 600 +
			digit_to_int32(__TIME__[1]) * 3600 +
			digit_to_int32(__TIME__[0]) * 36000;

		template<std::uint32_t A, std::uint32_t C, std::uint32_t Seed, std::uint32_t Limit>
		struct linear_congruential_engine
		{
			static constexpr std::uint32_t value = (A * linear_congruential_engine<A, C - 1, Seed, Limit>::value + C) % Limit;
		};

		template<std::uint32_t A, std::uint32_t Seed, std::uint32_t Limit>
		struct linear_congruential_engine<A, 0, Seed, Limit>
		{
			static constexpr std::uint32_t value = (A * Seed) % Limit;
		};

		template <int N, int Limit>
		struct meta_random
		{
			static constexpr std::uint32_t value = details::linear_congruential_engine<16807, N, details::random_seed, Limit>::value;
		};

		template<int A, int B>
		struct extended_euclidian
		{
			static constexpr int d = extended_euclidian<B, A% B>::d;
			static constexpr int x = extended_euclidian<B, A% B>::y;
			static constexpr int y = extended_euclidian<B, A% B>::x - (A / B) * extended_euclidian<B, A% B>::y;
		};

		template <int A>
		struct extended_euclidian<A, 0>
		{
			static constexpr int d = A;
			static constexpr int x = 1;
			static constexpr int y = 0;
		};

		constexpr int positive_modulo(int a, int n)
		{
			return (a % n + n) % n;
		}

		inline constexpr std::array<int, 30> prime_numbers
		{
			2, 3, 5, 7, 11, 13, 17, 19, 23, 29,
			31, 37, 41, 43, 47, 53, 59, 61, 67,
			71, 73, 79, 83, 89, 97, 101, 103,
			107, 109, 113
		};
	}

	template<typename Indexes, int A, int B>
	class meta_string;

	template<std::size_t... Indexes, int A, int B>
	class meta_string<std::index_sequence<Indexes...>, A, B>
	{
	public:
		static constexpr std::size_t buffer_size = sizeof...(Indexes);

		constexpr meta_string(std::string_view str) : encrypted_buffer_ { encrypt(str[Indexes])... }
		{
		}

		constexpr std::size_t size() const
		{
			return encrypted_buffer_.size();
		}

		constexpr auto decrypt() const
		{
			std::array<char, sizeof...(Indexes)> result{};

			for (std::size_t i = 0; i < sizeof...(Indexes); i++)
			{
				result[i] = decrypt(encrypted_buffer_[i]);
			}

			return result;
		}

		std::string decrypt_as_string() const
		{
			auto chars = decrypt();

			return std::string{ chars.begin(), chars.end() };
		}
	private:
		static constexpr int encrypt(char c)
		{
			return (A * c + B) % 127;
		}

		static constexpr char decrypt(int c)
		{
			return details::positive_modulo(details::extended_euclidian<127, A>::y * (c - B), 127);
		}

		std::array<int, sizeof...(Indexes)> encrypted_buffer_;
	};

	template<std::size_t Size>
	meta_string(const char(&str)[Size]) -> meta_string<std::make_index_sequence<Size - 1>, details::prime_numbers[details::meta_random<__COUNTER__, details::prime_numbers.size()>::value], details::meta_random<__COUNTER__, 126>::value>;
}
