#pragma once
#ifndef _MODULE_HPP_
#define _MODULE_HPP_

#include "vision_service_impl_implement.hpp"
#include "vision_service_impl.hpp"
#include <vector>
#include <functional>
#include <unordered_map>
#include <abi/consumer.hpp>

namespace glasssix::exposing::nessus::Service
{
	using service_function_ty = std::function<unknown_object(const param_hash_map<param_string, unknown_object>&)>;
	using service_map = std::unordered_map<param_string, service_function_ty>;

	class Service {
	public:
		virtual const void service_dump(std::unordered_map<param_string, service_function_ty>& service_map) const = 0;
	};

	class ServiceRegister {
	public:
		ServiceRegister(std::shared_ptr<Service> pplugin);
	};

	static std::vector<std::shared_ptr<Service>>& service_list_instance();

	void AddService(std::unordered_map<param_string, service_function_ty>& Service_map);


#define REGISTE_SERVICE(CLASS_NAME) \
	static ServiceRegister Service_##CLASS_NAME##_register(std::shared_ptr<Service>{new CLASS_NAME});

#define DELETE_FUNC meta::replace_return<unknown_object>(std::bind(&vision_service_impl::impl::delete_instance, std::placeholders::_1))

}
#endif