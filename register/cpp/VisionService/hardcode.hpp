#pragma once

#include <abi/param_span.hpp>
#include <abi/param_string.hpp>

namespace glasssix::exposing::nessus
{
	param_span<const param_string> get_hardcode_model_params(utf8_string_view name);
}
