cmake_minimum_required(VERSION 3.10.0)
project(scenelab VERSION 0.1.0 LANGUAGES C CXX)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always -W -Wall -Wextra -Wpedantic")

    # tries to improve compile time
    string(ASCII 27 Esc)

    find_program(MOLD_EXECUTABLE mold)
    if(MOLD_EXECUTABLE)
        message(STATUS "${Esc}[32mYIPPEE!!${Esc}[0m | mold found")
        set(CMAKE_EXE_LINKER_FLAGS "-fuse-ld=mold")
    endif()

    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        message(STATUS "${Esc}[32mYIPPEE!!${Esc}[0m | ccache found")
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    endif()
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS True)

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

# ImGuiFileDialog
set(IMGUIFILEDIALOG_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/ImGuiFileDialog)
set(IMGUIFILEDIALOG_SOURCES
    ${IMGUIFILEDIALOG_DIR}/ImGuiFileDialog.cpp
)
include_directories(${IMGUIFILEDIALOG_DIR})


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
add_executable(scenelab ${SOURCES} ${IMGUI_SOURCES} ${IMGUIZMO_SOURCES} ${IMGUIFILEDIALOG_SOURCES})

target_include_directories(scenelab SYSTEM PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/external/stb_image/include
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    find_package(X11 REQUIRED)
    find_package(Threads REQUIRED)

    target_link_libraries(scenelab PRIVATE
        glfw
        glad
        glm
        ${X11_LIBRARIES}
        Threads::Threads
    )
else()
    target_link_libraries(scenelab glfw glad glm)
endif()
