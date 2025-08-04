cmake_minimum_required(VERSION 3.28)
project(VK_tutorial)
set(CMAKE_BUILD_TYPE Debug)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_BUILD_TYPE Debug)

find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 3.4 REQUIRED)

FetchContent_Declare(tinyobjloader
        GIT_REPOSITORY "https://github.com/tinyobjloader/tinyobjloader"
        GIT_TAG "v2.0.0rc13"
)

FetchContent_Declare(stb
        GIT_REPOSITORY "https://github.com/nothings/stb"
        GIT_TAG "f58f558c120e9b32c217290b80bad1a0729fbb2c"
        FIND_PACKAGE_ARGS
)
FetchContent_MakeAvailable(tinyobjloader stb)
FetchContent_GetProperties(stb)


add_subdirectory(shaders)

add_executable(VK_tutorial main.cpp)

target_link_libraries(VK_tutorial Vulkan::Vulkan)
target_link_libraries(VK_tutorial glm::glm)
target_link_libraries(VK_tutorial tinyobjloader)
target_include_directories(VK_tutorial PRIVATE ${stb_SOURCE_DIR})
target_link_libraries(VK_tutorial glfw)

add_dependencies(VK_tutorial shaders)
