#pragma once

#include "license_record.hpp"
#include "authorized_device_record.hpp"

#include <memory>
#include <vector>
#include <optional>
#include <string_view>

#include <abi/guid.hpp>

namespace glasssix::license
{
	/// <summary>
	/// A utility for accessing records in the database.
	/// </summary>
	class license_database
	{
	public:
		class impl;

		license_database(std::string_view connection_str);
		virtual ~license_database();
		std::optional<license_record> get_license(const exposing::guid& id) const;
		std::vector<authorized_device_record> get_authorized_devices(const exposing::guid& license_id) const;
		std::vector<authorized_device_record> get_exact_authorized_devices(const exposing::guid& license_id, std::string_view machine_id) const;
		void add_or_update_license(const license_record& record) const;
		void add_or_update_authorized_device(const authorized_device_record& record) const;
	private:
		std::unique_ptr<impl> impl_;
	};
}
