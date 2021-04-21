cmake_minimum_required(VERSION 3.16.2)

project( "bitonic")

if(UNIX)
set( CMAKE_CXX_FLAGS "-std=c++0x")
endif(UNIX)

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "C:/Libraries/boost_1_65_1")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "C:/Libraries/boost_1_65_1/lib64-msvc-14.1")

find_package( Boost 1.65.1 REQUIRED COMPONENTS thread date_time system)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    add_definitions( "-DHAS_BOOST" )
endif()

set( SRCS ./bitonic.cpp)
set( HDRS ./Actor.h ./MessageQueue.h)

link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( ${Boost_INCLUDE_DIRS} )

add_executable( bitonic ${SRCS} ${HDRS} )

target_link_libraries( bitonic ${Boost_LIBRARIES})
