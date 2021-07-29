
set( SOURCES 

	baseview.cpp

	doodle.cpp
	debugdraw.cpp

	3dview.cpp
	uiview.cpp
	editview.cpp 
	selectedview.cpp 

	cursor.cpp 
	cameramarker.cpp

	utils.cpp 
	transform.cpp

	toolbox.cpp 
	basetool.cpp 
	nodetools.cpp 
	selectionmanager.cpp

	baseexporter.cpp 
	objexporter.cpp 
	vmfexporter.cpp 

	svar.cpp
	svarex.h
	settingsmenu.cpp

	texturebrowser.cpp

	grid.cpp
	
	input.cpp

	editorinterface.cpp
)


include_directories( . )
add_library( smaug STATIC ${SOURCES} )
target_include_directories( smaug PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} sshared smesh sworld )
target_link_libraries( smaug keyvalues sshared smesh sworld glm imgui )
set_target_properties( smaug PROPERTIES
	VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	CXX_STANDARD 20
)


if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options(smaug PRIVATE /Zc:preprocessor)
endif()


