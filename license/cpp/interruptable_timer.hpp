#pragma once

#include <memory>
#include <cstdint>
#include <functional>

namespace glasssix
{
	class interruptable_timer
	{
	public:
		class impl;

		interruptable_timer();
		virtual ~interruptable_timer();
		void start(std::int64_t interval_in_milliseconds, const std::function<void()>& callback) const;
		void stop() const;
	private:
		std::unique_ptr<impl> impl_;
	};
}
