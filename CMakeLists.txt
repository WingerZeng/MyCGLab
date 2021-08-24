cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

# 项目命名
project(MyCGLab VERSION 0.9)

# force to C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(WIN32)
    add_definitions(-D _DISABLE_EXTENDED_ALIGNED_STORAGE)
	set(BUILD_WITH_STATIC_CRT false)
endif()

# .a和.lib的输出路径
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
 # .so的输出路径
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
 # 可执行与.dll的输出路径
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

set(CMAKE_CXX_FLAGS_RELESSE "/MD")
set(CMAKE_CXX_FLAGS_DEBUG "/MDd")

add_subdirectory(thirdparty/glog-master EXCLUDE_FROM_ALL)
add_subdirectory(thirdparty/opencv-master EXCLUDE_FROM_ALL)

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
# 添加主程序子目录
add_subdirectory(src)