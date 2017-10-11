option(USE_FILE_SYSTEM "Disables asset compilation" ON)

set(compiler_dir ${CMAKE_CURRENT_LIST_DIR})

function(compile_assets tgt_name assets_folder)
	set(sources ${compiler_dir}/include/assets.h)
    if (USE_FILE_SYSTEM)
		list(APPEND sources ${compiler_dir}/file_storage.cpp)
    else()
		# Run some python shit
	endif()

	add_library(${tgt_name} STATIC ${sources})
	target_include_directories(${tgt_name} PUBLIC ${compiler_dir}/include)
	if (USE_FILE_SYSTEM)
		target_compile_definitions(${tgt_name} PUBLIC "-DASSETS_ROOT=\"${assets_folder}\"")
	endif()
endfunction()
