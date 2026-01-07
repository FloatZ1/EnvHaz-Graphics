cmake_minimum_required(VERSION 3.21)

# -----------------------------
# GLAD (static)
# -----------------------------
add_library(glad STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/src/glad.c
)

set_target_properties(glad PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_include_directories(glad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glad/include
)

# -----------------------------
# GLM
# -----------------------------
add_subdirectory(dependencies/glm/glm)

# -----------------------------
# Dependencies
# -----------------------------
find_package(OpenGL REQUIRED)
find_package(assimp CONFIG QUIET)

if (NOT assimp_FOUND)
    find_package(assimp REQUIRED)
endif()
find_package(Boost REQUIRED COMPONENTS serialization)

# -----------------------------
# SDL3 (cross-platform)
# -----------------------------
find_package(SDL3 CONFIG QUIET)

if (NOT SDL3_FOUND)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(SDL3 REQUIRED sdl3)

    add_library(SDL3::SDL3 INTERFACE IMPORTED)
    target_include_directories(SDL3::SDL3 INTERFACE ${SDL3_INCLUDE_DIRS})
    target_link_libraries(SDL3::SDL3 INTERFACE ${SDL3_LIBRARIES})
endif()

# -----------------------------
# EnvHazGraphics sources
# -----------------------------
file(GLOB_RECURSE LIB_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/EnvHazGraphics/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/EnvHazGraphics/include/*.hpp"
)

# -----------------------------
# EnvHazGraphics library
# -----------------------------
add_library(EnvHazGraphics ${LIB_SOURCES})

target_include_directories(EnvHazGraphics PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/EnvHazGraphics/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/dependencies/glm"
)

target_compile_definitions(EnvHazGraphics PUBLIC
    RESOURCES_PATH="${CMAKE_SOURCE_DIR}/app/assets/"
    EXTERNAL_LIB_PATH="${CMAKE_SOURCE_DIR}/dependencies/"
)
if(WIN32)
    target_compile_definitions(EnvHazGraphics PUBLIC PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(EnvHazGraphics PUBLIC PLATFORM_MAC)
elseif(UNIX)
    target_compile_definitions(EnvHazGraphics PUBLIC PLATFORM_LINUX)
endif()
target_compile_definitions(EnvHazGraphics PRIVATE
    BUILDING_EHAZGAPI
)

target_link_libraries(EnvHazGraphics PUBLIC
    glad
    OpenGL::GL
    SDL3::SDL3
    glm::glm
    assimp::assimp
    Boost::serialization
)
