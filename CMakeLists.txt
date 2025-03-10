cmake_minimum_required(VERSION 3.10)

project(TooManyBlocks VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Ensure CMAKE_BUILD_TYPE is set for single-config generators
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type" FORCE)
endif()

# Gather all .cpp files in the src directory
file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(${PROJECT_NAME} ${SOURCES})

# Enable DEBUG_MODE macro in Debug mode
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:DEBUG_MODE>)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    src/core
    src/core/log
    src/vendor
    dependencies/glew-2.1.0/include
    dependencies/glfw-3.4/include
)

# Add dependencies as subdirectories
option(BUILD_UTILS "Build the glewinfo and visualinfo utilities" OFF)
option(glew-cmake_BUILD_SHARED "Build GLEW as a shared library" OFF)
option(glew-cmake_BUILD_STATIC "Build GLEW as a static library" ON)
target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)
add_subdirectory(dependencies/glew-2.1.0)

add_subdirectory(dependencies/glfw-3.4)
target_link_libraries(${PROJECT_NAME} PRIVATE GLEW glfw)

# Additional configuration based on the current platform
if(UNIX AND NOT APPLE)
    message(STATUS "Detected Linux - Linking additional Unix-specific libraries")
    target_link_libraries(${PROJECT_NAME} PRIVATE GL X11 pthread)
elseif(WIN32)
    message(STATUS "Detected Windows - Linking additional Windows-specific libraries")
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32 glu32 User32 gdi32 Shell32)

    # In Release mode, define WinMain Entrypoint to get rid of console window
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        message(STATUS "Release Build: Enabling Windows GUI subsystem (No console)")
        # Properties for WinMain Entrypoint
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Release>:WIN_MAIN>)
            set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
        elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-mwindows")
        endif()
    else()
        message(STATUS "Building as default subsystem")
    endif()
else()
    message(STATUS "Unknown Platform... no specific configuration done!")
endif()

# Set output directories
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/bin
)

# Set up resource folder copying
set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/res)
set(DESTINATION_DIR ${CMAKE_BINARY_DIR}/bin/res)

add_custom_command(
    TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${RESOURCE_DIR} ${DESTINATION_DIR}
    COMMENT "Copying resource files to the build directory"
)