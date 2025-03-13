# ========================== GLFW ==========================
# GLFW has its own CMake file that exposes "glfw" as a static library
add_subdirectory(glfw)

# ========================== GLEW ==========================
# Configuration: Disable shared libraries and utility builds
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Disable shared libraries")  # Seems to not prevent shared build completely
set(BUILD_UTILS OFF CACHE BOOL "Disable building glewinfo and visualinfo" FORCE)
set(GLEW_NO_GLU ON CACHE BOOL "Disable GLU support" FORCE)

# GLEW has a large CMake file
add_subdirectory(glew/build/cmake)

# Ensure the correct include directories for GLEW
target_compile_definitions(glew_s PRIVATE GLEW_STATIC)
target_include_directories(glew_s INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/glew/include>)

# ========================== IMGUI ==========================
# Create a static library for ImGui with OpenGL and GLFW backends.
add_library(imgui STATIC
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/backends/imgui_impl_glfw.cpp
    imgui/backends/imgui_impl_opengl3.cpp
)

# Include the core and backend directories for ImGui.
target_include_directories(imgui PUBLIC imgui imgui/backends)

# Link against GLFW (Glew is not needed here if the main program links it).
target_link_libraries(imgui PRIVATE glfw)

# ========================== GLM ==========================
# Configuration: Enable fast math and allow output messages
set(GLM_QUIET OFF CACHE BOOL "Disable building silently" FORCE)
set(GLM_ENABLE_FAST_MATH ON CACHE BOOL "Enable GLM fast math" FORCE)

# GLM has its own CMake file exposing "glm" as a static library
add_subdirectory(glm)

# ========================== STB ==========================
# Expose stb_image as a static library
add_library(stb_image STATIC libs/stb_image.cpp)

# Include the stb header directory
target_include_directories(stb_image PUBLIC stb)
