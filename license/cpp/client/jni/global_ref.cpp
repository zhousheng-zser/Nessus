#include "global_ref.hpp"
#include "reflection_cache.hpp"

#include <utility>

namespace glasssix::jni
{
	global_ref::global_ref() noexcept : global_ref{ nullptr }
	{
	}

	global_ref::global_ref(jobject obj) : global_ref{ obj, false }
	{
	}

	global_ref::global_ref(jobject obj, bool takeOverOnly) : ref_{}
	{
		if (auto env = reflection_cache::instance().get_thread_env(); env && obj)
		{
			ref_ = takeOverOnly ? obj : env->NewGlobalRef(obj);
		}
	}

	global_ref::global_ref(const global_ref& other) : global_ref{ other.ref_ }
	{
	}

	global_ref::global_ref(global_ref&& other) noexcept : ref_{ std::exchange(other.ref_, nullptr) }
	{
	}

	global_ref::~global_ref()
	{
		if (auto env = reflection_cache::instance().get_thread_env(); env && ref_)
		{
			env->DeleteGlobalRef(ref_);
			ref_ = nullptr;
		}
	}

	global_ref::operator bool() const noexcept
	{
		return ref_;
	}

	global_ref& global_ref::operator=(const global_ref& right)
	{
		auto env = reflection_cache::instance().get_thread_env();

		ref_ = env && right.ref_ ? env->NewGlobalRef(right.ref_) : nullptr;

		return *this;
	}

	global_ref& global_ref::operator=(global_ref&& right) noexcept
	{
		ref_ = std::exchange(right.ref_, nullptr);

		return *this;
	}

	jobject global_ref::get() const noexcept
	{
		return ref_;
	}
}
