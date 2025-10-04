
cmake_minimum_required(VERSION 3.19.2)
project(EnvHazGraphics)
set(CMAKE_CXX_STANDARD 20)
 set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
option(BUILD_SHARED_LIBS "Build shared instead of static libraries" ON)
 add_compile_definitions(PROJECT_ROOT_DIR="${CMAKE_SOURCE_DIR}")
# Add subdirectories
add_subdirectory(lib)
add_subdirectory(app)
