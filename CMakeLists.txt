cmake_minimum_required(VERSION 3.18)

project(Facial-Attendence-Lambda)

#set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

set(SRCS
    src/main.cpp
    src/DBConnection.cpp
)

include_directories(include/)

find_package(Boost REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )
find_package(OpenSSL REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_executable(falambda ${SRCS})
target_link_libraries( falambda OpenSSL::SSL fmt::fmt)
target_compile_features( falambda PRIVATE cxx_std_17)
