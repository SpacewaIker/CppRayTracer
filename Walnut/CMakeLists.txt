cmake_minimum_required(VERSION 3.10)
project(Walnut CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Vulkan REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(${PROJECT_NAME} PRIVATE src/Walnut/ImGui ${Vulkan_INCLUDE_DIRS})

file(GLOB_RECURSE ${PROJECT_NAME}_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/**.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/**.h
)

add_library(${PROJECT_NAME} ${${PROJECT_NAME}_SRC})
target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARIES})
