#include "../license.hpp"
#include "entry.hpp"
#include "utils.hpp"
#include "global_ref.hpp"
#include "reflection_cache.hpp"

#include <cstdint>

#include <jni.h>

using namespace glasssix::jni;
using namespace glasssix::license;

namespace
{
	auto& internal_caches = get_license_reflection_caches();
}

extern "C"
{
	/*
	 * Class:     com_glasssix_license_License
	 * Method:    initSystem
	 * Signature: (Ljava/lang/String;Lcom/glasssix/license/LicenseDeadlineCallback;)V
	 */
	JNIEXPORT void JNICALL Java_com_glasssix_license_License_initSystem(JNIEnv* env, jclass clazz, jstring license_key, jobject deadline_callback)
	{
		if (deadline_callback)
		{
			return init_license_system(to_string(license_key).c_str(), [](void* context, const char* message, std::int64_t remaining_seconds)
				{
					if (auto env = reflection_cache::instance().get_thread_env())
					{
						global_ref callback{ static_cast<jobject>(context), true };

						env->CallVoidMethod(callback.get(), internal_caches.method_license_deadline_callback_run, env->NewStringUTF(message), remaining_seconds);
					}
				}, env->NewGlobalRef(deadline_callback));
		}

		init_license_system(to_string(license_key).c_str());
	}

	/*
	 * Class:     com_glasssix_license_License
	 * Method:    evaluate
	 * Signature: (Lcom/glasssix/license/EvaluateLicenseCallback;)V
	 */
	JNIEXPORT void JNICALL Java_com_glasssix_license_License_evaluate(JNIEnv* env, jclass clazz, jobject callback)
	{
		if (callback)
		{
			evaluate_license([](void* context, bool valid, const char* message, std::int64_t remaining_seconds)
				{
					if (auto env = reflection_cache::instance().get_thread_env())
					{
						global_ref callback{ static_cast<jobject>(context), true };

						env->CallVoidMethod(callback.get(), internal_caches.method_evaluate_license_callback_run, static_cast<jboolean>(valid ? JNI_TRUE : JNI_FALSE), env->NewStringUTF(message), remaining_seconds);
					}
				}, env->NewGlobalRef(callback));
		}
	}

	/*
	 * Class:     com_glasssix_license_License
	 * Method:    requestAsync
	 * Signature: (Lcom/glasssix/license/RequestLicenseAsyncCallback;)V
	 */
	JNIEXPORT void JNICALL Java_com_glasssix_license_License_requestAsync(JNIEnv* env, jclass clazz, jobject callback)
	{
		if (callback)
		{
			request_license_async([](void* context, bool success, const char* message)
				{
					if (auto env = reflection_cache::instance().get_thread_env())
					{
						global_ref callback{ static_cast<jobject>(context), true };

						env->CallVoidMethod(callback.get(), internal_caches.method_request_license_async_callback_run, static_cast<jboolean>(success ? JNI_TRUE : JNI_FALSE), env->NewStringUTF(message));
					}
				}, env->NewGlobalRef(callback));
		}
	}
}
