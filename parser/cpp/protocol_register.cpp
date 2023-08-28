#include "protocol_register.hpp"
#include <vector>

namespace glasssix::exposing::nessus::Protocol
{
	ProtocolRegister::ProtocolRegister(std::shared_ptr<Protocol> pplugin) {
		protocol_list_instance().push_back(pplugin);
	}

	std::vector<std::shared_ptr<Protocol>>& protocol_list_instance()
	{
		static std::vector<std::shared_ptr<Protocol>> protocol_list;
		return protocol_list;
	}

	void AddProtocol(std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, Json::Value&, param_span<std::uint8_t>&, guid&, param_span<std::uint8_t>&)>>& protocol_map)
	{
		for (auto p : protocol_list_instance())
		{
			auto tmp_protoc = p->parser_protocol_dump();
			protocol_map.insert(tmp_protoc.begin(), tmp_protoc.end());
		}
	}

}

