#include "../../parser/cpp/parser.hpp"

#include <fstream>
#include <string>
#include <iterator>
#include <streambuf>
#include <filesystem>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <CppUnitTest.h>

using namespace glasssix::exposing;
using namespace glasssix::exposing::nessus;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace
{
	std::filesystem::path dll_directory;
}

BOOL __stdcall DllMain(HMODULE module, DWORD reason, void* reserved)
{
	if (reason == DLL_PROCESS_ATTACH)
	{
		std::string path(32767, '\0');

		path.resize(GetModuleFileNameA(module, path.data(), path.size()));
		dll_directory = std::filesystem::path{ path }.parent_path();
		std::filesystem::current_path(dll_directory);
	}

	return TRUE;
}

namespace glasssix::unit_test
{
	TEST_CLASS(test_parser)
	{
	public:
		TEST_METHOD(test_longinus_detectEx)
		{
			parser& parser_ = parser::instance();
			std::string result = parser_.init_plugin("plugin_configure.json");

			std::string protocol = "Longinus.new", device = "{\"device\":-1}";
			result = parser_.parse(protocol, device);

			protocol = "Longinus.detectEx";

			Json::Reader reader;
			Json::Value newResult;
			reader.parse(result, newResult);
			std::string instance_guid = newResult["instance_guid"].asString();

			std::ifstream fs{ "Longinus_detectEx.message", std::ios::binary };
			std::string message(std::istreambuf_iterator<char>{ fs }, std::istreambuf_iterator<char>{});

			Json::Value root;
			reader.parse(message, root);
			root["instance_guid"] = Json::Value(instance_guid);

			Json::FastWriter writer;
			std::string message_ = writer.write(root);

			result = parser_.parse(protocol, message_);
		}

		TEST_METHOD(test_irisiviel)
		{
			std::string topic = "Irisviel.new";
			std::string json = R"(
{
	"single_database_capacity":1000,
	"dimension":128,
	"working_directory":".",
	"event_id":"6de84d24-a9ff-48b7-8b36-9459146c8ffa",
	"autoAssignmentInstance":0,
	"device":-1,
	"reserved":""
})";
			Logger::WriteMessage(parser::instance().init_plugin("plugin_configure.json").c_str());
			Logger::WriteMessage(parser::instance().parse(topic, json).c_str());
		}
	};
}
