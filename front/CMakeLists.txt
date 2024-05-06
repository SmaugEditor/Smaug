
set( SOURCES 

	main.cpp
	smaugapp.cpp

	shadermanager.cpp 
	texturemanager.cpp
	modelmanager.cpp

	meshrenderer.cpp
	worldrenderer.cpp 
	basicdraw.cpp

	filesystem.cpp 

	interfaceimpl.cpp
)

set( VERTEX_SHADERS shaders/vs_preview_view.sc shaders/vs_model.sc shaders/vs_grid.sc shaders/vs_line.sc )
set( FRAGMENT_SHADERS shaders/fs_preview_view.sc shaders/fs_model.sc shaders/fs_grid.sc shaders/fs_line.sc )

set( SHADER_OUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders" )
set( GLSL_VERSION 430 )
set( DX_MODEL 5_0 )

if(APPLE)
    set( SHADER_PLATFORMS metal )
elseif(UNIX)
    set( SHADER_PLATFORMS glsl spirv )
else()
    set( SHADER_PLATFORMS glsl dx11 spirv )
endif()

add_custom_target( invalidate_shaders )

foreach(VS ${VERTEX_SHADERS})
    get_filename_component(VS_PATH "${VS}" ABSOLUTE)

    add_shader("${VS_PATH}" VERTEX
        OUTPUT "${SHADER_OUT_DIR}"
        GLSL_VERSION ${GLSL_VERSION}
        DX11_MODEL ${DX_MODEL} 
        PLATFORMS ${SHADER_PLATFORMS})

		
    add_custom_command(TARGET invalidate_shaders PRE_BUILD
        COMMAND "${CMAKE_COMMAND}" -E touch "${VS_PATH}")
endforeach()


foreach(FS ${FRAGMENT_SHADERS})
    get_filename_component(FS_PATH "${FS}" ABSOLUTE)

    add_shader("${FS_PATH}" FRAGMENT
        OUTPUT "${SHADER_OUT_DIR}"
        GLSL_VERSION ${GLSL_VERSION}
        DX11_MODEL ${DX_MODEL} 
        PLATFORMS ${SHADER_PLATFORMS})
		 
    add_custom_command(TARGET invalidate_shaders PRE_BUILD
        COMMAND "${CMAKE_COMMAND}" -E touch "${FS_PATH}")
		
endforeach()

include_directories( . )
add_executable( front ${SOURCES} ${VERTEX_SHADERS} ${FRAGMENT_SHADERS} shaders/varying.def.sc )
target_include_directories( front PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} assimp sshared smesh sworld smaug bigg lodepng )
target_link_libraries( front sshared smesh sworld smaug bigg lodepng assimp)
set_target_properties( front PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	CXX_STANDARD 20
)


if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options(front PRIVATE /Zc:preprocessor)
endif()

install(DIRECTORY ${SHADER_OUT_DIR} DESTINATION bin)


