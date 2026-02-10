cmake_minimum_required(VERSION 3.30)

project(rme)

find_package(GLUT REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenGL REQUIRED)
find_package(wxWidgets COMPONENTS base core aui gl adv REQUIRED)

if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
	add_compile_options(-Wno-deprecated-enum-enum-conversion)
endif ()

include(${wxWidgets_USE_FILE})
include(source/CMakeLists.txt)
add_executable(${PROJECT_NAME} WIN32 ${rme_H} ${rme_SRC})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:__DEBUG__>)

include_directories(
    ${OPENGL_INCLUDE_DIR}
    ${GLUT_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    ${wxWidgets_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    ${ZLIB_LIBRARIES}
)
