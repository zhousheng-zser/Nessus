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
		void load(exposing::param_span<const std::uint8_t> machine_id, exposing::param_span<const std::uint8_t> user_portrait, std::time_t timestamp) const;
		void load(std::string_view path, exposing::param_span<const std::uint8_t> machine_id) const;
		void save(std::string_view path) const;
		void set_client_data_timestamp(std::time_t timestamp) const;
		void generate(exposing::param_span<const std::uint8_t> machine_id, std::time_t client_data_timestamp) const;
		std::vector<std::uint8_t> user_portrait() const;
		std::vector<std::uint8_t> forward(exposing::param_span<const std::uint8_t> buffer) const;
		std::vector<std::uint8_t> countermarch(exposing::param_span<const std::uint8_t> buffer) const;
	private:
		std::unique_ptr<impl> impl_;
	};
}
