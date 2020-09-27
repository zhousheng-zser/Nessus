#include "local_ref.hpp"

#include <utility>

namespace glasssix::jni
{
	local_ref::local_ref() noexcept : local_ref{ nullptr }
	{
	}

	local_ref::local_ref(jobject obj) : local_ref{ obj, false }
	{
	}

	local_ref::local_ref(jobject obj, bool takeOverOnly) : ref_{}
	{
		if (auto env = get_current_thread_env(); env && obj)
		{
			ref_ = takeOverOnly ? obj : env->NewLocalRef(obj);
		}
	}

	local_ref::local_ref(const local_ref& other) : local_ref{ other.ref_ }
	{
	}

	local_ref::local_ref(local_ref&& other) noexcept : ref_{ std::exchange(other.ref_, nullptr) }
	{
	}

	local_ref::~local_ref()
	{
		if (auto env = get_current_thread_env(); env && ref_)
		{
			env->DeleteLocalRef(ref_);
			ref_ = nullptr;
		}
	}

	local_ref::operator bool() const noexcept
	{
		return ref_;
	}

	local_ref& local_ref::operator=(const local_ref& right)
	{
		auto env = get_current_thread_env();

		ref_ = env && right.ref_ ? env->NewLocalRef(right.ref_) : nullptr;

		return *this;
	}

	local_ref& local_ref::operator=(local_ref&& right) noexcept
	{
		ref_ = std::exchange(right.ref_, nullptr);

		return *this;
	}

	jobject local_ref::get() const noexcept
	{
		return ref_;
	}
}
