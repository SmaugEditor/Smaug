

set( SOURCES 
	raytest.cpp
	mesh.cpp
	tessellate.cpp
	slice.cpp
	meshtest.cpp
	meshedit.cpp
)


include_directories( . )
add_library( smesh STATIC ${SOURCES} )
target_include_directories( smesh PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} sshared )
target_link_libraries( smesh sshared glm )

set_target_properties( smesh PROPERTIES
	CXX_STANDARD 20
)

if ( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
	target_compile_options(smesh PRIVATE /Zc:preprocessor)
endif()



