

set( SOURCES 
	shared.cpp	
	log.cpp
)


include_directories( . )
add_library( sshared STATIC ${SOURCES} )
target_include_directories( sshared PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )
target_link_libraries( sshared glm )

set_target_properties( sshared PROPERTIES
	CXX_STANDARD 20
)

if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options(sshared PRIVATE /Zc:preprocessor)
endif()



