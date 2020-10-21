#include "license_database.hpp"
#include "license_error.hpp"

#include <ctime>
#include <cstddef>
#include <string_view>

#include <pqxx/pqxx>

namespace glasssix::license
{
	namespace
	{
		constexpr std::string_view query_get_license{ R"(SELECT id, organization, allowed_device_count, authorized_device_count, EXTRACT(EPOCH FROM creation_time)::bigint as creation_time, EXTRACT(EPOCH FROM expiration_time)::bigint as expiration_time FROM public.licenses WHERE id = $1)" };
		constexpr std::string_view query_get_authorized_devices{ R"(SELECT license_id, machine_id, EXTRACT(EPOCH FROM last_authorization_time)::bigint as last_authorization_time FROM public.authorized_devices WHERE license_id = $1)" };
		constexpr std::string_view query_get_exact_authorized_devices{ R"(SELECT license_id, machine_id, EXTRACT(EPOCH FROM last_authorization_time)::bigint as last_authorization_time FROM public.authorized_devices WHERE license_id = $1 AND machine_id = $2)" };
		constexpr std::string_view query_add_or_update_license{ R"(INSERT INTO public.licenses (id, organization, allowed_device_count, authorized_device_count, creation_time, expiration_time) VALUES ($1, $2, $3, $4, TO_TIMESTAMP($5), TO_TIMESTAMP($6)) ON CONFLICT(id) DO UPDATE SET organization = $2, allowed_device_count = $3, authorized_device_count = $4, creation_time = TO_TIMESTAMP($5), expiration_time = TO_TIMESTAMP($6))" };
		constexpr std::string_view query_add_or_update_authorized_devices{ R"(INSERT INTO public.authorized_devices (license_id, machine_id, last_authorization_time) VALUES ($1, $2, TO_TIMESTAMP($3)) ON CONFLICT(license_id, machine_id) DO UPDATE SET last_authorization_time = TO_TIMESTAMP($3))" };
	}

	class license_database::impl
	{
	public:
		impl(std::string_view connection_str) : connection_{ std::string{ connection_str } }
		{
#ifdef _WIN32
			connection_.set_client_encoding("gb18030");
#endif

			if (!connection_.is_open())
			{
				throw license_error{ "Failed to connect the database." };
			}
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

			return result;
		}

		std::vector<authorized_device_record> get_exact_authorized_devices(const exposing::guid& license_id, exposing::param_span<const std::uint8_t> machine_id)
		{
			pqxx::work work{ connection_ };
			std::vector<authorized_device_record> result;
			auto records = work.exec_params(std::string{ query_get_exact_authorized_devices }, exposing::to_string(license_id), std::basic_string_view<std::byte>{ reinterpret_cast<const std::byte*>(machine_id.data()), machine_id.size() });

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

			return result;
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

			work.exec_params(std::string{ query_add_or_update_authorized_devices }, exposing::to_string(record.license_id), std::basic_string_view<std::byte>{ reinterpret_cast<const std::byte*>(record.machine_id.data()), record.machine_id.size() }, record.last_authorization_time);
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

	std::vector<authorized_device_record> license_database::get_exact_authorized_devices(const exposing::guid& license_id, exposing::param_span<const std::uint8_t> machine_id) const
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
