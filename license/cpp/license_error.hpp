#pragma once

#include <stdexcept>

namespace glasssix::license
{
	struct license_error : std::runtime_error
	{
		using runtime_error::runtime_error;
	};
}
