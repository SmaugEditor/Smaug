cmake_minimum_required( VERSION 3.0 )

project( smaug )
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_subdirectory( thirdparty/bigg )

add_subdirectory( Smaug )