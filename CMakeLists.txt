cmake_minimum_required(VERSION 3.10)
project(ImGuiChatbot)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenGL
find_package(OpenGL REQUIRED)

# GLFW options
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Wayland and X11 options
option(GLFW_BUILD_WAYLAND "Build GLFW with Wayland support" ON)
option(GLFW_BUILD_X11 "Build GLFW with X11 support" ON)

# Add GLFW
add_subdirectory(external/glfw)

# Add GLAD
add_library(glad STATIC external/glad/src/glad.c)
target_include_directories(glad PUBLIC external/glad/include)

# Add ImGui source files
set(IMGUI_DIR external/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# Create executable
add_executable(ImGuiChatbot main.cpp ${IMGUI_SOURCES})

# Include directories
target_include_directories(ImGuiChatbot PRIVATE
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${OPENGL_INCLUDE_DIR}
    external/glad/include
)

# Link libraries
target_link_libraries(ImGuiChatbot PRIVATE
    glfw
    ${OPENGL_LIBRARIES}
    glad
)

# Add compile definitions for ImGui
target_compile_definitions(ImGuiChatbot PRIVATE
    IMGUI_IMPL_OPENGL_LOADER_GLAD
)

# Path to the font file
set(FONT_FILE "${CMAKE_SOURCE_DIR}/external/fonts/Inter-Regular.ttf")

# Add custom command to copy the font to the build directory
add_custom_command(
    TARGET ImGuiChatbot POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${FONT_FILE}" "$<TARGET_FILE_DIR:ImGuiChatbot>/Inter-Regular.ttf"
)

# Define the font path relative to the executable
target_compile_definitions(ImGuiChatbot PRIVATE
    IMGUI_FONT_PATH="Inter-Regular.ttf"
)