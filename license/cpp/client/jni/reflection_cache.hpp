#pragma once

#include "global_ref.hpp"

#include <tuple>
#include <memory>
#include <optional>
#include <functional>
#include <string_view>
#include <initializer_list>

#include <jni.h>
#include <singleton.hpp>

namespace glasssix::jni
{
	struct java_vm_context
	{
		JavaVM* vm;
		int version;
	};

	class reflection_cache : public singleton<reflection_cache>
	{
	public:
		class impl;
		friend singleton;

		virtual ~reflection_cache();
		std::shared_ptr<java_vm_context> init(JavaVM* vm) const;
		JNIEnv* get_thread_env() const;
		std::shared_ptr<java_vm_context> context() const;
		global_ref_ex<jclass> get_class_cache(int key) const;
		jfieldID get_field_cache(int key) const;
		jmethodID get_method_cache(int key) const;
		void add_class_cache(int key, std::string_view name) const;
		void add_field_caches(int class_key, std::initializer_list<std::tuple<int, std::string_view, std::string_view>> fields) const;
		void add_method_caches(int class_key, std::initializer_list<std::tuple<int, std::string_view, std::string_view>> methods) const;
		void add_static_method_caches(int class_key, std::initializer_list<std::tuple<int, std::string_view, std::string_view>> methods) const;
	private:
		reflection_cache();
		std::unique_ptr<impl> impl_;
	};
}
