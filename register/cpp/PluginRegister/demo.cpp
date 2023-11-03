#include "abi/consumer.hpp"
#include "plugin_manager.hpp"

#include <iostream>

void test()
{
	using namespace glasssix::exposing;
	using namespace glasssix::exposing::nessus;

	// 鍔犺浇鎻掍欢绠＄悊鍣� DLL
	if (auto factory = get_component_loader().add_module_with_factory(u8"g6.pluginManager.dll"))
	{
		try
		{
			// 鍒涘缓鎻掍欢绠＄悊鍣ㄥ苟鍔犺浇鎸囧畾鐩綍涓嬫墍鏈夋彃浠�
			auto manager = factory.create_by_name(u8"glasssix.nessus.pluginManager").as<plugin_manager>();

			manager.load_from_directory(u8R"(D:\plugins)");

			// 鏂瑰紡涓€锛氭墽琛屾煇涓彃浠朵笅鐨勬煇涓嚱鏁帮紙鏁堢巼杈冧綆锛�
			// 鍋囪杩斿洖鍊兼槸涓€涓� map<string, object>
			auto array = make_param_vector<int>(1, 2, 3, 4, 5);
			auto result = manager
				.execute(u8"鎻掍欢鍚�", u8"鍑芥暟鍚�", make_param_hash_map<param_string, unknown_object>(
					{
						{ u8"瀛楃涓�", box(u8"瑁呯瀛楃涓插弬鏁�") },
						{ u8"瑁呯鏁村瀷", box(123) },
						{ u8"瑁呯娴偣鍨�", box(3.14) },
						{ u8"瑁呯鍗曠簿搴︽诞鐐瑰瀷", box(1.234f) },
						{ u8"鏁扮粍", array }
					}))
				.as<param_hash_map<param_string, unknown_object>>();

			// 鏂瑰紡浜岋細鍏堣幏鍙栨彃浠舵帴鍙ｅ紩鐢紝鍐嶈皟鐢ㄦ彃浠剁殑 execute 鍑芥暟锛堟晥鐜囪緝楂橈級
			if (auto plugin = manager.lookup(u8"鎻掍欢鍚�"))
			{
				auto result2 = plugin.execute(u8"鍑芥暟鍚�", make_param_hash_map<param_string, unknown_object>(
					{
						{ u8"瀛楃涓�", box(u8"瑁呯瀛楃涓插弬鏁�") },
						{ u8"瑁呯鏁村瀷", box(123) },
						{ u8"瑁呯娴偣鍨�", box(3.14) },
						{ u8"瑁呯鍗曠簿搴︽诞鐐瑰瀷", box(1.234f) },
						{ u8"鏁扮粍", array }
					}
				));
			}

			auto feature = result.get_value(u8"鐗瑰緛鍊�").as<param_vector<float>>();
			auto length = unbox<int>(result.get_value(u8"鐭╁舰妗嗚竟闀�"));

			// 澶勭悊杩斿洖鍊�1 锛堝紓甯稿畨鍏ㄥ瀷锛�
			if (unknown_object obj{ nullptr }; result.try_get_value(u8"鐗瑰緛鍊�", obj))
			{
				// 杞崲涓哄叿浣撶殑鎺ュ彛绫诲瀷
				auto feature = obj.as<param_vector<float>>();

				// TO DO: 澶勭悊鐗瑰緛鍊�
			}
			
			if (unknown_object obj{ nullptr }; result.try_get_value(u8"鐭╁舰妗嗚竟闀�", obj))
			{
				// 鎷嗙鍩虹绫诲瀷
				auto length = unbox<int>(obj);

				// TO DO: 澶勭悊杈归暱
			}

			// ...

			// 澶勭悊杩斿洖鍊�2锛堝紓甯稿瀷锛�
			auto significant_points = result.get_value(u8"鍏抽敭鐐�").as<param_vector<int>>();
		}
		catch (const abi_error& ex)
		{
			std::cout << "鍙戠敓閿欒锛�" << ex.what_to_narrow() << std::endl;
		}
	}
}
