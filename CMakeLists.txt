project(xxx)

set(CMAKE_CXX_STANDARD 11)

add_subdirectory(dependencies)

include(compile_assets/compile_assets.cmake)
compile_assets(assets "${CMAKE_CURRENT_LIST_DIR}/demos/assets")

add_library(glespp INTERFACE)
target_include_directories(glespp INTERFACE glespp glm)

add_subdirectory(imgui_render)

function(add_demo name)
    add_executable(${name} demos/${name}.cpp)
    target_link_libraries(${name} glespp glfw glm glad assets assimp imgui imgui_render)
endfunction()

add_demo(triangle)
add_demo(texture)
add_demo(cube)
add_demo(phong)
