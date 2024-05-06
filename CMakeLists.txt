cmake_minimum_required( VERSION 3.9 )

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build )


set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)

project( smaug )

set(BIGG_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BIGG_ASSET_GEN ON CACHE BOOL "" FORCE)

add_subdirectory( thirdparty/bigg )
add_subdirectory( thirdparty/portable-file-dialogs )
add_subdirectory( thirdparty/keyvalues )
add_subdirectory( thirdparty/assimp )
add_subdirectory( thirdparty/lodepng.cmake )

include_directories( thirdparty )

add_subdirectory( sshared )
add_subdirectory( smesh )
add_subdirectory( sworld )
add_subdirectory( Smaug )
add_subdirectory( front )

set(CMAKE_XCODE_GENERATE_SCHEME OFF)
set_target_properties(front PROPERTIES XCODE_GENERATE_SCHEME ON)

if ( APPLE )
    target_compile_definitions(bgfx PUBLIC BGFX_CONFIG_MULTITHREADED=0)
endif()


if ( CMAKE_CXX_COMPILER_ID MATCHES ".*Clang" )
    set_target_properties(assimp PROPERTIES COMPILE_FLAGS "-Wno-shorten-64-to-32 -Wno-deprecated -Wno-unused -Wno-ordered-compare-function-pointers")
endif()