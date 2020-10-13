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
		template<typename Duration>
		constexpr Duration abs_duration(Duration&& duration) noexcept
		{
			return Duration{ std::forward<Duration>(duration).count() < 0 ? -std::forward<Duration>(duration).count() : std::forward<Duration>(duration).count() };
		}

		template<typename DestinationTimePoint, typename SourceTimePoint>
		DestinationTimePoint time_point_cast(SourceTimePoint&& time_point)
		{
			return DestinationTimePoint::clock::now() + (std::forward<SourceTimePoint>(time_point) - std::decay_t<SourceTimePoint>::clock::now());
		}
	}

	/// <summary>
	/// Support for a UTC clock.
	/// </summary>
	struct utc_unix_timestamp_clock
	{
		using rep = std::chrono::system_clock::rep;
		using period = std::chrono::system_clock::period;
		using duration = std::chrono::system_clock::duration;
		using time_point = std::chrono::time_point<utc_unix_timestamp_clock>;

		static constexpr bool is_steady = false;

		static time_point now() noexcept;
		static std::time_t to_time_t(const time_point& time) noexcept;
		static time_point from_time_t(std::time_t timestamp) noexcept;
	};

	/// <summary>
	/// Retrieves the current UTC timestamp.
	/// </summary>
	/// <returns>The current UTC timestamp</returns>
	std::time_t get_timestamp();

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
		return utc_unix_timestamp_clock::to_time_t(time_point_cast<utc_unix_timestamp_clock::time_point>(std::forward<TimePoint>(time_point)));
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
		return time_point_cast<TimePoint>(utc_unix_timestamp_clock::from_time_t(timestamp));
	}
}
