#include "global_ref.hpp"

#include <jni.h>

namespace glasssix::jni
{
	struct license_reflection_caches
	{
		global_ref_ex<jclass> class_file;
		global_ref_ex<jclass> class_context;
		global_ref_ex<jclass> class_activity_thread;
		global_ref_ex<jclass> class_settings_secure;
		global_ref_ex<jclass> class_license_deadline_callback;
		global_ref_ex<jclass> class_evaluate_license_callback;
		global_ref_ex<jclass> class_request_license_async_callback;

		jmethodID method_context_get_files_dir;
		jmethodID method_file_get_absolute_path;
		jmethodID method_settings_secure_get_string;
		jmethodID method_context_get_content_resolver;
		jmethodID method_activity_thread_get_application;
		jmethodID method_activity_thread_current_activity_thread;
		jmethodID method_license_deadline_callback_run;
		jmethodID method_evaluate_license_callback_run;
		jmethodID method_request_license_async_callback_run;
	};

	license_reflection_caches& get_license_reflection_caches();
}
