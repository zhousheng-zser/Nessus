#include "abi/consumer.hpp"
#include "plugin_manager.hpp"

#include <iostream>

void test()
{
	using namespace glasssix::exposing;
	using namespace glasssix::exposing::nessus;

	// 加载插件管理器 DLL
	if (auto factory = component_loader::instance().add_module_with_factory(u8"g6.pluginManager.dll"))
	{
		try
		{
			// 创建插件管理器并加载指定目录下所有插件
			auto manager = factory.create_instance(u8"glasssix.nessus.pluginManager").as<plugin_manager>();

			manager.load_from_directory(u8R"(D:\plugins)");

			// 方式一：执行某个插件下的某个函数（效率较低）
			// 假设返回值是一个 map<string, object>
			auto array = make_param_vector<int>(1, 2, 3, 4, 5);
			auto result = manager
				.execute(u8"插件名", u8"函数名", make_param_hash_map<param_string, unknown_object>(
					{
						{ u8"字符串", box(u8"装箱字符串参数") },
						{ u8"装箱整型", box(123) },
						{ u8"装箱浮点型", box(3.14) },
						{ u8"装箱单精度浮点型", box(1.234f) },
						{ u8"数组", array }
					}))
				.as<param_hash_map<param_string, unknown_object>>();

			// 方式二：先获取插件接口引用，再调用插件的 execute 函数（效率较高）
			if (auto plugin = manager.lookup(u8"插件名"))
			{
				auto result2 = plugin.execute(u8"函数名", make_param_hash_map<param_string, unknown_object>(
					{
						{ u8"字符串", box(u8"装箱字符串参数") },
						{ u8"装箱整型", box(123) },
						{ u8"装箱浮点型", box(3.14) },
						{ u8"装箱单精度浮点型", box(1.234f) },
						{ u8"数组", array }
					}
				));
			}

			auto feature = result.get_value(u8"特征值").as<param_vector<float>>();
			auto length = unbox<int>(result.get_value(u8"矩形框边长"));

			// 处理返回值1 （异常安全型）
			if (unknown_object obj{ nullptr }; result.try_get_value(u8"特征值", obj))
			{
				// 转换为具体的接口类型
				auto feature = obj.as<param_vector<float>>();

				// TO DO: 处理特征值
			}
			
			if (unknown_object obj{ nullptr }; result.try_get_value(u8"矩形框边长", obj))
			{
				// 拆箱基础类型
				auto length = unbox<int>(obj);

				// TO DO: 处理边长
			}

			// ...

			// 处理返回值2（异常型）
			auto significant_points = result.get_value(u8"关键点").as<param_vector<int>>();
		}
		catch (const abi_error& ex)
		{
			std::cout << "发生错误：" << ex.what_to_narrow() << std::endl;
		}
	}
}
