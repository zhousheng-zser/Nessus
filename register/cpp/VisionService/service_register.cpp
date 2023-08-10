#include "service_register.hpp"

#include <vector>

namespace glasssix::exposing::nessus::Service
{
	ServiceRegister::ServiceRegister(std::shared_ptr<Service> pplugin) {
		service_list_instance().push_back(pplugin);
	}

	std::vector<std::shared_ptr<Service>>& service_list_instance()
	{
		static std::vector<std::shared_ptr<Service>> service_list;
		return service_list;
	}

	void AddService(std::unordered_map<param_string, service_function_ty>& Service_map)
	{
		for (auto serv : service_list_instance())
		{
			serv->service_dump(Service_map);
		}
	}

}

