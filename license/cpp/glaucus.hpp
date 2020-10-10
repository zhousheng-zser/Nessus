#pragma once 

#include <ctime>
#include <memory>
#include <vector>
#include <cstdint>
#include <string_view>

#include <abi/param_span.hpp>

namespace glasssix::crypto
{
	/// <summary>
	/// Provides a user portrait and cryptography facilities.
	/// </summary>
	class glaucus
	{
	public:
		class impl;

		glaucus();
		virtual ~glaucus();
		void load(std::string_view path, exposing::param_span<std::uint8_t> machine_id);
		void save(std::string_view path);
		void set_client_data_timestamp(std::time_t timestamp);
		void generate(exposing::param_span<const std::uint8_t> machine_id, std::time_t timestamp);
		std::vector<std::uint8_t> user_portrait() const;
		std::vector<std::uint8_t> forward(exposing::param_span<const std::uint8_t> buffer) const;
		std::vector<std::uint8_t> countermarch(exposing::param_span<const std::uint8_t> buffer) const;
	private:
		std::unique_ptr<impl> impl_;
	};
}
