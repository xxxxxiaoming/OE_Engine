cmake_minimum_required(VERSION 3.15)

# 默认设置 vcpkg 工具链路径（解决依赖问题）
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/../vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
endif()

# 设置工程名称，此名称会生成对应的解决方案文件 OE_Engine.sln
project(OE_Engine C CXX)

# 开启文件夹归类功能（这会将 ALL_BUILD 和 ZERO_CHECK 收纳到一个虚拟文件夹中）
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 查找通过 vcpkg 安装的依赖库
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(Stb REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)
find_package(mikktspace CONFIG REQUIRED)

# ====================================================================================
# 1. 引擎项目：OE_Engine (.vcxproj)
# ====================================================================================
# 收集引擎代码
file(GLOB_RECURSE ENGINE_SOURCES "OE_Engine/src/*.cpp" "OE_Engine/src/*.c")
file(GLOB_RECURSE ENGINE_HEADERS "OE_Engine/src/*.h" "OE_Engine/src/*.hpp")

# GLAD 的代码路径
set(GLAD_SOURCES "Dependences/glad/src/glad.c")

# 定义静态库目标（生成 OE_Engine.vcxproj）
add_library(OE_Engine STATIC 
    ${ENGINE_SOURCES} 
    ${ENGINE_HEADERS}
    ${GLAD_SOURCES}
)

# 包含头文件目录（PUBLIC 使依赖此库的应用程序也能使用这些包含路径）
target_include_directories(OE_Engine PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/OE_Engine/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/Dependences/glad/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/Dependences/gli"
    "${Stb_INCLUDE_DIR}"
)

# 链接第三方库
target_link_libraries(OE_Engine PUBLIC 
    glfw
    glm::glm
    imgui::imgui
    assimp::assimp
    mikktspace::mikktspace
)

# macOS 特有配置：链接系统内置的 OpenGL 框架
if (APPLE)
    find_library(OPENGL_FRAMEWORK OpenGL)
    target_link_libraries(OE_Engine PRIVATE ${OPENGL_FRAMEWORK})
endif()

# Windows 特有配置：隐式链接 OpenGL
if (WIN32)
    target_link_libraries(OE_Engine PRIVATE opengl32)
endif()

# ====================================================================================
# 2. 应用程序项目：Simple_Render_Application (.vcxproj)
# ====================================================================================
# 收集应用代码
file(GLOB_RECURSE APP_SOURCES "Simple_Render_Application/src/*.cpp" "Simple_Render_Application/src/*.c")
file(GLOB_RECURSE APP_HEADERS "Simple_Render_Application/src/*.h" "Simple_Render_Application/src/*.hpp")

# 定义可执行文件目标（生成 Simple_Render_Application.vcxproj）
add_executable(Simple_Render_Application 
    ${APP_SOURCES} 
    ${APP_HEADERS}
)

# 将应用程序链接到引擎库
target_link_libraries(Simple_Render_Application PRIVATE OE_Engine)

# 将工作目录设置为应用所在的源码目录，方便程序调试时读取 res 文件夹下的资源
set_property(TARGET Simple_Render_Application PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Simple_Render_Application")
