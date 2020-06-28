#include "parser.hpp"
#include "abi/guid.hpp"

namespace glasssix
{
	namespace exposing
	{
		namespace nessus
		{
			std::vector<std::string> split(const std::string &s, const std::string &seperator)
			{
				std::vector<std::string> result;
				typedef std::string::size_type string_size;
				string_size i = 0;

				while (i != s.size())
				{
					int flag = 0;
					while (i != s.size() && flag == 0)
					{
						flag = 1;
						for (string_size x = 0; x < seperator.size(); ++x)
							if (s[i] == seperator[x])
							{
								++i;
								flag = 0;
								break;
							}
					}

					flag = 0;
					string_size j = i;
					while (j != s.size() && flag == 0)
					{
						for (string_size x = 0; x < seperator.size(); ++x)
							if (s[j] == seperator[x])
							{
								flag = 1;
								break;
							}
						if (flag == 0)
							++j;
					}
					if (i != j)
					{
						result.push_back(s.substr(i, j - i));
						i = j;
					}
				}
				return result;
			}
		}
	}
}

string glasssix::exposing::nessus::parser::parse(string& protocol, string & jsonstr)
{
	Json::Value value;

	if (!ready)
	{
		value["status"] = Json::Value("parser init plugin exception");
		return writer.write(value);
	}

	simdjson::dom::element root;
	try
	{
		std::lock_guard<std::mutex> lck(mut_parse);
		root = parser_.parse(jsonstr);
	}
	catch (const std::exception& ex)
	{
		value["status"] = Json::Value("parse json error");
		return writer.write(value);
	}

	std::vector<string> str_vec = split(protocol, ".");
	if (str_vec.size() != 2)
	{
		value["status"] = Json::Value("topic illegal");
		return writer.write(value);
	}

	
	string instance_type = str_vec[0];
	string method = str_vec[1];

	if (method == "new")
	{
		uint64_t instance = 0;
		std::function<Json::Value(plugin_interface&, simdjson::dom::element&, uint64_t&)> func;
		try
		{
			func = protocol_map.at(protocol);
		}
		catch (const std::exception&)
		{
			value["status"] = Json::Value("Function of the topic not register");
			return writer.write(value);
		}
		
		value = func(plugin, root, instance);
		
		if (value["status"] == "OK")
		{
			//根据instance生成guid
			string instance_guid(to_char_array(create_guid_from_bytes(meta::to_array(instance))).data());

			std::lock_guard<std::mutex> lck(mut_instance_map);

			instance_map[instance_guid] = std::tuple<string, uint64_t, std::shared_ptr<std::mutex>>(instance_type, instance, std::shared_ptr<std::mutex>(new std::mutex));
		}
	}
	else
	{
		string instance_guid = "";
		try
		{
			instance_guid = string(root["instance_guid"].get<std::string_view>().value());
		}
		catch (const std::exception& ex)
		{
			value["status"] = Json::Value("Lack of \"instance_guid\"");
			return writer.write(value);
		}

		std::shared_ptr<std::mutex> mut_instance;
		try
		{
			std::lock_guard<std::mutex> lck(mut_instance_map);
			mut_instance = std::get<2>(instance_map.at(instance_guid));
		}
		catch (const std::exception& ex)
		{
			value["status"] = Json::Value(instance_guid + " instance not found");
			return writer.write(value);
		}

		if (method == "release")
		{
			std::lock_guard<std::mutex> lck_instance_map(mut_instance_map);
			std::lock_guard<std::mutex> lck_instance(*mut_instance);
			try
			{
				std::lock_guard<std::mutex> lck(mut_instance_map);
				auto &instance_tuple = instance_map.at(instance_guid);
			}
			catch (const std::exception& ex)
			{
				value["status"] = Json::Value(instance_guid + " instance not found");
				return writer.write(value);
			}

			std::function<Json::Value(plugin_interface&, simdjson::dom::element&, uint64_t&)> func;
			try
			{
				func = protocol_map.at(protocol);
			}
			catch (const std::exception& ex)
			{
				value["status"] = Json::Value("Function of the topic not register");
				return writer.write(value);
			}

			value = func(plugin, root, std::get<1>(instance_map[instance_guid]));
			instance_map.erase(instance_guid);
		}
		else
		{
			std::lock_guard<std::mutex> lck_instance(*mut_instance);
			try
			{
				std::lock_guard<std::mutex> lck(mut_instance_map);
				auto &instance_tuple = instance_map.at(instance_guid);
			}
			catch (const std::exception& ex)
			{
				value["status"] = Json::Value(instance_guid + " instance not found");
				return writer.write(value);
			}

			std::function<Json::Value(plugin_interface&, simdjson::dom::element&, uint64_t&)> func;
			try
			{
				func = protocol_map.at(protocol);
			}
			catch (const std::exception& ex)
			{
				value["status"] = Json::Value("Function of the topic not register");
				return writer.write(value);
			}

			value = func(plugin, root, std::get<1>(instance_map[instance_guid]));
		}
	}	

	return writer.write(value);
}
