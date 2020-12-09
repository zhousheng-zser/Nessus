# Nessus
Nessus被设计为一个统一入口组件，依赖于[Primitives](https://github.com/CompileSense/Excalibur)，包含解析器和授权系统两大模块。其通过将动态链接库(.dll)或共享库(.so)以插件形式加载的方式来获取新的功能，并对外提供一套预定义规则的调用接口来实现服务。

- 解析器(parser)：提供了初始化插件列表及注册授权码接口，提供了协议解析入口，详细说明请参考《Glasssix人脸识别C语言SDK解析器及授权系统说明》
- 授权系统： 提供了验证当前设备证书有效性、申请有效许可证、注册临期回调函数的接口，详细说明请参考《Glasssix人脸识别C语言SDK解析器及授权系统说明》

#### 特性
- 解析器及授权系统接口以C语言封装，方便跨语言调用，利于C#、Python等其他语言二次封装，特别地，针对java语言，解析器及授权系统都封装了jni接口，并提供java开发包。
- 解析器的协议解析入口为方便跨语言开发及考虑到扩展性，参数定义了param及data字段，目前param字段接收json字符串形式的参数，data字段预留为额外数据参数。

#### 编译环境
|  系统 |  windows | ubuntu18.04  | centos7  | android  |
| ------------ | ------------ | ------------ | ------------ | ------------ |
|  编译器 | MSVC19.28  |  gcc7.4.1  |  gcc8.3.1 |  NDK-21d |
|  cmake版本 | 3.18.20081302-MSVC_2  |  3.15.0 | 3.15.0  |  3.15.0  |

#### 编译步骤
- **windows**
1. 将工程根目录下CMakeSettings.json.example重命名为CMakeSettings.json
2. 使用visual studio 2019以cmake工程形式打开Nessus/cmake目录
3. 在VS中打开CMakeSettings.json，配置COMMON_LIBRARY_DIRS、WITH_JNI、及JNI_INCLUDE_DIRS
4. 执行build

- **ubuntu**
1. 安装boost(version 1.65以上)及opencv(version 3.4.1以上)
1. cd Nessus/cmake
2. mkdir build
3. cd build
4. cmake .. -DCOMMON_LIBRARY_DIRS=[where your excalibur and primitives library exist] -DWITH_JNI=ON -DJNI_INCLUDE_DIRS="${JAVA_HOME}/include;${JAVA_HOME}/include/linux"
5. make

- **centos**
1. 安装gcc8.3.1、boost(version 1.65以上)及opencv(version 3.4.1以上)
2. scl enable devtoolset-8 bash
3. cd Nessus/cmake
4. mkdir build
5. cd build
6. cmake .. -DCOMMON_LIBRARY_DIRS=[where your excalibur and primitives library exist] -DWITH_JNI=ON -DJNI_INCLUDE_DIRS="${JAVA_HOME}/include;${JAVA_HOME}/include/linux"
7. make

- **Android**
1. cd Nessus/cmake
2. mkdir build
3. cmake.exe .. -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=E:/android-ndk-r21d/build/cmake/android.toolchain.cmake -DCMAKE_MAKE_PROGRAM=E:/android-ndk-r21d/prebuilt/windows-x86_64/bin/make.exe -DANDROID_PLATFORM=android-21 -DANDROID_ABI=arm64-v8a -DCOMMON_LIBRARY_DIRS=E:/Research/Source/Repos/Excalibur/cmake/build/lib
4. make

### 未完待续