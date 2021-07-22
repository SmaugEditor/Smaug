

set( SOURCES 
	editoractions.cpp
	worldeditor.cpp
	worldsave.cpp
	worldtest.cpp
	actionmanager.cpp
)


include_directories( . )
add_library( sworld STATIC ${SOURCES} )
target_include_directories( sworld PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} sshared smesh )
target_link_libraries( sworld sshared smesh glm keyvalues )

set_target_properties( sworld PROPERTIES
	CXX_STANDARD 20
)

if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options(sworld PRIVATE /Zc:preprocessor)
endif()



