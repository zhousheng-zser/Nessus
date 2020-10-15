#pragma once

#include <ctime>
#include <chrono>
#include <utility>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace glasssix
{
	namespace details
	{
		template<typename DestinationTimePoint, typename SourceTimePoint>
		DestinationTimePoint time_point_cast(SourceTimePoint&& time_point)
		{
			return DestinationTimePoint::clock::now() + (std::forward<SourceTimePoint>(time_point) - std::decay_t<SourceTimePoint>::clock::now());
		}
	}

	/// <summary>
	/// Retrieves the current local timestamp.
	/// </summary>
	/// <returns>The current local timestamp</returns>
	std::time_t get_local_timestamp();

	/// <summary>
	/// Converts a time point to another time point.
	/// </summary>
	/// <typeparam name="DestinationTimePoint">The destination type</typeparam>
	/// <typeparam name="SourceTimePoint">The source type</typeparam>
	/// <param name="time_point">The time point</param>
	/// <param name="tolerance">The tolerance</param>
	/// <param name="limit">The limit of iterations</param>
	/// <returns></returns>
	template<typename DestinationTimePoint, typename SourceTimePoint>
	DestinationTimePoint time_point_cast(SourceTimePoint&& time_point, const typename SourceTimePoint::duration& tolerance = typename SourceTimePoint::duration{ 100 }, std::size_t limit = 5)
	{
		std::size_t index = 0;
		DestinationTimePoint current;
		auto epsilon = SourceTimePoint::duration::max();

		do
		{
			auto destination = details::time_point_cast<DestinationTimePoint>(std::forward<SourceTimePoint>(time_point));
			auto source = details::time_point_cast<SourceTimePoint>(destination);
			auto delta = std::chrono::abs(source - std::forward<SourceTimePoint>(time_point));
			
			if (delta < epsilon)
			{
				current = destination;
				epsilon = delta;
			}

		} while (++index < limit && epsilon > tolerance);

		return current;
	}

	/// <summary>
	/// Converts a time point to time_t.
	/// </summary>
	/// <typeparam name="TimePoint">The time point type</typeparam>
	/// <param name="time_point">The time point</param>
	/// <returns>The timestamp</returns>
	template<typename TimePoint>
	std::time_t to_time_t(TimePoint&& time_point)
	{
		return std::chrono::system_clock::to_time_t(details::time_point_cast<std::chrono::system_clock::time_point>(std::forward<TimePoint>(time_point)));
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
		return time_point_cast<TimePoint>(std::chrono::system_clock::from_time_t(timestamp));
	}
}
