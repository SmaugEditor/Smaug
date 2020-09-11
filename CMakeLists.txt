cmake_minimum_required( VERSION 3.0 )

project( smaug )
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BIGG_EXAMPLES OFF)
set(BIGG_ASSET_GEN ON)

add_subdirectory( thirdparty/bigg )
add_subdirectory( thirdparty/portable-file-dialogs )
add_subdirectory( thirdparty/keyvalues )

include_directories( thirdparty )
add_subdirectory( Smaug )