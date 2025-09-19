cmake_minimum_required(VERSION 3.10.0)
project(scenelab VERSION 0.1.0 LANGUAGES C CXX)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -Wpedantic")
endif()

set(CMAKE_CXX_STANDARD 20)
file(GLOB_RECURSE SOURCES src/**.cpp)


add_subdirectory(external/glfw)
add_subdirectory(external/glad)
add_subdirectory(external/glm)

# ImGui
set(IMGUI_DIR external/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)
include_directories(${IMGUI_DIR})


# ImGuizmo
set(IMGUIZMO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ImGuizmo)
set(IMGUIZMO_SOURCES
    ${IMGUIZMO_DIR}/ImGuizmo.cpp
)
include_directories(${IMGUIZMO_DIR})

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_SOURCE_DIR}/external/stb_image/include)
add_executable(scenelab ${SOURCES} ${IMGUI_SOURCES} ${IMGUIZMO_SOURCES})

target_link_libraries(scenelab glfw glad)
