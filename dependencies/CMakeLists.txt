
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