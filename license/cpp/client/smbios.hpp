#pragma once

#include "smbios_info.hpp"

#include <optional>
#include <exception>

namespace glasssix::smbios
{
	struct smbios_error : std::exception
	{
		using exception::exception;
	};

	struct smbios_unsupported_version : smbios_error
	{
		smbios_unsupported_version();
	};

	/// <summary>
	/// Reads the SMBIOS inforamtion.
	/// </summary>
	/// <returns>The SMBIOS information</returns>
	std::optional<smbios_info> read_smbios_info();
}
