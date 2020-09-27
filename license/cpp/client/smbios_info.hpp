#pragma once

#include <string>
#include <cstdint>

namespace glasssix::smbios
{
	enum class smbios_data_type : std::uint8_t
	{
		bios_info = 0,
		system_info = 1,
		board_info = 2,
		processor_info = 4
	};

	struct smbios_bios_info
	{
		std::string vendor;
		std::string version;
		std::string release_date;
		std::uint8_t rom_size;
		std::uint8_t major_release;
		std::uint8_t minor_release;
		std::uint8_t ec_firmware_major;
		std::uint8_t ec_firmware_minor;
		std::uint64_t characteristics;
	};

	struct smbios_system_info
	{
		std::string uuid;
		std::string family;
		std::string version;
		std::string sku_number;
		std::string product_name;
		std::string manufacturer;
		std::string serial_number;
		std::uint8_t wake_up_type;
	};

	struct smbios_board_info
	{
		std::uint8_t type;
		std::string product;
		std::string version;
		std::string assert_tag;
		std::string manufacturer;
		std::string serial_number;
		std::string location_in_chassis;
		std::uint8_t feature_flags;
	};

	struct smbios_processor_info
	{
		std::string version;
		std::string manufacturer;
		std::string socket_designation;
		std::string serial_number;
		std::string assert_tag;
		std::string part_number;
		std::uint8_t type;
		std::uint8_t family;
		std::uint8_t status;
		std::uint8_t upgrade;
		std::uint8_t voltage;
		std::uint8_t core_count;
		std::uint8_t core_enabled;
		std::uint8_t thread_count;
		std::uint16_t family_2;
		std::uint16_t external_clock;
		std::uint16_t max_speed;
		std::uint16_t current_speed;
		std::uint16_t characteristics;
		std::uint64_t processor_id;
	};

	struct smbios_info
	{
		smbios_bios_info bios;
		smbios_board_info board;
		smbios_system_info system;
		smbios_processor_info processor;
	};
}
