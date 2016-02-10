cmake_minimum_required(VERSION 2.4.7)

project( "bitonic")

if(UNIX)
set( CMAKE_CXX_FLAGS "-std=c++0x")
endif(UNIX)

find_package( Boost 1.43.0 REQUIRED COMPONENTS thread date_time system)

set( SRCS ./bitonic.cpp)
set( HDRS ./Actor.h ./MessageQueue.h)

link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( ${Boost_INCLUDE_DIRS} )

add_executable( bitonic ${SRCS} ${HDRS} )

target_link_libraries( bitonic ${Boost_LIBRARIES})
