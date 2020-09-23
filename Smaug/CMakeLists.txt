set( SOURCES 

	main.cpp
	smaugapp.cpp

	baseview.cpp

	shadermanager.cpp 
	texturemanager.cpp
	basicdraw.cpp 

	3dview.cpp
	uiview.cpp
	editview.cpp 
	selectedview.cpp 

	cursor.cpp 

	utils.cpp 

	worldeditor.cpp 
	worldrenderer.cpp 

	actionmanager.cpp 
	editoractions.cpp 
	toolbox.cpp 
	basetool.cpp 
	nodetools.cpp 

	filesystem.cpp 
	baseexporter.cpp 
	objexporter.cpp 
	vmfexporter.cpp 

)

set( VERTEX_SHADERS shaders/vs_preview_view.sc )
set( FRAGMENT_SHADERS shaders/fs_preview_view.sc )

set( SHADER_OUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders" )
set( GLSL_VERSION 430 )
set( DX_MODEL 5_0 )
set( SHADER_PLATFORMS glsl dx11 )


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
add_executable( smaug ${SOURCES} ${VERTEX_SHADERS} ${FRAGMENT_SHADERS} )
target_include_directories( smaug PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} )
target_link_libraries( smaug bigg keyvalues )


install(DIRECTORY ${SHADER_OUT_DIR} DESTINATION bin)