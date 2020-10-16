#pragma once

#include <stdexcept>

namespace glasssix::crypto
{
	struct crypto_error : std::runtime_error
	{
		using runtime_error::runtime_error;
	};
}
