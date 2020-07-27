cmake_minimum_required(VERSION 3.17)

project(LaplacianSelection)
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
include(FindThreads)
conan_basic_setup(TARGETS)
add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS) 
add_executable(LaplacianSelection main.cpp)
target_compile_features( LaplacianSelection PRIVATE cxx_std_20 )
target_link_libraries( LaplacianSelection Threads::Threads CONAN_PKG::opencv
  CONAN_PKG::boost)
