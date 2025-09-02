
# lib/CMakeLists.txt

# Add GLAD (static)
add_library(glad STATIC
    ${CMAKE_SOURCE_DIR}/dependencies/glad/src/glad.c
)
 set_target_properties(glad PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(glad PUBLIC
    ${CMAKE_SOURCE_DIR}/dependencies/glad/include
)

# Find OpenGL and SDL3
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)

# Gather library sources
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/EnvHazGraphics/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/EnvHazGraphics/include/*.hpp"
)

# Define EnvHazGraphics (shared or static)
add_library(EnvHazGraphics ${LIB_SOURCES})
target_include_directories(EnvHazGraphics PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/EnvHazGraphics/include"
    ${SDL3_INCLUDE_DIRS}
)
target_compile_definitions(EnvHazGraphics PUBLIC
    RESOURCES_PATH="${CMAKE_SOURCE_DIR}/assets/"
    EXTERNAL_LIB_PATH="${CMAKE_SOURCE_DIR}/dependencies/"
)
target_compile_definitions(EnvHazGraphics PRIVATE BUILDING_EHAZGAPI)
target_link_libraries(EnvHazGraphics PUBLIC
    glad
    OpenGL::GL
    ${SDL3_LIBRARIES}
)
