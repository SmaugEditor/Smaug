cmake_minimum_required( VERSION 3.1 )

set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/build )
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)

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
