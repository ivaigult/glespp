
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)
set(GLFW_INSTALL OFF)
set(GLFW_VULKAN_STATIC OFF)
set(GLFW_DOCUMENT_INTERNALS OFF)

add_subdirectory(glfw)

add_library(glad STATIC glfw/deps/glad.c)
target_include_directories(glad PUBLIC glfw/deps)

add_subdirectory(glm)

add_library(gli INTERFACE)
target_include_directories(gli INTERFACE gli)

set(BUILD_SHARED_LIBS  OFF)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
add_subdirectory(assimp)
target_include_directories(assimp PUBLIC assimp/include $<TARGET_PROPERTY:assimp,BINARY_DIR>/../include)

add_library(imgui STATIC
	imgui/imconfig.h
	imgui/imgui.cpp
	imgui/imgui.h
	imgui/imgui_demo.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_internal.h
	imgui/stb_rect_pack.h
	imgui/stb_textedit.h
	imgui/stb_truetype.h
)

target_include_directories(imgui PUBLIC imgui)