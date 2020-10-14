#include "entry.hpp"
#include "cache_key.hpp"
#include "global_ref.hpp"
#include "reflection_cache.hpp"

#include <fstream>

#include <jni.h>

using namespace glasssix::jni;

namespace
{
	enum class class_key
	{
		file,
		context,
		activity_thread,
		settings_secure,
		evaluate_license_callback,
		request_license_async_callback
	};

	enum class method_key
	{
		// Instance methods
		file_get_absolute_path,
		context_get_files_dir,
		context_get_content_resolver,
		activity_thread_get_application,
		settings_secure_get_string,
		evaluate_license_callback_run,
		request_license_async_callback_run,

		// Static methods
		static_activity_thread_current_activity_thread
	};

	license_reflection_caches internal_caches;
}

JNIEXPORT jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
	auto context = reflection_cache::instance().init(vm);

	if (!context)
	{
		return JNI_ERR;
	}

	reflection_cache::instance().add_class_cache(arg_enum_v<class_key::file>, "java/io/File");
	reflection_cache::instance().add_class_cache(arg_enum_v<class_key::context>, "android/content/Context");
	reflection_cache::instance().add_class_cache(arg_enum_v<class_key::activity_thread>, "android/app/ActivityThread");
	reflection_cache::instance().add_class_cache(arg_enum_v<class_key::settings_secure>, "android/provider/Settings$Secure");
	reflection_cache::instance().add_class_cache(arg_enum_v<class_key::evaluate_license_callback>, "com/glasssix/license/EvaluateLicenseCallback");
	reflection_cache::instance().add_class_cache(arg_enum_v<class_key::request_license_async_callback>, "com/glasssix/license/RequestLicenseAsyncCallback");
	reflection_cache::instance().add_method_caches(arg_enum_v<class_key::file>,
		{
			{ arg_enum_v<method_key::file_get_absolute_path>, "getAbsolutePath", "()Ljava/lang/String;" },
		});

	reflection_cache::instance().add_method_caches(arg_enum_v<class_key::context>,
		{
			{ arg_enum_v<method_key::context_get_files_dir>, "getFilesDir", "()Ljava/io/File;" },
			{ arg_enum_v<method_key::context_get_content_resolver>, "getContentResolver", "()Landroid/content/ContentResolver;" }
		});

	reflection_cache::instance().add_method_caches(arg_enum_v<class_key::activity_thread>,
		{
			{ arg_enum_v<method_key::activity_thread_get_application>, "getApplication", "()Landroid/app/Application;" },
		});

	reflection_cache::instance().add_static_method_caches(arg_enum_v<class_key::activity_thread>,
		{
			{ arg_enum_v<method_key::static_activity_thread_current_activity_thread>, "currentActivityThread", "()Landroid/app/ActivityThread;" }
		});

	reflection_cache::instance().add_static_method_caches(arg_enum_v<class_key::settings_secure>,
		{
			{ arg_enum_v<method_key::settings_secure_get_string>, "getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;" }
		});

	reflection_cache::instance().add_method_caches(arg_enum_v<class_key::evaluate_license_callback>,
		{
			{ arg_enum_v<method_key::evaluate_license_callback_run>, "run", "(ZLjava/lang/String;J)V" }
		});

	reflection_cache::instance().add_method_caches(arg_enum_v<class_key::request_license_async_callback>,
		{
			{ arg_enum_v<method_key::request_license_async_callback_run>, "run", "(ZLjava/lang/String;)V" }
		});


	internal_caches.class_file = reflection_cache::instance().get_class_cache(arg_enum_v<class_key::file>);
	internal_caches.class_context = reflection_cache::instance().get_class_cache(arg_enum_v<class_key::context>);
	internal_caches.class_activity_thread = reflection_cache::instance().get_class_cache(arg_enum_v<class_key::activity_thread>);
	internal_caches.class_settings_secure = reflection_cache::instance().get_class_cache(arg_enum_v<class_key::settings_secure>);
	internal_caches.class_evaluate_license_callback = reflection_cache::instance().get_class_cache(arg_enum_v<class_key::evaluate_license_callback>);
	internal_caches.class_request_license_async_callback = reflection_cache::instance().get_class_cache(arg_enum_v<class_key::request_license_async_callback>);

	internal_caches.method_context_get_files_dir = reflection_cache::instance().get_method_cache(arg_enum_v<method_key::context_get_files_dir>);
	internal_caches.method_file_get_absolute_path = reflection_cache::instance().get_method_cache(arg_enum_v<method_key::file_get_absolute_path>);
	internal_caches.method_settings_secure_get_string = reflection_cache::instance().get_method_cache(arg_enum_v<method_key::settings_secure_get_string>);
	internal_caches.method_context_get_content_resolver = reflection_cache::instance().get_method_cache(arg_enum_v<method_key::context_get_content_resolver>);
	internal_caches.method_activity_thread_get_application = reflection_cache::instance().get_method_cache(arg_enum_v<method_key::activity_thread_get_application>);
	internal_caches.method_activity_thread_current_activity_thread = reflection_cache::instance().get_method_cache(arg_enum_v<method_key::static_activity_thread_current_activity_thread>);
	internal_caches.method_evaluate_license_callback_run = reflection_cache::instance().get_method_cache(arg_enum_v<method_key::evaluate_license_callback_run>);
	internal_caches.method_request_license_async_callback_run = reflection_cache::instance().get_method_cache(arg_enum_v<method_key::request_license_async_callback_run>);

	return context->version;
}

JNIEXPORT void JNI_OnUnload(JavaVM* vm, void* reserved)
{
}

namespace glasssix::jni
{
	license_reflection_caches& get_license_reflection_caches()
	{
		return internal_caches;
	}
}
