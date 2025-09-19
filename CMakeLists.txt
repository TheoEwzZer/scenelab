cmake_minimum_required(VERSION 3.20)

project(scenelab LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/external)

include(FetchContent)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw.git
  GIT_TAG 3.4
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
  glad
  GIT_REPOSITORY https://github.com/Dav1dde/glad.git
  GIT_TAG v2.0.8
)
FetchContent_MakeAvailable(glad)

find_package(Python3 REQUIRED)
execute_process(
  COMMAND ${Python3_EXECUTABLE} -m pip install glad2
  RESULT_VARIABLE PIP_RESULT
)
if(NOT PIP_RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to install glad2")
endif()

execute_process(
  COMMAND ${Python3_EXECUTABLE} -m glad --api gl:core=3.3 --out-path ${CMAKE_BINARY_DIR}/glad c
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  RESULT_VARIABLE GLAD_RESULT
)
if(NOT GLAD_RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to generate GLAD files")
endif()

add_library(glad STATIC
  ${CMAKE_BINARY_DIR}/glad/src/gl.c
)
target_include_directories(glad PUBLIC ${CMAKE_BINARY_DIR}/glad/include)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1
)
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glm)

add_executable(scenelab
  src/main.cpp
)

target_include_directories(scenelab PRIVATE ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(scenelab PRIVATE
  glfw
  glad
  glm::glm
  imgui
  imgui_backend
)

if(UNIX)
  find_package(X11 REQUIRED)
  target_link_libraries(scenelab PRIVATE dl pthread X11::X11)
endif()

if(MSVC)
  target_compile_options(scenelab PRIVATE $<$<CONFIG:Debug>:/W4 /permissive->)
else()
  target_compile_options(scenelab PRIVATE $<$<CONFIG:Debug>:-Wall -Wextra -Wpedantic>)
endif()

FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG v1.92.3
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

add_library(imgui_backend STATIC
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui_backend PUBLIC
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
  ${CMAKE_BINARY_DIR}/glad/include
)
target_link_libraries(imgui_backend PUBLIC imgui glfw glad)
target_compile_definitions(imgui_backend PUBLIC "IMGUI_IMPL_OPENGL_LOADER_GLAD")
