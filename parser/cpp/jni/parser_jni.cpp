#ifndef _PARSER_JNI_HPP_
#define _PARSER_JNI_HPP_

#include "../parser.hpp"

#include <memory>
#include <cstdlib>
#include <algorithm>
#include <string_view>
#include <type_traits>
#include <mutex>

#include <jni.h>
#include <os_context.hpp>

static std::string jstring2string(JNIEnv* env, jstring jstr)
{
	std::shared_ptr<const char> native_str{ env->GetStringUTFChars(jstr, nullptr), [&](const char* inner) { env->ReleaseStringUTFChars(jstr, inner); } };

	return std::string(native_str.get(), env->GetStringLength(jstr));
}

static glasssix::exposing::param_string jstring2paramstring(JNIEnv* env, jstring jstr)
{
	std::shared_ptr<const char> native_str{ env->GetStringUTFChars(jstr, nullptr), [&](const char* inner) { env->ReleaseStringUTFChars(jstr, inner); } };

	return glasssix::exposing::param_string(native_str.get(), env->GetStringLength(jstr));
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

namespace
{
	glasssix::exposing::nessus::parser parser_singleton;
}

#ifdef __cplusplus
extern "C" {
#endif
	JNIEXPORT jstring JNICALL Java_com_glasssix_parser_Parser_parse(JNIEnv* env, jobject thiz, jstring jprotocol, jstring jstr)
	{
		static std::once_flag flag;
		std::call_once(flag, [&] {parser_singleton = glasssix::exposing::make_exported_interface<glasssix::exposing::nessus::parser>(); });

		glasssix::exposing::param_string protocol = jstring2paramstring(env, jprotocol);
		glasssix::exposing::param_string jsonstr = jstring2paramstring(env, jstr);

		glasssix::exposing::param_string result = parser_singleton.parse(protocol, jsonstr);
		return char2Jstring(env, result.data(), result.size());
	}

	JNIEXPORT jstring JNICALL Java_com_glasssix_parser_Parser_initPlugin(JNIEnv* env, jobject thiz, jstring jstr)
	{
		glasssix::exposing::param_string config_file_path = jstring2paramstring(env, jstr);

		glasssix::exposing::param_string status = parser_singleton.init_plugin(config_file_path);
		return char2Jstring(env, status.data(), status.size());
	}

#ifdef __ANDROID__
	JNIEXPORT jint JNI_OnLoad(JavaVM* jvm, void* reserved)
	{
		static constexpr std::string_view environment_var{ "G6_ANDROID_PACKAGE_RESOURCE_DIRECTORY" };
		static constexpr int available_versions[] =
		{
			JNI_VERSION_1_6,
			JNI_VERSION_1_4,
			JNI_VERSION_1_2,
			JNI_VERSION_1_1
		};

		if (jvm == nullptr)
		{
			return false;
		}

		JNIEnv* env = nullptr;
		auto version = std::find_if(std::begin(available_versions), std::end(available_versions), [&](int value) { return jvm->GetEnv(reinterpret_cast<void**>(&env), value) == JNI_OK; });

		if (version == std::end(available_versions))
		{
			return JNI_FALSE;
		}

		std::shared_ptr<_jstring> name{ env->NewStringUTF("G6_ANDROID_PACKAGE_RESOURCE_DIRECTORY"), [&](jstring inner) { env->DeleteLocalRef(inner); } };
		std::shared_ptr<_jclass> class_system{ env->FindClass("java/lang/System"), [&](jclass inner) { env->DeleteLocalRef(inner); } };
		auto method_get_property = env->GetStaticMethodID(class_system.get(), "getProperty", "(Ljava/lang/String;)Ljava/lang/String;");
		std::shared_ptr<_jstring> value{ static_cast<jstring>(env->CallStaticObjectMethod(class_system.get(), method_get_property, name.get())), [&](jstring inner) { env->DeleteLocalRef(inner); } };

		// Sets the environment variable.
		auto native_value = jstring2string(env, value.get());
		
		return (glasssix::os_context::set_environment_variable(environment_var.data(), native_value.c_str()) , *version);
	}
#endif

#ifdef __cplusplus
}
#endif

#endif
