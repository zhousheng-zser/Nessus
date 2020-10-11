#include "interruptable_timer.hpp"

#include <mutex>
#include <atomic>
#include <chrono>
#include <thread>
#include <utility>
#include <optional>
#include <condition_variable>

namespace glasssix
{
	namespace
	{
		struct timer_info
		{
			std::function<void()> callback;
			std::chrono::milliseconds interval;

			template<typename Callback, typename Interval>
			timer_info(Callback&& callback, Interval&& interval) : callback{ std::forward<Callback>(callback) }, interval{ std::forward<Interval>(interval) }
			{
			}

			explicit operator bool() const noexcept
			{
				return callback && interval > std::chrono::milliseconds::zero();
			}
		};
	}

	class interruptable_timer::impl
	{
	public:
		impl() : idie_running_{ true }, idie_pending_{ true }, timer_running_{ false }, global_running_{ true }, worker_thread_{ std::bind(&impl::timer_routine, this) }
		{
		}

		~impl()
		{
			change_state(nullptr, false, false);
			global_running_.store(false, std::memory_order_release);

			if (worker_thread_.joinable())
			{
				worker_thread_.join();
			}
		}

		void start(std::int64_t interval_in_milliseconds, const std::function<void()>& callback)
		{
			stop();
			change_state(std::make_shared<timer_info>(callback, std::chrono::milliseconds{ interval_in_milliseconds }), true, false);
		}

		void stop()
		{
			change_state(nullptr, false, true);

			std::unique_lock lock{ mutex_idle_ };

			cond_idle_.wait(lock, [this] { return idie_running_.load(std::memory_order_acquire); });
		}
	private:
		void change_state(const std::shared_ptr<timer_info>& new_state, bool timer_running, bool idle_running)
		{
			std::atomic_store_explicit(&timer_info_, new_state, std::memory_order_release);
			timer_running_.store(timer_running, std::memory_order_release);
			idie_pending_.store(idle_running, std::memory_order_release);
			cond_idle_.notify_all();
			cond_timer_.notify_all();
		}

		void timer_routine()
		{
			// Keeps the worker thread running.
			while (global_running_.load(std::memory_order_acquire))
			{
				{
					std::unique_lock lock{ mutex_idle_ };

					cond_idle_.notify_all();
					cond_idle_.wait(lock, [this] { return !idie_pending_.load(std::memory_order_acquire); });
				}

				idie_running_.store(false, std::memory_order_release);
				idie_pending_.store(false, std::memory_order_release);

				if (auto info = std::atomic_load_explicit(&timer_info_, std::memory_order_acquire); info && *info)
				{
					while (timer_running_.load(std::memory_order_acquire))
					{
						info->callback();

						std::unique_lock lock{ mutex_timer_ };

						cond_timer_.wait_for(lock, info->interval, [this] { return !timer_running_.load(std::memory_order_acquire); });
					}
				}

				idie_running_.store(true, std::memory_order_release);
				idie_pending_.store(false, std::memory_order_release);
			}
		}

		std::mutex mutex_idle_;
		std::mutex mutex_timer_;
		std::thread worker_thread_;
		std::atomic_bool idie_running_;
		std::atomic_bool idie_pending_;
		std::atomic_bool timer_running_;
		std::atomic_bool global_running_;
		std::condition_variable cond_idle_;
		std::condition_variable cond_timer_;
		std::shared_ptr<timer_info> timer_info_;
	};

	interruptable_timer::interruptable_timer() : impl_{ std::make_unique<impl>() }
	{

	}

	interruptable_timer::~interruptable_timer()
	{
	}

	void interruptable_timer::start(std::int64_t interval_in_milliseconds, const std::function<void()>& callback) const
	{
		impl_->start(interval_in_milliseconds, callback);
	}

	void interruptable_timer::stop() const
	{
		impl_->stop();
	}
}
