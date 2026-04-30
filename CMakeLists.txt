cmake_minimum_required(VERSION 3.15)
project(OpenGL_Engine C CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 查找通过 vcpkg 安装的依赖库
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(stb CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(assimp CONFIG REQUIRED)

# 收集引擎代码
file(GLOB_RECURSE ENGINE_SOURCES "OpenGL_Engine/src/*.cpp" "OpenGL_Engine/src/*.c")
file(GLOB_RECURSE ENGINE_HEADERS "OpenGL_Engine/src/*.h" "OpenGL_Engine/src/*.hpp")

# GLAD 的代码路径
set(GLAD_SOURCES "Dependences/glad/src/glad.c")

# 收集应用代码
file(GLOB_RECURSE APP_SOURCES "Simple_Particle_Application/src/*.cpp" "Simple_Particle_Application/src/*.c")
file(GLOB_RECURSE APP_HEADERS "Simple_Particle_Application/src/*.h" "Simple_Particle_Application/src/*.hpp")

# 定义可执行文件
add_executable(SimpleParticleApp 
    ${ENGINE_SOURCES} 
    ${ENGINE_HEADERS}
    ${APP_SOURCES} 
    ${APP_HEADERS}
    ${GLAD_SOURCES}
)

# 包含头文件目录
target_include_directories(SimpleParticleApp PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/OpenGL_Engine/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/Dependences/glad/include"
)

# 链接第三方库
target_link_libraries(SimpleParticleApp PRIVATE 
    glfw
    glm::glm
    stb::stb
    imgui::imgui
    assimp::assimp
)

# macOS 特有配置：链接系统内置的 OpenGL 框架
if (APPLE)
    find_library(OPENGL_FRAMEWORK OpenGL)
    target_link_libraries(SimpleParticleApp PRIVATE ${OPENGL_FRAMEWORK})
endif()

# Windows 特有配置：隐式链接 OpenGL
if (WIN32)
    target_link_libraries(SimpleParticleApp PRIVATE opengl32)
endif()
