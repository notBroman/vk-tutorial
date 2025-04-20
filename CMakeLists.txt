cmake_minimum_required(VERSION 3.28)
project(VK_tutorial)

include(FetchContent)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 3.4 REQUIRED)

FetchContent_Declare(tinyobjloader
        GIT_REPOSITORY "https://github.com/tinyobjloader/tinyobjloader"
        GIT_TAG "v2.0.0rc13"
)
FetchContent_MakeAvailable(tinyobjloader)

add_subdirectory(shaders)

add_executable(VK_tutorial main.cpp)

target_link_libraries(VK_tutorial Vulkan::Vulkan)
target_link_libraries(VK_tutorial glm::glm)
target_link_libraries(VK_tutorial tinyobjloader)
target_link_libraries(VK_tutorial glfw)

add_dependencies(VK_tutorial shaders)
