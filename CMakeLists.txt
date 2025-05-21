cmake_minimum_required(VERSION 3.25.1)
# Define the data directory path
project(ml_from_scratch_cpp)
add_definitions(-DDATA_DIR="${CMAKE_SOURCE_DIR}/data/")
# Add include directory to the include path
include_directories(include)
# Create the executables
add_subdirectory(src)
