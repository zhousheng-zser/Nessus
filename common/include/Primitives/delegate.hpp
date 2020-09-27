#pragma once

#include <atomic>
#include <vector>
#include <stdexcept>
#include <functional>

namespace glasssix
{
    class delegate_token
    {
    public:
        delegate_token() : id_{}
        {
        }

        delegate_token(const std::function<void(uint64_t)>& remove_handler, uint64_t id, const std::weak_ptr<void>& observer) : remove_handler_{ remove_handler }, id_{ id }, observer_{ observer }
        {
        }

        delegate_token(const delegate_token& other) = delete;
        delegate_token(delegate_token&& other)
        {
            *this = std::move(other);
        }

        virtual ~delegate_token()
        {
            clean();
        }

        delegate_token& operator=(const delegate_token& right) = delete;
        delegate_token& operator=(delegate_token&& right)
        {
            clean();
            id_ = std::move(right.id_);
            observer_ = std::move(right.observer_);
            remove_handler_ = std::move(right.remove_handler_);

            return *this;
        }

        void clean()
        {
            // We assume that the object keeps alive with the observer.
            auto observer = observer_.lock();

            if (observer && remove_handler_)
            {
                remove_handler_(id_);
            }
        }
    private:
        uint64_t id_;
        std::weak_ptr<void> observer_;
        std::function<void(uint64_t)> remove_handler_;
    };
    
    template<typename TResult, typename... TArgs>
    class simple_delegate
    {
    public:
        using function_type = std::function<TResult(TArgs...)>;
    public:
        simple_delegate() : lifetime_observer_{ std::make_shared<int>() }
        {
            update_readable_buffer_core();
        }

        virtual ~simple_delegate() = default;

        auto& operator+=(function_type&& func)
        {
            add_listener_core(std::move(func));

            return *this;
        }

        auto& operator+=(const function_type& func)
        {
            add_listener_core(func);

            return *this;
        }

        auto& operator+=(const simple_delegate<TResult, TArgs...>& right)
        {
            for (auto&[id, func] : right.listeners_)
            {
                add_listener_core(func);
            }

            return *this;
        }

		template<typename... TParameters>
		TResult operator()(TParameters&&... args)
        {
            // Fetch the buffer automically.
            std::shared_ptr<function_type[]> buffer = std::atomic_load(&readable_buffer_);

            // A specialization for 'void'.
            if constexpr (std::is_void_v<TResult>)
            {
                for (auto ptr = buffer.get(); *ptr; ptr++)
                {
                    (*ptr)(std::forward<TParameters>(args)...);
                }
            }
            else
            {
                // Initialize the default value.
                TResult result{};

                for (auto ptr = buffer.get(); *ptr; ptr++)
                {
                    result = (*ptr)(std::forward<TParameters>(args)...);
                }

                return result;
            }
        }

        auto add_listener_auto_removal(const function_type& func)
        {
            auto id = add_listener_core(func);

            return std::make_shared<delegate_token>(std::bind(&simple_delegate::remove_listener, this, std::placeholders::_1), id, lifetime_observer_);
        }

        auto add_listener_auto_removal(function_type&& func)
        {
            auto id = add_listener_core(func);

            return std::make_shared<delegate_token>(std::bind(&simple_delegate::remove_listener, this, std::placeholders::_1), id, lifetime_observer_);
        }

        void remove_listener(uint64_t id)
        {
            auto item = std::find_if(listeners_.begin(), listeners_.end(), [&](const auto& item)
            {
                return item.first == id;
            });

            if (item != listeners_.end())
            {
                listeners_.erase(item);

                // Create a new readable buffer atomically.
                update_readable_buffer_core();
            }
        }
    private:
        template<typename TFunctor>
        auto add_listener_core(TFunctor&& func)
        {
            if (!std::forward<TFunctor>(func))
            {
                throw std::runtime_error{ "The listener functor cannot be empty." };
            }
            
            auto&[id, handler] = listeners_.emplace_back(global_counter_++, std::forward<TFunctor>(func));

            // Create a new readable buffer atomically.
            update_readable_buffer_core();

            return id;
        }

        void update_readable_buffer_core()
        {
            std::shared_ptr<function_type[]> buffer{ new function_type[listeners_.size() + 1], [](function_type* inner) { delete[] inner; } };

            size_t index = 0;
            for (auto&[id, handler] : listeners_)
            {
                buffer[index++] = handler;
            }

            std::atomic_store(&readable_buffer_, buffer);
        }
    private:
        std::shared_ptr<void> lifetime_observer_;
        std::shared_ptr<function_type[]> readable_buffer_;
        std::vector<std::pair<uint64_t, function_type>> listeners_;
    private:
        inline static std::atomic_uint64_t global_counter_ = 0;
    };
}
