
# app/CMakeLists.txt

file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp"
)

add_executable(MyApp ${APP_SOURCES})
target_include_directories(MyApp PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
target_link_libraries(MyApp PRIVATE EnvHazGraphics)
