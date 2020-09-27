#pragma once

#include <ctime>
#include <chrono>
#include <utility>
#include <type_traits>

namespace glasssix::time_utils
{
	/// <summary>
	/// Retrieves the current timestamp.
	/// </summary>
	/// <returns>The current timestamp</returns>
	inline std::time_t get_timestamp()
	{
		return std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	}

	/// <summary>
	/// Converts a time point to time_t.
	/// </summary>
	/// <typeparam name="TimePoint">The time point type</typeparam>
	/// <param name="time">The time point</param>
	/// <returns>The timestamp</returns>
	template<typename TimePoint>
	std::time_t to_time_t(TimePoint&& time)
	{
		auto duration = std::forward<TimePoint>(time) - std::decay_t<TimePoint>::clock::now() + std::chrono::system_clock::now();
		
		return std::chrono::system_clock::to_time_t(std::chrono::time_point_cast<std::chrono::system_clock::duration>(duration));
	}

	/// <summary>
	/// Converts a time_t to a time point.
	/// </summary>
	/// <typeparam name="TimePoint">The time point type</typeparam>
	/// <param name="timestamp">The timestamp</param>
	/// <returns>The time point</returns>
	template<typename TimePoint>
	TimePoint from_time_t(std::time_t timestamp)
	{
		auto duration = std::chrono::time_point_cast<typename TimePoint::clock::duration>(std::chrono::system_clock::from_time_t(timestamp));
		
		return duration - std::chrono::system_clock::now() + TimePoint::clock::now();
	}
}
