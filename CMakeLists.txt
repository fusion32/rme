cmake_minimum_required(VERSION 3.30)

project(rme)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

find_package(GLUT REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenGL REQUIRED)
find_package(wxWidgets COMPONENTS html aui gl adv core net base REQUIRED)

find_package(nlohmann_json CONFIG REQUIRED)

# LibArchive disabled in compilation level by default, see "#define OTGZ_SUPPORT" in the "definitions.h" file
# If you need use, enable this:
#find_package(LibArchive REQUIRED)
#${LibArchive_INCLUDE_DIRS} ${LibArchive_LIBRARIES}

#if(APPLE)
#    set(CMAKE_PREFIX_PATH /usr/local/opt/libarchive)
#endif()

if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
	add_compile_options(-Wno-deprecated-enum-enum-conversion)
endif ()

include(${wxWidgets_USE_FILE})
include(source/CMakeLists.txt)
add_executable(${PROJECT_NAME} ${rme_H} ${rme_SRC})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD_REQUIRED ON)

include_directories(
    ${LibArchive_INCLUDE_DIRS}
    ${OPENGL_INCLUDE_DIR}
    ${GLUT_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME}
    ${wxWidgets_LIBRARIES}
    ${LibArchive_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${GLUT_LIBRARIES}
    ${ZLIB_LIBRARIES}
    nlohmann_json::nlohmann_json
)
