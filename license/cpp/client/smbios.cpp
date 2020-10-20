#include "smbios.hpp"
#include "io.hpp"
#include "meta_string.hpp"

#include <new>
#include <string>
#include <variant>
#include <algorithm>
#include <functional>
#include <string_view>

#include <abi/guid.hpp>
#include <abi/meta.hpp>

#ifdef _WIN32
#define NOGDI
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#elif defined(__linux__) && !defined(__ANDROID__)
#include <fstream>
#endif

namespace glasssix::smbios
{
	namespace
	{
#if defined(__linux__) && !defined(__ANDROID__)
		constexpr crypto::meta_string anchor_string_v2{ "_SM_" };
		constexpr crypto::meta_string anchor_string_v3{ "_SM3_" };
		constexpr crypto::meta_string dmi_table{ "/sys/firmware/dmi/tables/DMI" };
		constexpr crypto::meta_string smbios_entry_point{ "/sys/firmware/dmi/tables/smbios_entry_point" };
#endif

		template<typename... Callables> struct alternatives : Callables...
		{
			using Callables::operator()...;
		};

		template<typename... Callables> alternatives(Callables...)->alternatives<Callables...>;

		/// <summary>
		/// Available SMBIOS types.
		/// </summary>
		enum class smbios_data_type : std::uint8_t
		{
			bios_info = 0,
			system_info = 1,
			board_info = 2,
			processor_info = 4
		};

		/// <summary>
		/// SMBIOS entry point 2.x.
		/// </summary>
		struct smbios_entry_point_v2
		{
			std::uint8_t anchor_string[4];
			std::uint8_t checksum;
			std::uint8_t length;
			std::uint8_t major_version;
			std::uint8_t minor_version;
			std::uint16_t max_size;
			std::uint8_t revision;
			std::uint8_t formatted_area[5];
			std::uint8_t intermediate_anchor_string[5];
			std::uint8_t intermediate_checksum;
			std::uint16_t table_length;
			std::uint32_t table_address;
			std::uint16_t number_of_structures;
			std::uint8_t bcd_revision;
			std::uint8_t dummy_padding;
		};

		/// <summary>
		/// SMBIOS entry point 3.x.
		/// </summary>
		struct smbios_entry_point_v3
		{
			std::uint8_t anchor_string[5];
			std::uint8_t checksum;
			std::uint8_t length;
			std::uint8_t major_version;
			std::uint8_t minor_version;
			std::uint8_t decrev;
			std::uint8_t revision;
			std::uint8_t reserved;
			std::uint32_t max_table_size;
			std::uint64_t table_address;
		};

		struct win32_raw_smbios_data
		{
			std::uint8_t used_20_calling_method;
			std::uint8_t smbios_major_version;
			std::uint8_t smbios_minor_version;
			std::uint8_t dmi_revision;
			std::uint32_t length;
			std::uint8_t smbios_table_data[1];
		};

		struct smbios_header
		{
			smbios_data_type type;
			std::uint8_t length;
			std::uint16_t handle;
		};

		struct raw_smbios_bios_info
		{
			smbios_header header;
			std::uint8_t vendor;
			std::uint8_t version;
			std::uint16_t starting_addr_seg;
			std::uint8_t release_date;
			std::uint8_t rom_size;
			std::uint64_t characteristics;
			std::uint8_t extension[2];
			std::uint8_t major_release;
			std::uint8_t minor_release;
			std::uint8_t ec_firmware_major;
			std::uint8_t ec_firmware_minor;
		};

		struct raw_smbios_system_info
		{
			smbios_header header;
			std::uint8_t manufacturer;
			std::uint8_t product_name;
			std::uint8_t version;
			std::uint8_t serial_number;
			std::uint8_t uuid[16];
			std::uint8_t wake_up_type;
			std::uint8_t sku_number;
			std::uint8_t family;
		};

		struct raw_smbios_board_info
		{
			smbios_header header;
			std::uint8_t manufacturer;
			std::uint8_t product;
			std::uint8_t version;
			std::uint8_t serial_number;
			std::uint8_t asset_tag;
			std::uint8_t feature_flags;
			std::uint8_t location_in_chassis;
			std::uint16_t chassis_handle;
			std::uint8_t type;
			std::uint8_t num_obj_handle;
			std::uint16_t* obj_handle;
		};

		struct raw_smbios_processor_info
		{
			std::uint8_t socket_designation;
			std::uint8_t type;
			std::uint8_t family;
			std::uint8_t manufacturer;
			std::uint64_t processor_id;
			std::uint8_t version;
			std::uint8_t voltage;
			std::uint16_t external_clock;
			std::uint16_t max_speed;
			std::uint16_t current_speed;
			std::uint8_t status;
			std::uint8_t upgrade;
			std::uint16_t l1_cache_handle;
			std::uint16_t l2_cache_handle;
			std::uint16_t l3_cache_handle;
			std::uint8_t serial_number;
			std::uint8_t assert_tag;
			std::uint8_t part_number;
			std::uint8_t core_count;
			std::uint8_t core_enabled;
			std::uint8_t thread_count;
			std::uint16_t characteristics;
			std::uint16_t family_2;
		};

		std::string_view locate_string(const char* buffer, std::size_t index)
		{
			static constexpr char empty_char = '\0';

			if (buffer == nullptr || index == 0 || *buffer == empty_char)
			{
				return &empty_char;
			}

			while (--index != 0)
			{
				buffer += (std::strlen(buffer) + 1);
			}

			return buffer;
		}

		void parse_smbios_bios_info(const smbios_header& header, const char* buffer, smbios_info& info)
		{
			// A type in standard layout can be casted to its first member.
			auto& bios_info = info.bios;
			auto& raw_bios_info = reinterpret_cast<const raw_smbios_bios_info&>(header);

			bios_info.rom_size = raw_bios_info.rom_size;
			bios_info.major_release = raw_bios_info.major_release;
			bios_info.minor_release = raw_bios_info.minor_release;
			bios_info.ec_firmware_major = raw_bios_info.ec_firmware_major;
			bios_info.ec_firmware_minor = raw_bios_info.ec_firmware_minor;
			bios_info.vendor = locate_string(buffer, raw_bios_info.vendor);
			bios_info.version = locate_string(buffer, raw_bios_info.version);
			bios_info.vendor = locate_string(buffer, raw_bios_info.vendor);

		}

		void parse_smbios_system_info(const smbios_header& header, const char* buffer, smbios_info& info)
		{
			// A type in standard layout can be casted to its first member.
			auto& system_info = info.system;
			auto& raw_system_info = reinterpret_cast<const raw_smbios_system_info&>(header);

			system_info.wake_up_type = raw_system_info.wake_up_type;
			system_info.family = locate_string(buffer, raw_system_info.family);
			system_info.version = locate_string(buffer, raw_system_info.version);
			system_info.sku_number = locate_string(buffer, raw_system_info.sku_number);
			system_info.product_name = locate_string(buffer, raw_system_info.product_name);
			system_info.manufacturer = locate_string(buffer, raw_system_info.manufacturer);
			system_info.serial_number = locate_string(buffer, raw_system_info.serial_number);

			using namespace exposing;

			auto uuid_bytes{ meta::apply_index_sequence<sizeof(raw_smbios_system_info::uuid)>([&](auto... indexes) { return std::array{ raw_system_info.uuid[indexes]... }; }) };
			guid uuid
			{
				meta::make_number<std::uint32_t>(meta::sub_array<0, 4>::get(uuid_bytes)),
				meta::make_number<std::uint16_t>(meta::sub_array<4, 2>::get(uuid_bytes)),
				meta::make_number<std::uint16_t>(meta::sub_array<6, 2>::get(uuid_bytes)),
				meta::sub_array<8, 8>::get(uuid_bytes)
			};
			auto uuid_chars = to_char_array(uuid);

			system_info.uuid.assign(uuid_chars.data(), uuid_chars.size());
		}

		void parse_smbios_board_info(const smbios_header& header, const char* buffer, smbios_info& info)
		{
			// A type in standard layout can be casted to its first member.
			auto& board_info = info.board;
			auto& raw_board_info = reinterpret_cast<const raw_smbios_board_info&>(header);

			board_info.type = raw_board_info.type;
			board_info.product = locate_string(buffer, raw_board_info.product);
			board_info.version = locate_string(buffer, raw_board_info.version);
			board_info.assert_tag = locate_string(buffer, raw_board_info.asset_tag);
			board_info.manufacturer = locate_string(buffer, raw_board_info.manufacturer);
			board_info.serial_number = locate_string(buffer, raw_board_info.serial_number);
			board_info.location_in_chassis = locate_string(buffer, raw_board_info.location_in_chassis);
		}

		void parse_smbios_processor_info(const smbios_header& header, const char* buffer, smbios_info& info)
		{
			// A type in standard layout can be casted to its first member.
			auto& processor_info = info.processor;
			auto& raw_processor_info = reinterpret_cast<const raw_smbios_processor_info&>(header);

			processor_info.type = raw_processor_info.type;
			processor_info.family = raw_processor_info.family;
			processor_info.status = raw_processor_info.status;
			processor_info.upgrade = raw_processor_info.upgrade;
			processor_info.voltage = raw_processor_info.voltage;
			processor_info.core_count = raw_processor_info.core_count;
			processor_info.core_enabled = raw_processor_info.core_enabled;
			processor_info.thread_count = raw_processor_info.thread_count;
			processor_info.family_2 = raw_processor_info.family_2;
			processor_info.external_clock = raw_processor_info.external_clock;
			processor_info.max_speed = raw_processor_info.max_speed;
			processor_info.current_speed = raw_processor_info.current_speed;
			processor_info.characteristics = raw_processor_info.characteristics;
			processor_info.processor_id = raw_processor_info.processor_id;
			processor_info.version = locate_string(buffer, raw_processor_info.version);
			processor_info.manufacturer = locate_string(buffer, raw_processor_info.manufacturer);
			processor_info.socket_designation = locate_string(buffer, raw_processor_info.socket_designation);
			processor_info.serial_number = locate_string(buffer, raw_processor_info.serial_number);
			processor_info.assert_tag = locate_string(buffer, raw_processor_info.assert_tag);
			processor_info.part_number = locate_string(buffer, raw_processor_info.part_number);
		}

		smbios_info parse_smbios_data(const std::uint8_t* raw_data, std::size_t size)
		{
			static constexpr std::array dispatcher_table
			{
				std::pair{ smbios_data_type::bios_info, &parse_smbios_bios_info },
				std::pair{ smbios_data_type::system_info, &parse_smbios_system_info },
				std::pair{ smbios_data_type::board_info, &parse_smbios_board_info },
				std::pair{ smbios_data_type::processor_info, &parse_smbios_processor_info }
			};

			smbios_info result;

			for (auto ptr = raw_data, end_ptr = ptr + size; ptr < end_ptr; ptr += 2)
			{
				auto& header = *std::launder(reinterpret_cast<const smbios_header*>(ptr));
				auto buffer = reinterpret_cast<const char*>(&header) + header.length;

				if (auto iter = std::find_if(dispatcher_table.begin(), dispatcher_table.end(), [&](const auto& inner) { return inner.first == header.type; }); iter != dispatcher_table.end())
				{
					iter->second(header, buffer, result);
				}

				// Skips the header.
				ptr += header.length;

				// Skips the string area and locates the string terminating flags '0000h'.
				while ((*ptr | *(ptr + 1)) != 0)
				{
					ptr++;
				}
			}

			return result;
		}

#if defined(__linux__) && !defined(__ANDROID__)
		/// <summary>
		/// Try to get the SMBIOS entry point.
		/// </summary>
		/// <returns>The entry point</returns>
		std::optional<std::variant<smbios_entry_point_v2, smbios_entry_point_v3>> get_smbios_entry_point()
		{
			auto entry_point_buffer = io::read_all_bytes(smbios_entry_point.decrypt_as_string());

			if (!entry_point_buffer || entry_point_buffer->size() < std::max(anchor_string_v2.size(), anchor_string_v3.size()))
			{
				return std::nullopt;
			}

			std::array<std::uint8_t, anchor_string_v2.size()> assuming_anchor_string_v2;
			std::array<std::uint8_t, anchor_string_v3.size()> assuming_anchor_string_v3;

			std::copy(entry_point_buffer->begin(), entry_point_buffer->begin() + anchor_string_v2.size(), assuming_anchor_string_v2.begin());
			std::copy(entry_point_buffer->begin(), entry_point_buffer->begin() + anchor_string_v3.size(), assuming_anchor_string_v3.begin());

			if (assuming_anchor_string_v2 == anchor_string_v2.decrypt_as_bytes())
			{
				smbios_entry_point_v2 result{};

				return (std::copy(entry_point_buffer->begin(), entry_point_buffer->end(), reinterpret_cast<std::uint8_t*>(&result)), result);
			}

			if (assuming_anchor_string_v3 == anchor_string_v3.decrypt_as_bytes())
			{
				smbios_entry_point_v3 result{};

				return (std::copy(entry_point_buffer->begin(), entry_point_buffer->end(), reinterpret_cast<std::uint8_t*>(&result)), result);
			}

			return std::nullopt;
		}
#endif
	}

	smbios_unsupported_version::smbios_unsupported_version() : smbios_error{ "The current SMBIOS version is too low. The minimum version should be 2.6." }
	{
	}

#ifdef _WIN32
	std::optional<smbios_info> read_smbios_info()
	{
		static constexpr std::uint32_t signature = 'RSMB';

		std::uint32_t size = GetSystemFirmwareTable(signature, 0, nullptr, 0);

		if (size == 0)
		{
			return std::nullopt;
		}

		auto buffer = new std::uint8_t[size];
		auto raw_data = new (buffer) win32_raw_smbios_data;
		std::uint16_t version = (static_cast<std::uint16_t>(raw_data->smbios_major_version) << 8) + raw_data->smbios_minor_version;

		return version >= 0x0206 ?
			GetSystemFirmwareTable(signature, 0, raw_data, size) == size ? std::optional{ parse_smbios_data(raw_data->smbios_table_data, raw_data->length) } : std::nullopt :
			throw smbios_unsupported_version{};
	}
#elif defined(__linux__) && !defined(__ANDROID__)
	std::optional<smbios_info> read_smbios_info()
	{
		// Reads the entry point data.
		auto entry_point = get_smbios_entry_point();

		if (!entry_point)
		{
			return std::nullopt;
		}

		std::uint16_t version = std::visit([](auto&& inner) { return static_cast<std::uint16_t>(std::forward<decltype(inner)>(inner).major_version << 8) + std::forward<decltype(inner)>(inner).minor_version; }, *entry_point);

		if (version < 0x0206)
		{
			throw smbios_unsupported_version{};
		}

		// Parses the DMI data.
		auto dmi_data = io::read_all_bytes(dmi_table.decrypt_as_string());

		return dmi_data ? std::optional{ parse_smbios_data(dmi_data->data(), dmi_data->size()) } : std::nullopt;
	}
#endif
}
