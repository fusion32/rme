cmake_minimum_required(VERSION 3.30)

project(rme)

find_package(GLUT REQUIRED)
find_package(ZLIB REQUIRED)
find_package(OpenGL REQUIRED)
find_package(wxWidgets COMPONENTS base core aui gl adv REQUIRED)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU|Clang")
	add_compile_options(
		-Werror
		-Wall
		-Wpedantic
		-Wextra
		-Wno-unused-parameter
		-Wno-nested-anon-types
		-Wsuggest-override
	)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# TODO(fusion): Enable MSVC warnings.
	#add_compile_options(-W3 -WX)
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

target_precompile_headers(${PROJECT_NAME} PRIVATE source/main.h)

