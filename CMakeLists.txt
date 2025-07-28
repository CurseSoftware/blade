cmake_minimum_required(VERSION 3.20.0)
project(blade VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizerRel" "RelWithDebInfo")
endif()

# If your LSP does not find the path to your local libs/includes, you can use something like this
# to force the compile_flags.json to direct an include path to it
# set(CMAKE_CXX_FLAGS "-Wall -I/usr/include/c++/11 -I/usr/include/x86_64-linux-gnu/c++/11")

# Source files for the assembly
# NOTE: you may need to add subfolders directly here
#       I prefer explicitly writing them to GLOB_RECURSE
#       I use the .cc notation for C++ source files
file(GLOB ASSEMBLY_SOURCES
    "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/*.cc"
    "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/core/*.cc"
    "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/resources/*.cc"
    "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/window/*.cc"
    "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/gfx/*.cc"
    "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/gfx/vulkan/*.cc"
)

file(GLOB ASSEMBLY_HEADERS
    "${PROJECT_SOURCE_DIR}/${PROJECT_NAME}/*.h"
)

# Build dependencies if you have any

# Set the include path
set(ASSEMBLY_INCLUDE_DIR 
    "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}"
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(X11 REQUIRED)
    if (NOT X11_FOUND)
        message(FATAL_ERROR "X11 Not Found")
    endif()
endif()


add_library(${PROJECT_NAME} SHARED ${ASSEMBLY_SOURCES} ${ASSEMBLY_HEADERS})

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan Vulkan::Headers)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries("${PROJECT_NAME}" ${X11_LIBRARIES} ${X11_Xmu_LIB})
    target_include_directories("${PROJECT_NAME}" PRIVATE ${X11_INCLUDE_DIR})
    # target_link_libraries("${PROJECT_NAME}" PRIVATE X11)
    # target_link_libraries("${PROJECT_NAME}" PRIVATE XRM)
endif()

target_include_directories(${PROJECT_NAME}
    PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${ASSEMBLY_INCLUDE_DIR}"
    # include any external dependency include directories here as well...
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:BLADE_DEBUG>
    $<$<CONFIG:Release>:BLADE_RELEASE>
)


#Generate compiler commands for using clangd LSP
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

# Build examples
# Remember to link any libraries you might need
add_subdirectory(apps/window)
add_subdirectory(apps/gfx_simple)
