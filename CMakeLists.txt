cmake_minimum_required(VERSION 3.16.0)

project( "bitonic")

if(UNIX)
set( CMAKE_CXX_FLAGS "-std=c++0x")
endif(UNIX)

find_package( Boost 1.50.0 REQUIRED COMPONENTS thread date_time system)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    add_definitions( "-DHAS_BOOST" )
endif()

set( SRCS ./bitonic.cpp)
set( HDRS ./Actor.h ./MessageQueue.h)

link_directories ( ${Boost_LIBRARY_DIRS} )
include_directories ( ${Boost_INCLUDE_DIRS} )

add_executable( bitonic ${SRCS} ${HDRS} )

target_link_libraries( bitonic ${Boost_LIBRARIES})
