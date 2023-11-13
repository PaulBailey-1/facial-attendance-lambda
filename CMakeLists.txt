cmake_minimum_required(VERSION 3.18)

project(Facial-Attendence)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(SRCS
    src/main.cpp
    src/DBConnection.cpp
)

include_directories(include/)

find_package(Boost REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
find_package(OpenSSL REQUIRED)

add_executable(fa ${SRCS})
target_link_libraries( fa OpenSSL::SSL) #Boost::boost ${Boost_LIBRARIES} )
target_compile_features(fa PRIVATE cxx_std_17)
