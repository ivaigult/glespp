project(xxx)

add_subdirectory(dependencies)

include(compile_assets/compile_assets.cmake)
compile_assets(assets "${CMAKE_CURRENT_LIST_DIR}/demos/assets")

add_library(glespp INTERFACE)
target_include_directories(glespp INTERFACE glespp)

function(add_demo name)
    add_executable(${name} demos/${name}.cpp)
    target_link_libraries(${name} glespp glfw glad glm gli assets)
endfunction()

add_demo(triangle)
add_demo(texture)