
add_library(imgui_render STATIC
	imgui_render.cpp
	imgui_render.hpp
)

target_include_directories(imgui_render PUBLIC .)

target_link_libraries(imgui_render 
    glespp 
    imgui 
	glfw 
	glad
	glm
)