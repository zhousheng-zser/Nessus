#include "parser.hpp"

#include <fstream>
#include <string>
#include <iterator>
#include <streambuf>

using namespace glasssix::exposing;
using namespace glasssix::exposing::nessus;

int main()
{
	parser& parser_ = parser::instance();
	std::string result = parser_.init_plugin("plugin_configure.json");
	std::cout << "init_plugin: " << result << std::endl;

	std::string protocol = "Longinus.new", device = "{\"device\":-1}";
	result = parser_.parse(protocol, device);
	std::cout << "Longinus.new: " << result << std::endl;

	protocol = "Longinus.detectEx";

	Json::Reader reader;
	Json::Value newResult;
	reader.parse(result, newResult);
	std::string instance_guid = newResult["instance_guid"].asString();
	//std::cout << instance_guid << std::endl;

	std::ifstream fs{ "Longinus_detectEx.message", std::ios::binary };
	std::string message(std::istreambuf_iterator<char>{ fs }, std::istreambuf_iterator<char>{});

	Json::Value root;
	reader.parse(message, root);
	root["instance_guid"] = Json::Value(instance_guid);

	Json::FastWriter writer;
	std::string message_ = writer.write(root);

	result = parser_.parse(protocol, message_);
	std::cout << "Longinus.detectEx: " << result << std::endl;

	return 0;
}
