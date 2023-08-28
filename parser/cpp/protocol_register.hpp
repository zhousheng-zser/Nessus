#ifndef _MODULE_HPP_
#define _MODULE_HPP_

#include "message_protocol_jsoncpp.hpp"
#include <vector>
#include "json.h"
#include "plugin_interface.hpp"

namespace glasssix::exposing::nessus::Protocol
{
	using protocol_function = std::function<Json::Value(plugin_interface&, Json::Value&, param_span<std::uint8_t>&, guid&, param_span<std::uint8_t>&)>;
	
	class Protocol {
	public:
		virtual const std::unordered_map<std::string, protocol_function> parser_protocol_dump() const = 0;
	};

	class ProtocolRegister {
	public:
		ProtocolRegister(std::shared_ptr<Protocol> pplugin);
	};

	static std::vector<std::shared_ptr<Protocol>>& protocol_list_instance();

#define REGISTE_PROTOCOL(CLASS_NAME) \
	static ProtocolRegister protocol_##CLASS_NAME##_register(std::shared_ptr<Protocol>{new CLASS_NAME});

	void AddProtocol(std::unordered_map<std::string, std::function<Json::Value(plugin_interface&, Json::Value&, param_span<std::uint8_t>&, guid&, param_span<std::uint8_t>&)>>& protocol_map);

}
#endif