#include "license_database.hpp"

#include <ctime>
#include <cstddef>
#include <string_view>

#include <pqxx/pqxx>

namespace glasssix::license
{
	namespace
	{
		constexpr std::string_view query_get_license{ R"(SELECT id, organization, allowed_device_count, authorized_device_count, extract(epoch from creation_time)::bigint, extract(epoch from expiration_time)::bigint FROM public.licenses WHERE id = $1)" };
		constexpr std::string_view query_get_authorized_devices{ R"(SELECT id, license_id, machine_id, extract(epoch from last_authorization_time)::bigint FROM public.authorized_devices WHERE license_id = $1)" };
		constexpr std::string_view query_get_exact_authorized_devices{ R"(SELECT id, license_id, machine_id, extract(epoch from last_authorization_time)::bigint FROM public.authorized_devices WHERE license_id = $1 AND machine_id = $2)" };
		constexpr std::string_view query_add_or_update_license
		{
			R"(IF EXISTS (SELECT 1 FROM public.licenses WHERE id = $1))
BEGIN
	UPDATE public.licenses SET organization = $2, allowed_device_count = $3, authorized_device_count = $4, creation_time = to_timestamp($5), expiration_time = to_timestamp($6) WHERE id = $1
END
ELSE
BEGIN
	INSERT INTO public.licenses (id, organization, allowed_device_count, authorized_device_count, creation_time, expiration_time) VALUES ($1, $2, $3, $4, to_timestamp($5), to_timestamp($6))
END)"
		};

		constexpr std::string_view query_add_or_update_authorized_device
		{
			R"(IF EXISTS (SELECT 1 FROM public.authorized_devices WHERE license_id = $1))
BEGIN
	UPDATE public.authorized_devices SET machine_id = $2, last_authorization_time = to_timestamp($3) WHERE license_id = $1
END
ELSE
BEGIN
	INSERT INTO public.authorized_devices (id, license_id, machine_id, last_authorization_time) VALUES (uuid_generate_v4(), $1, $2, to_timestamp($3))
END)"
		};
	}

	class license_database::impl
	{
	public:
		impl(std::string_view connection_str) : connection_{ std::string{ connection_str } }
		{
		}

		std::optional<license_record> get_license(const exposing::guid& id)
		{
			pqxx::work work{ connection_ };
			auto records = work.exec_params(std::string{ query_get_license }, exposing::to_string(id));

			return records.empty() ?
				std::nullopt :
				std::optional
				{
					license_record
					{
						exposing::guid{ records.front()["id"].as<std::string_view>() },
						records.front()["organization"].as<std::string>(),
						records.front()["allowed_device_count"].as<std::int32_t>(),
						records.front()["authorized_device_count"].as<std::int32_t>(),
						records.front()["creation_time"].as<std::time_t>(),
						records.front()["expiration_time"].as<std::time_t>()
					}
				};
		}

		std::vector<authorized_device_record> get_authorized_devices(const exposing::guid& license_id)
		{
			pqxx::work work{ connection_ };
			std::vector<authorized_device_record> result;
			auto records = work.exec_params(std::string{ query_get_authorized_devices }, exposing::to_string(license_id));

			for (const auto& item : records)
			{
				auto machine_id = item["machine_id"].as<std::basic_string_view<std::byte>>();
				auto machine_id_u8_byte = reinterpret_cast<const std::uint8_t*>(machine_id.data());

				result.emplace_back(authorized_device_record
					{
						exposing::guid{ item["license_id"].as<std::string_view>() },
						std::vector<std::uint8_t>(machine_id_u8_byte, machine_id_u8_byte + machine_id.size()),
						item["last_authorization_time"].as<std::time_t>()
					});
			}
		}

		std::vector<authorized_device_record> get_exact_authorized_devices(const exposing::guid& license_id, std::string_view machine_id)
		{
			pqxx::work work{ connection_ };
			std::vector<authorized_device_record> result;
			auto records = work.exec_params(std::string{ query_get_exact_authorized_devices }, exposing::to_string(license_id), machine_id);

			for (const auto& item : records)
			{
				auto machine_id = item["machine_id"].as<std::basic_string_view<std::byte>>();
				auto machine_id_u8_byte = reinterpret_cast<const std::uint8_t*>(machine_id.data());

				result.emplace_back(authorized_device_record
					{
						exposing::guid{ item["license_id"].as<std::string_view>() },
						std::vector<std::uint8_t>(machine_id_u8_byte, machine_id_u8_byte + machine_id.size()),
						item["last_authorization_time"].as<std::time_t>()
					});
			}
		}

		void add_or_update_license(const license_record& record)
		{
			pqxx::work work{ connection_ };
			
			work.exec_params(std::string{ query_add_or_update_license }, exposing::to_string(record.id), record.organization, record.allowed_device_count, record.authorized_device_count, record.creation_time, record.expiration_time);
			work.commit();
		}

		void add_or_update_authorized_device(const authorized_device_record& record)
		{
			pqxx::work work{ connection_ };

			work.exec_params(std::string{ query_add_or_update_authorized_device }, exposing::to_string(record.license_id), std::basic_string_view<std::byte>{ reinterpret_cast<const std::byte*>(record.machine_id.data()), record.machine_id.size() }, record.last_authorization_time);
			work.commit();
		}
	private:
		pqxx::connection connection_;
	};

	license_database::license_database(std::string_view connection_str) : impl_{ std::make_unique<impl>(connection_str) }
	{
	}

	license_database::~license_database()
	{
	}

	std::optional<license_record> license_database::get_license(const exposing::guid& id) const
	{
		return impl_->get_license(id);
	}

	std::vector<authorized_device_record> license_database::get_authorized_devices(const exposing::guid& license_id) const
	{
		return impl_->get_authorized_devices(license_id);
	}

	std::vector<authorized_device_record> license_database::get_exact_authorized_devices(const exposing::guid& license_id, std::string_view machine_id) const
	{
		return impl_->get_exact_authorized_devices(license_id, machine_id);
	}

	void license_database::add_or_update_license(const license_record& record) const
	{
		impl_->add_or_update_license(record);
	}

	void license_database::add_or_update_authorized_device(const authorized_device_record& record) const
	{
		impl_->add_or_update_authorized_device(record);
	}
}
