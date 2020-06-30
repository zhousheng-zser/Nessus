#ifndef _PARSER_JNI_HPP_
#define _PARSER_JNI_HPP_

#include <jni.h>
#include "../parser.hpp"

#ifdef __cplusplus
extern "C" {
#endif

static std::string jstring2string(JNIEnv *env, jstring jstr)
{
	char *rtn = nullptr;
	jclass strClazz = env->FindClass("java/lang/String");
	jstring strEncode = env->NewStringUTF("utf-8");
	jmethodID mid = env->GetMethodID(strClazz, "getBytes", "(Ljava/lang/String;)[B");
	jbyteArray bytes = (jbyteArray)env->CallObjectMethod(jstr, mid, strEncode);
	jsize len = env->GetArrayLength(bytes);
	jbyte *p = env->GetByteArrayElements(bytes, JNI_FALSE);
	if(len > 0)
	{
		rtn = (char *)malloc(len + 1);
		memcpy(rtn, p, len);
		rtn[len] = 0;
	}
	
	env->ReleaseByteArrayElements(bytes, p, 0);
	env->DeleteLocalRef(bytes);
	env->DeleteLocalRef(strEncode);
	env->DeleteLocalRef(strClazz);
	
	std::string str(rtn);
	free(rtn);
	
	return str;
}

static jstring char2Jstring(JNIEnv *env, const char *pat, size_t len)
{
	jclass strClazz = env->FindClass("java/lang/String");
	jmethodID mid_String_constructor = env->GetMethodID(strClazz, "<init>", "([BLjava/lang/String;)V");
	jbyteArray bytes = env->NewByteArray(len);
	env->SetByteArrayRegion(bytes, 0, len, (jbyte *)pat);
	jstring encoding = env->NewStringUTF("utf-8");
	
	jstring jstr = (jstring)env->NewObject(strClazz, mid_String_constructor, bytes, encoding);
	
	env->DeleteLocalRef(encoding);
	env->DeleteLocalRef(bytes);
	env->DeleteLocalRef(strClazz);
	
	return jstr;
}

JNIEXPORT void JNICALL Java_com_glasssix_parser_parser_init(JNIEnv *env, jobject thiz)
{
	glasssix::exposing::nessus::parser *p = &glasssix::exposing::nessus::parser::instance();
	jclass clazz = env->GetObjectClass(thiz);
	jfieldID fid_mObject = env->GetFieldID(clazz, "mObject", "J");
	env->SetLongField(thiz, fid_mObject, (jlong)p);
	
	env->DeleteLocalRef(clazz);
}

JNIEXPORT jstring JNICALL Java_com_glasssix_parser_parser_parse(JNIEnv *env, jobject thiz, jstring jtopic, jstring jstr)
{
	jclass clazz = env->GetObjectClass(thiz);
	jfieldID fid_mObject = env->GetFieldID(clazz, "mObject", "J");
	jlong p = env->GetLongField(thiz, fid_mObject);
	glasssix::exposing::nessus::parser *instance = (glasssix::exposing::nessus::parser *)p;
	
	std::string topic = jstring2string(env, jtopic);
	std::string jsonstr = jstring2string(env, jstr);
	env->DeleteLocalRef(clazz);
	
	std::string result = instance->parse(topic, jsonstr);
	return char2Jstring(env, result.c_str(), result.length());
}

JNIEXPORT void JNICALL Java_com_glasssix_parser_parser_initPlugin(JNIEnv *env, jobject thiz, jstring jstr)
{
	jclass clazz = env->GetObjectClass(thiz);
	jfieldID fid_mObject = env->GetFieldID(clazz, "mObject", "J");
	jlong p = env->GetLongField(thiz, fid_mObject);
	glasssix::exposing::nessus::parser *instance = (glasssix::exposing::nessus::parser *)p;
	
	std::string config_file_path = jstring2string(env, jstr);
	env->DeleteLocalRef(clazz);
	
	instance->init_plugin(config_file_path);
}

#ifdef __cplusplus
}
#endif

#endif