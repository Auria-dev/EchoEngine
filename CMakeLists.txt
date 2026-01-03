cmake_minimum_required(VERSION 3.16..4.0)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

project("EchoEngine" VERSION 0.0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (UNIX AND NOT APPLE)
    include(CheckLinkerFlag)
    check_linker_flag(CXX "-fuse-ld=mold" LINKER_SUPPORTS_MOLD)
    if (LINKER_SUPPORTS_MOLD)
        add_link_options("-fuse-ld=mold")
    else()
        check_linker_flag(CXX "-fuse-ld=lld" LINKER_SUPPORTS_LLD)
        if (LINKER_SUPPORTS_LLD)
            add_link_options("-fuse-ld=lld")
        endif()
    endif()
endif()

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

add_subdirectory(vendor/glfw)

# GLAD
add_library(Glad vendor/glad/src/glad.c)
target_include_directories(Glad PUBLIC vendor/glad/include)

# GLM
add_subdirectory(vendor/glm)

# ImGui
set(IMGUI_DIR "vendor/imgui")
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp

    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

add_library(ImGui STATIC ${IMGUI_SOURCES})
target_include_directories(ImGui SYSTEM PUBLIC ${IMGUI_DIR} ${IMGUI_DIR}/backends)
target_link_libraries(ImGui PRIVATE glfw Glad)

# STB Image
set(STB_IMPL_FILE "${CMAKE_BINARY_DIR}/stb_image.cpp")
if(NOT EXISTS ${STB_IMPL_FILE})
    file(WRITE ${STB_IMPL_FILE} "#define STB_IMAGE_IMPLEMENTATION\n#include \"stb_image.h\"")
endif()

add_library(STBImage STATIC ${STB_IMPL_FILE})
target_include_directories(STBImage PUBLIC vendor/stb_image)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS 
    "src/*.h" 
    "src/*.cpp" 
    "src/*.c"
)

if (UNIX AND NOT APPLE)
    find_package(X11 REQUIRED)
endif()

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC src)

set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD ON)
target_precompile_headers(${PROJECT_NAME} PRIVATE
    <vector>
    <string>
    <iostream>
    <glm/glm.hpp>
    <glm/gtc/matrix_transform.hpp>
)

target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    glfw 
    Glad 
    glm::glm 
    ImGui
    STBImage
)

if (UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE 
        X11::X11
        ${CMAKE_DL_LIBS} 
        pthread
    )
endif()

add_custom_target(run
    COMMAND $<TARGET_FILE:${PROJECT_NAME}>
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    USES_TERMINAL
    COMMENT "Building and running ${PROJECT_NAME}..."
)
