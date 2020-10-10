#include "utils.hpp"
#include "local_ref.hpp"
#include "global_ref.hpp"

#include <mutex>
#include <regex>
#include <atomic>
#include <memory>
#include <fstream>
#include <iterator>
#include <optional>

#include <abi/sha3.hpp>
#include <abi/meta.hpp>

namespace glasssix::jni
{
	namespace meta = exposing::meta;
	namespace hashing = exposing::hashing;

	namespace
	{
		/// <summary>
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

		struct java_vm_context
		{
			JavaVM* vm;
			JavaVMInitArgs init_args;
		};

		std::atomic<std::optional<java_vm_context>> current_context;
		
		/// <summary>
		/// Gets the JVM of the current process.
		/// </summary>
		/// <returns>The JVM</returns>
		std::optional<java_vm_context> get_current_context()
		{
			auto context = current_context.load(std::memory_order_acquire);
			
			if (!context)
			{
				jsize size = 1;
				JavaVM* value = nullptr;
				JavaVMInitArgs init_args;

				if (JavaVMInitArgs init_args; JNI_GetDefaultJavaVMInitArgs(&init_args) == JNI_OK && JNI_GetCreatedJavaVMs(&value, size, &size) == JNI_OK)
				{
					current_context.store(java_vm_context{ value, init_args }, std::memory_order::memory_order_release);
				}

				return current_context.load(std::memory_order_acquire);
			}

			return context;
		}

		/// <summary>
		/// Gets the context of the main activity.
		/// </summary>
		/// <param name="env">The JNI environment</param>
		/// <returns>The context</returns>
		jobject get_main_activity_context(JNIEnv* env)
		{
			static std::once_flag flag;
			static jmethodID method_get_application;
			static jmethodID method_current_activity_thread;
			static global_ref_ex<jclass> class_activity_thread;

			// Caches the JNI information.
			std::call_once(flag, [&]
				{
					class_activity_thread = local_ref_ex<jclass>{ env->FindClass("android/app/ActivityThread") }.get();
					method_get_application = env->GetMethodID(class_activity_thread.get(), "getApplication", "()Landroid/app/Application;");
					method_current_activity_thread = env->GetStaticMethodID(class_activity_thread.get(), "currentActivityThread", "()Landroid/app/ActivityThread;");
				});

			auto current_activity_thread = env->CallStaticObjectMethod(class_activity_thread.get(), method_current_activity_thread);

			return env->CallObjectMethod(current_activity_thread, method_get_application);
		}

		/// <summary>
		/// Gets the CPU serial number of ARM hardware.
		/// </summary>
		/// <returns>The CPU serial number</returns>
		std::string get_cpu_serial_number()
		{
			thread_local std::regex pattern{ R"(Serial\s*?\:\s*?(\S+?)$)", std::regex_constants::icase | std::regex_constants::ECMAScript };

			if (std::ifstream stream{ "/proc/cpuinfo", std::ios::in | std::ios::binary })
			{
				std::smatch matches;
				std::string buffer_(std::istreambuf_iterator<char>{ stream }, std::istreambuf_iterator<char>{});

				return std::regex_search(buffer_, matches, pattern) ? matches[1].str() : std::string();
			}

			return std::string();
		}
		
		/// <summary>
		/// Gets the Android ID.
		/// </summary>
		/// <returns>The Android ID</returns>
		std::string get_android_id()
		{
			static std::once_flag flag;
			static jmethodID method_get_string;
			static jmethodID method_get_context_resolver;
			static global_ref_ex<jclass> class_context;
			static global_ref_ex<jclass> class_settings_secure;

			auto env = get_current_thread_env();

			if (env == nullptr)
			{
				return std::string();
			}

			auto context = get_main_activity_context(env);

			if (context == nullptr)
			{
				return std::string();
			}

			std::call_once(flag, [&]
				{
					class_context = local_ref_ex<jclass>{ env->GetObjectClass(context) }.get();
					class_settings_secure = local_ref_ex<jclass>{ env->FindClass("android/provider/Settings$Secure") }.get();
					method_get_context_resolver = env->GetMethodID(class_context.get(), "getContentResolver", "()Landroid/content/ContentResolver;");
					method_get_string = env->GetStaticMethodID(class_settings_secure.get(), "getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;");
				});

			local_ref content_resolver{ env->CallObjectMethod(context, method_get_context_resolver) };
			auto android_id = static_cast<jstring>(env->CallStaticObjectMethod(class_settings_secure.get(), method_get_string, content_resolver, env->NewStringUTF("android_id")));

			return to_string(android_id);
		}
	}

	JNIEnv* get_current_thread_env()
	{
		auto context = get_current_context();

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
			if (context && context->vm->GetEnv(env, context->init_args.version) == JNI_EDETACHED)
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

	std::string get_android_device_id()
	{
		std::string input;
		auto hash_code = hashing::sha3::hash_sha3_512(reinterpret_cast<const std::uint8_t*>(input.append(get_android_id()).append(get_cpu_serial_number()).c_str()), input.size());
		auto hash_chars = meta::to_char_array(hash_code);

		return std::string(hash_chars.begin(), hash_chars.end());
	}

	std::string get_application_files_directory()
	{
		static std::once_flag flag;
		static jmethodID method_get_files_dir;
		static jmethodID method_get_absolute_path;
		static global_ref_ex<jclass> class_context;
		static global_ref_ex<jclass> class_file;

		auto env = get_current_thread_env();

		if (env == nullptr)
		{
			return std::string();
		}

		auto context = get_main_activity_context(env);

		if (context == nullptr)
		{
			return std::string();
		}

		std::call_once(flag, [&]
			{
				class_context = local_ref_ex<jclass>{ env->GetObjectClass(context) }.get();
				class_file = local_ref_ex<jclass>{ env->FindClass("java/io/File") }.get();
				method_get_files_dir = env->GetMethodID(class_context.get(), "getFilesDir", "()Ljava/io/File;");
				method_get_absolute_path = env->GetMethodID(class_file.get(), "getAbsolutePath", "()Ljava/lang/String;");
			});

		auto files = env->CallObjectMethod(context, method_get_files_dir);
		auto directory = static_cast<jstring>(env->CallObjectMethod(files, method_get_absolute_path));

		return to_string(directory);
	}

	std::string to_string(jstring str)
	{
		if (auto env = get_current_thread_env(); env && str)
		{
			return std::string(env->GetStringUTFChars(str, nullptr), env->GetStringUTFLength(str));
		}

		return std::string();
	}
}
