
set(ASSIMP_NO_EXPORT ON)

set( SOURCES 

	main.cpp
	smaugapp.cpp

	baseview.cpp

	shadermanager.cpp 
	texturemanager.cpp
	modelmanager.cpp
	basicdraw.cpp 
	debugdraw.cpp

	3dview.cpp
	uiview.cpp
	editview.cpp 
	selectedview.cpp 

	cursor.cpp 

	log.cpp
	utils.cpp 

	raytest.cpp
	mesh/mesh.cpp
	mesh/tessellate.cpp
	mesh/slice.cpp
	mesh/meshtest.cpp
	mesh/meshedit.cpp
	meshrenderer.cpp

	worldeditor.cpp 
	worldrenderer.cpp 
	worldsave.cpp

	actionmanager.cpp 
	editoractions.cpp 
	toolbox.cpp 
	basetool.cpp 
	nodetools.cpp 

	filesystem.cpp 
	baseexporter.cpp 
	objexporter.cpp 
	vmfexporter.cpp 

	svar.cpp
	svarex.h
	settingsmenu.cpp

	texturebrowser.cpp

	grid.cpp
	

	input.cpp
)

set( VERTEX_SHADERS shaders/vs_preview_view.sc shaders/vs_model.sc shaders/vs_grid.sc shaders/vs_line.sc )
set( FRAGMENT_SHADERS shaders/fs_preview_view.sc shaders/fs_model.sc shaders/fs_grid.sc shaders/fs_line.sc )

set( SHADER_OUT_DIR "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders" )
set( GLSL_VERSION 430 )
set( DX_MODEL 5_0 )

if(UNIX)
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
add_executable( smaug ${SOURCES} ${VERTEX_SHADERS} ${FRAGMENT_SHADERS} shaders/varying.def.sc )
target_include_directories( smaug PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} assimp mesh )
target_link_libraries( smaug bigg keyvalues assimp lodepng )
set_target_properties( smaug PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	CXX_STANDARD 20
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT smaug)

if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options(smaug PRIVATE /Zc:preprocessor)
endif()

install(DIRECTORY ${SHADER_OUT_DIR} DESTINATION bin)


