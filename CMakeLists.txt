# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Set the project name and specify C++ standard
project(CMakeTestProject VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Notify about the current platform build
if(UNIX AND NOT APPLE)
    message(STATUS "Detected Linux")
elseif(APPLE)
    message(STATUS "Detected macOS")
elseif(WIN32)
    message(STATUS "Detected Windows")
else()
    message(STATUS "Unknown Platform...")
endif()

function(add_source_group_from_directory dir)
    file(GLOB_RECURSE files "${dir}/*")

    foreach(file ${files})
        # Get the relative path of the file
        get_filename_component(relative_path "${file}" REALPATH)
        string(REPLACE "${CMAKE_SOURCE_DIR}/" "" relative_path "${relative_path}")

        # Get the directory portion of the relative path
        get_filename_component(dir_name "${relative_path}" DIRECTORY)

        # Replace slashes with backslashes for Visual Studio
        string(REPLACE "/" "\\" group_name "${dir_name}")

        # Create a group based on the directory path
        source_group("${group_name}" FILES "${file}")
    endforeach()
endfunction()

#Set default build type to Release if none is specified
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type was set - Autoselected \"Release\"")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (default is Release)" FORCE)
endif()

# Options for GLEW configuration
option(BUILD_UTILS "Build the glewinfo and visualinfo utilities" OFF)
option(glew-cmake_BUILD_SHARED "Build GLEW as a shared library" OFF)
option(glew-cmake_BUILD_STATIC "Build GLEW as a static library" ON)

# Add dependencies as subdirectories
add_subdirectory(dependencies/glew-2.1.0)
add_subdirectory(dependencies/glfw-3.4)

# Gather all .cpp files in the src directory
file(GLOB_RECURSE SOURCES "src/*.cpp" "src/*.h")

# Define the executable target without WIN32 for Debug mode
add_executable(${PROJECT_NAME} ${SOURCES})

# Platform-specific include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    src/core
    src/core/log
    src/vendor
    dependencies/glew-2.1.0/include
    dependencies/glfw-3.4/include
)

# Link dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE GLEW glfw)

# Platform-specific library linking
if(WIN32)
    message(STATUS "Linking additional Windows-specific libraries")
    target_link_libraries(${PROJECT_NAME} PRIVATE opengl32 User32 gdi32 Shell32)
elseif(UNIX)
    message(STATUS "Linking additional Unix-specific libraries")
    target_link_libraries(${PROJECT_NAME} PRIVATE GL X11 pthread) # Add common libraries for Linux
else()
    message(STATUS "No additional libraries linked!") 
endif()


# Define compile definitions and options
target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG_MODE>              # Enable DEBUG_MODE macro in Debug mode
    $<$<CONFIG:Release>:NDEBUG>                # Enable NDEBUG macro in Release mode
)

# Windows-specific settings for console/GUI applications in Release mode
if(WIN32)
    # Set WIN32_EXECUTABLE to avoid a console window in Release mode on Windows
    set_target_properties(${PROJECT_NAME} PROPERTIES
        $<$<CONFIG:Release>:WIN32_EXECUTABLE> TRUE
    )

    # Define WIN_MAIN to switch to WinMain entry point on Windows in Release mode
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:WIN_MAIN>
    )
endif()

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4>             # MSVC-specific compile options
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>  # GCC-specific compile options
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>  # Clang-specific compile options
)

#Platform-specific compile and link options
if(MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        set_target_properties(${PROJECT_NAME} PROPERTIES
            WIN32_EXECUTABLE TRUE
            LINK_FLAGS "/SUBSYSTEM:WINDOWS"
        )
    endif()
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if(WIN32)
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:-mwindows>
        )
    endif()
endif()

# Set output directories
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${CMAKE_BINARY_DIR}/bin
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${CMAKE_BINARY_DIR}/bin
)

# Only create source groups if using Visual Studio
if(MSVC)
    add_source_group_from_directory("${CMAKE_SOURCE_DIR}/src")
endif()

# Set up resource folder copying
set(RESOURCE_DIR ${CMAKE_SOURCE_DIR}/res)
set(DESTINATION_DIR ${CMAKE_BINARY_DIR}/bin/res)

add_custom_command(
    TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${RESOURCE_DIR} ${DESTINATION_DIR}
    COMMENT "Copying resource files to the build directory"
)