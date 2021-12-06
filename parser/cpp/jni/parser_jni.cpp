#include "../parser.hpp"

#include <memory>
#include <cstdlib>
#include <algorithm>
#include <string_view>
#include <type_traits>

#include <jni.h>
#include <os_context.hpp>

#ifndef G6_DISABLE_LICENSE
#include <vulcanus/license.hpp>
#endif

using namespace glasssix;

#ifndef G6_DISABLE_LICENSE
using namespace glasssix::license;
#endif

static inline std::string jstring2string(JNIEnv* env, jstring jstr)
{
	if (jstr == nullptr)
		return {};

	std::shared_ptr<const char> native_str{ env->GetStringUTFChars(jstr, nullptr), [&](const char* inner) { env->ReleaseStringUTFChars(jstr, inner); } };

	return std::string(native_str.get(), env->GetStringUTFLength(jstr));
}

static inline glasssix::exposing::param_string jstring2paramstring(JNIEnv* env, jstring jstr)
{
	if (jstr == nullptr)
		return {};

	std::shared_ptr<const char> native_str{ env->GetStringUTFChars(jstr, nullptr), [&](const char* inner) { env->ReleaseStringUTFChars(jstr, inner); } };

	return glasssix::exposing::param_string(native_str.get(), env->GetStringUTFLength(jstr));
}

static jstring char2Jstring(JNIEnv* env, const char* pat, size_t len)
{
	jclass strClazz = env->FindClass("java/lang/String");
	jmethodID mid_String_constructor = env->GetMethodID(strClazz, "<init>", "([BLjava/lang/String;)V");
	jbyteArray bytes = env->NewByteArray(static_cast<jsize>(len));
	env->SetByteArrayRegion(bytes, 0, static_cast<jsize>(len), (jbyte*)pat);
	jstring encoding = env->NewStringUTF("utf-8");

	jstring jstr = (jstring)env->NewObject(strClazz, mid_String_constructor, bytes, encoding);

	env->DeleteLocalRef(encoding);
	env->DeleteLocalRef(bytes);
	env->DeleteLocalRef(strClazz);

	return jstr;
}


#ifdef __cplusplus
extern "C" {
#endif
	JNIEXPORT void JNICALL Java_com_glasssix_parser_Parser_init(JNIEnv* env, jobject thiz)
	{
		jclass clazz = env->GetObjectClass(thiz);
		jfieldID fid_mObject = env->GetFieldID(clazz, "mObject", "J");
		auto parser_object = glasssix::exposing::make_exported_interface<glasssix::exposing::nessus::parser>();
		env->SetLongField(thiz, fid_mObject, reinterpret_cast<jlong>(glasssix::exposing::detach_abi(parser_object)));
		env->DeleteLocalRef(clazz);
	}

	JNIEXPORT void JNICALL Java_com_glasssix_parser_Parser_release(JNIEnv* env, jobject thiz)
	{
		jclass clazz = env->GetObjectClass(thiz);
		jfieldID fid_mObject = env->GetFieldID(clazz, "mObject", "J");
		jlong p = env->GetLongField(thiz, fid_mObject);

		glasssix::exposing::unknown_object parser_objcet{ reinterpret_cast<void*>(p) };

		env->SetLongField(thiz, fid_mObject, (jlong)0);
		env->DeleteLocalRef(clazz);
	}

	JNIEXPORT jstring JNICALL Java_com_glasssix_parser_Parser_parse(JNIEnv* env, jobject thiz, jstring jtopic, jstring jstr_param, jbyteArray dataArray, jbyteArray externalArray)
	{
#ifndef G6_DISABLE_LICENSE
		try
		{
			check_last_license_error();
		}
		catch (const exposing::abi_error& ex)
		{
			std::unique_ptr<_jclass, std::function<void(jclass)>> class_exception{ env->FindClass("java/lang/Exception"), [&](jclass inner) { env->DeleteLocalRef(inner); } };

			return (env->ThrowNew(class_exception.get(), ex.what_to_narrow().c_str()), nullptr);
		}
#endif

		jclass clazz = env->GetObjectClass(thiz);
		jfieldID fid_mObject = env->GetFieldID(clazz, "mObject", "J");
		jlong p = env->GetLongField(thiz, fid_mObject);

		auto parser_object{ glasssix::exposing::create_from_abi<glasssix::exposing::nessus::parser>(reinterpret_cast<void*>(p)) };

		glasssix::exposing::param_string topic = jstring2paramstring(env, jtopic);
		glasssix::exposing::param_string str_param = jstring2paramstring(env, jstr_param);

		glasssix::exposing::param_span<std::uint8_t> data(nullptr, 0);
		jbyte* data_ptr = nullptr;
		if (dataArray != nullptr)
		{
			data_ptr = env->GetByteArrayElements(dataArray, 0);
			data = glasssix::exposing::param_span<std::uint8_t>(reinterpret_cast<std::uint8_t*>(data_ptr), env->GetArrayLength(dataArray));
		}

		glasssix::exposing::param_span<std::uint8_t> external(nullptr, 0);
		jbyte* external_ptr = nullptr;
		if (externalArray != nullptr)
		{
			external_ptr = env->GetByteArrayElements(externalArray, 0);
			external = glasssix::exposing::param_span<std::uint8_t>(reinterpret_cast<std::uint8_t*>(external_ptr), env->GetArrayLength(externalArray));
		}

		glasssix::exposing::param_string result = parser_object.parse(topic, str_param, data, external);

		if(external_ptr)
			env->ReleaseByteArrayElements(externalArray, external_ptr, 0);
		if(data_ptr)
			env->ReleaseByteArrayElements(dataArray, data_ptr, 0);
		env->DeleteLocalRef(clazz);

		return char2Jstring(env, result.data(), result.size());
	}

	JNIEXPORT jstring JNICALL Java_com_glasssix_parser_Parser_initPlugin(JNIEnv* env, jobject thiz, jstring jstr, jstring license_key)
	{
		std::unique_ptr<_jclass, std::function<void(jclass)>> class_exception{ env->FindClass("java/lang/Exception"), [&](jclass inner) { env->DeleteLocalRef(inner); } };

#ifndef G6_DISABLE_LICENSE
		init_license_system(jstring2string(env, license_key).c_str());

		try
		{
			check_last_license_error();
		}
		catch (const exposing::abi_error& ex)
		{
			return (env->ThrowNew(class_exception.get(), ex.what_to_narrow().c_str()), nullptr);
		}
#endif

		jclass clazz = env->GetObjectClass(thiz);
		jfieldID fid_mObject = env->GetFieldID(clazz, "mObject", "J");
		jlong p = env->GetLongField(thiz, fid_mObject);

		auto parser_object{ glasssix::exposing::create_from_abi<glasssix::exposing::nessus::parser>(reinterpret_cast<void*>(p)) };

		glasssix::exposing::param_string config_file_path = jstring2paramstring(env, jstr);
		glasssix::exposing::param_string status = parser_object.init_plugin(config_file_path);

		env->DeleteLocalRef(clazz);

		return char2Jstring(env, status.data(), status.size());
	}

//#ifdef __ANDROID__
//	JNIEXPORT jint JNI_OnLoad(JavaVM* jvm, void* reserved)
//	{
//		static constexpr std::string_view environment_var{ "G6_ANDROID_PACKAGE_RESOURCE_DIRECTORY" };
//		static constexpr int available_versions[] =
//		{
//			JNI_VERSION_1_6,
//			JNI_VERSION_1_4,
//			JNI_VERSION_1_2,
//			JNI_VERSION_1_1
//		};
//
//		if (jvm == nullptr)
//		{
//			return false;
//		}
//
//		JNIEnv* env = nullptr;
//		auto version = std::find_if(std::begin(available_versions), std::end(available_versions), [&](int value) { return jvm->GetEnv(reinterpret_cast<void**>(&env), value) == JNI_OK; });
//
//		if (version == std::end(available_versions))
//		{
//			return JNI_FALSE;
//		}
//
//		std::shared_ptr<_jstring> name{ env->NewStringUTF("G6_ANDROID_PACKAGE_RESOURCE_DIRECTORY"), [&](jstring inner) { env->DeleteLocalRef(inner); } };
//		std::shared_ptr<_jclass> class_system{ env->FindClass("java/lang/System"), [&](jclass inner) { env->DeleteLocalRef(inner); } };
//		auto method_get_property = env->GetStaticMethodID(class_system.get(), "getProperty", "(Ljava/lang/String;)Ljava/lang/String;");
//		std::shared_ptr<_jstring> value{ static_cast<jstring>(env->CallStaticObjectMethod(class_system.get(), method_get_property, name.get())), [&](jstring inner) { env->DeleteLocalRef(inner); } };
//
//		// Sets the environment variable.
//		auto native_value = jstring2string(env, value.get());
//		
//		return (glasssix::os_context::set_environment_variable(environment_var.data(), native_value.c_str()) , *version);
//	}
//#endif

#ifdef __cplusplus
}
#endif
