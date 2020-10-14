#include "reflection_cache.hpp"
#include "cache_key.hpp"
#include "local_ref.hpp"
#include "global_ref.hpp"

#include <array>
#include <variant>
#include <type_traits>
#include <unordered_map>

namespace glasssix::jni
{
	namespace
	{
		/// Some JNI functions may contain parameters that are void** or JNIEnv** across different JNI versions.
		/// We provides an adapter here to support auto-casting.
		/// </summary>
		struct env_adapter
		{
			JNIEnv* env;

			JNIEnv* get() const noexcept
			{
				return env;
			}

			operator JNIEnv** () noexcept
			{
				return &env;
			}

			operator void** () noexcept
			{
				return reinterpret_cast<void**>(&env);
			}
		};
	}

	class reflection_cache::impl
	{
	public:
		impl()
		{
		}

		std::optional<java_vm_context> init(JavaVM* vm)
		{
			static constexpr std::array available_versions
			{
				JNI_VERSION_1_6,
				JNI_VERSION_1_4,
				JNI_VERSION_1_2,
				JNI_VERSION_1_1
			};

			env_adapter adapter;
			
			if (auto iter = std::find_if(available_versions.begin(), available_versions.end(), [&](int inner) { return vm->GetEnv(adapter, inner) == JNI_OK; }); iter != available_versions.end())
			{
				java_vm_context result{ vm, *iter };

				return (context_.store(result, std::memory_order::memory_order_release), result);
			}

			return std::nullopt;
		}

		JNIEnv* get_thread_env() const
		{
			auto context = context_.load(std::memory_order_acquire);

			if (!context)
			{
				return nullptr;
			}

			thread_local bool attached_in_native_code = false;

			// Creates a thread-local JNIEnv* using a trick.
			thread_local auto creator = [context]
			{
				env_adapter env{ nullptr };

				// Attaches the current thread if necessary.
				if (context && context->vm->GetEnv(env, context->version) == JNI_EDETACHED)
				{
					return context->vm->AttachCurrentThread(env, nullptr) == JNI_OK ? (attached_in_native_code = true, env.get()) : nullptr;
				}

				return env.get();
			};

			thread_local auto freeing_handler = [context](JNIEnv*)
			{
				if (context && attached_in_native_code)
				{
					context->vm->DetachCurrentThread();
				}
			};

			thread_local std::shared_ptr<JNIEnv> result{ creator(), freeing_handler };

			return result.get();
		}

		std::optional<java_vm_context> context() const
		{
			return context_.load(std::memory_order::memory_order_acquire);
		}

		global_ref_ex<jclass> get_class_cache(int key) const
		{
			return get_item_cache_internal<global_ref_ex<jclass>, jclass>(key);
		}

		jfieldID get_field_cache(int key) const
		{
			return get_item_cache_internal<jfieldID>(key);
		}

		jmethodID get_method_cache(int key) const
		{
			return get_item_cache_internal<jmethodID>(key);
		}

		void add_class_cache(int key, std::string_view name)
		{
			auto env = get_thread_env();

			if (env == nullptr)
			{
				return;
			}

			// Finds the class.
			if (local_ref_ex<jclass> clazz{ env->FindClass(name.data()), true })
			{
				cache_.insert_or_assign(make_cache_key<jclass>(key), global_ref_ex<jclass>{ clazz.get() });
			}
		}

		void add_field_caches(int class_key, std::initializer_list<std::tuple<int, std::string_view, std::string_view>> fields)
		{
			if (auto env = get_thread_env())
			{
				add_item_caches_internal<jfieldID>(env, class_key, env->functions->GetFieldID, fields);
			}
		}

		void add_method_caches(int class_key, std::initializer_list<std::tuple<int, std::string_view, std::string_view>> methods)
		{
			if (auto env = get_thread_env())
			{
				add_item_caches_internal<jmethodID>(env, class_key, env->functions->GetMethodID, methods);
			}
		}

		void add_static_method_caches(int class_key, std::initializer_list<std::tuple<int, std::string_view, std::string_view>> methods)
		{
			if (auto env = get_thread_env())
			{
				add_item_caches_internal<jmethodID>(env, class_key, env->functions->GetStaticMethodID, methods);
			}
		}
	private:
		template<typename T, typename Category = T>
		auto get_item_cache_internal(int key) const -> std::enable_if_t<std::is_constructible_v<T, std::nullptr_t>, T>
		{
			auto iter = cache_.find(make_cache_key<Category>(key));

			return iter != cache_.end() ? std::get<T>(iter->second) : T{ nullptr };
		}

		template<typename T>
		void add_item_caches_internal(JNIEnv* env, int class_key, T(*handler)(JNIEnv*, jclass, const char*, const char*), std::initializer_list<std::tuple<int, std::string_view, std::string_view>> items)
		{
			if (auto iter = cache_.find(make_cache_key<jclass>(class_key)); iter != cache_.end())
			{
				auto clazz = std::get<global_ref_ex<jclass>>(iter->second);

				// Adds the items.
				for (auto [item_key, item_name, item_signature] : items)
				{
					if (auto item_id = handler(env, clazz.get(), item_name.data(), item_signature.data()))
					{
						cache_.insert_or_assign(make_cache_key<T>(item_key), item_id);
					}
				}
			}
		}

		std::atomic<std::optional<java_vm_context>> context_;
		std::unordered_map<cache_key, std::variant<global_ref_ex<jclass>, jfieldID, jmethodID>> cache_;
	};

	reflection_cache::reflection_cache() : impl_{ std::make_unique<impl>() }
	{
	}

	reflection_cache::~reflection_cache()
	{
	}

	std::optional<java_vm_context> reflection_cache::init(JavaVM* vm) const
	{
		return impl_->init(vm);
	}

	JNIEnv* reflection_cache::get_thread_env() const
	{
		return impl_->get_thread_env();
	}

	std::optional<java_vm_context> reflection_cache::context() const
	{
		return impl_->context();
	}

	global_ref_ex<jclass> reflection_cache::get_class_cache(int key) const
	{
		return impl_->get_class_cache(key);
	}

	jfieldID reflection_cache::get_field_cache(int key) const
	{
		return impl_->get_field_cache(key);
	}

	jmethodID reflection_cache::get_method_cache(int key) const
	{
		return impl_->get_method_cache(key);
	}

	void reflection_cache::add_class_cache(int key, std::string_view name) const
	{
		impl_->add_class_cache(key, name);
	}

	void reflection_cache::add_field_caches(int class_key, std::initializer_list<std::tuple<int, std::string_view, std::string_view>> fields) const
	{
		impl_->add_field_caches(class_key, fields);
	}

	void reflection_cache::add_method_caches(int class_key, std::initializer_list<std::tuple<int, std::string_view, std::string_view>> methods) const
	{
		impl_->add_method_caches(class_key, methods);
	}

	void reflection_cache::add_static_method_caches(int class_key, std::initializer_list<std::tuple<int, std::string_view, std::string_view>> methods) const
	{
		impl_->add_static_method_caches(class_key, methods);
	}
}
