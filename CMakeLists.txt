cmake_minimum_required(VERSION 3.2)

project(obs-transition-matrix)

##########################################
# find libobs                            #
##########################################
include(external/FindLibObs.cmake)
find_package(LibObs REQUIRED)

##########################################
# QT support                             #
##########################################
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)

# Find includes in corresponding build directories
set(CMAKE_PREFIX_PATH "${QTDIR}")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(obs-transition-matrix_HEADERS
	obs-transition-matrix.hpp
	)

set(obs-transition-matrix_SOURCES
	obs-transition-matrix.cpp
	)

include_directories(
	${LIBOBS_INCLUDE_DIR}
	"${LIBOBS_INCLUDE_DIR}/../UI/obs-frontend-api"
	${Qt5Core_INCLUDES}
	${Qt5Widgets_INCLUDES}
)

add_library(obs-transition-matrix MODULE
	${obs-transition-matrix_HEADERS}
	${obs-transition-matrix_SOURCES}
	)

target_link_libraries(obs-transition-matrix
	${OBS_FRONTEND_LIB}
	${LIBOBS_LIB}
	Qt5::Core
	Qt5::Widgets
	)

# --- Windows-specific build settings and tasks ---
if(WIN32)
	if(CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(ARCH_NAME "64bit")
		set(OBS_BUILDDIR_ARCH "build64")
	else()
		set(ARCH_NAME "32bit")
		set(OBS_BUILDDIR_ARCH "build32")
	endif()

	# --- Release package helper ---
	# The "release" folder has a structure similar OBS' one on Windows
	set(RELEASE_DIR "${PROJECT_SOURCE_DIR}/release")

	add_custom_command(TARGET obs-transition-matrix POST_BUILD
		# If config is Release, package release files
		COMMAND if $<CONFIG:Release>==1 (
		"${CMAKE_COMMAND}" -E make_directory
		"${RELEASE_DIR}/data/obs-plugins/obs-transition-matrix"
		"${RELEASE_DIR}/obs-plugins/${ARCH_NAME}")

		COMMAND if $<CONFIG:Release>==1 ("${CMAKE_COMMAND}" -E copy_directory
		"${PROJECT_SOURCE_DIR}/data"
		"${RELEASE_DIR}/data/obs-plugins/obs-transition-matrix")

		# If config is RelWithDebInfo, package release files
		COMMAND if $<CONFIG:RelWithDebInfo>==1 (
		"${CMAKE_COMMAND}" -E make_directory
		"${RELEASE_DIR}/data/obs-plugins/obs-transition-matrix"
		"${RELEASE_DIR}/obs-plugins/${ARCH_NAME}")

		COMMAND if $<CONFIG:RelWithDebInfo>==1 ("${CMAKE_COMMAND}" -E copy_directory
		"${PROJECT_SOURCE_DIR}/data"
		"${RELEASE_DIR}/data/obs-plugins/obs-transition-matrix")

		COMMAND if $<CONFIG:RelWithDebInfo>==1 ("${CMAKE_COMMAND}" -E copy
		"$<TARGET_PDB_FILE:obs-transition-matrix>"
		"${RELEASE_DIR}/obs-plugins/${ARCH_NAME}")

		# Copy to obs-studio dev environment for immediate testing
		COMMAND if $<CONFIG:Debug>==1 (
		"${CMAKE_COMMAND}" -E copy
		"$<TARGET_FILE:obs-transition-matrix>"
		"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/obs-plugins/${ARCH_NAME}")

		COMMAND if $<CONFIG:Debug>==1 (
		"${CMAKE_COMMAND}" -E copy
		"$<TARGET_PDB_FILE:obs-transition-matrix>"
		"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/obs-plugins/${ARCH_NAME}")

		COMMAND if $<CONFIG:Debug>==1 (
		"${CMAKE_COMMAND}" -E make_directory
		"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/data/obs-plugins/obs-transition-matrix")

		COMMAND if $<CONFIG:Debug>==1 (
		"${CMAKE_COMMAND}" -E copy_directory
		"${PROJECT_SOURCE_DIR}/data"
		"${LIBOBS_INCLUDE_DIR}/../${OBS_BUILDDIR_ARCH}/rundir/$<CONFIG>/data/obs-plugins/obs-transition-matrix")
		)
	# --- End of sub-section ---
endif()
# --- End of section ---

# --- Linux-specific build settings and tasks ---
if(UNIX AND NOT APPLE)
	set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-unused-function -Werror-implicit-function-declaration -Wno-missing-field-initializers ${CMAKE_CXX_FLAGS} -std=c++11 -fno-strict-aliasing")
	set(CMAKE_C_FLAGS "-Wall -Wextra -Wno-unused-function -Werror-implicit-function-declaration -Wno-missing-braces -Wno-missing-field-initializers ${CMAKE_C_FLAGS} -std=gnu99 -fno-strict-aliasing")

	set_target_properties(obs-transition-matrix PROPERTIES PREFIX "")

	file(GLOB locale_files data/locale/*.ini)

	if(NOT CUSTOM)
		install(TARGETS obs-transition-matrix LIBRARY DESTINATION
			"${CMAKE_INSTALL_PREFIX}/lib/obs-plugins")
		install(FILES ${locale_files} DESTINATION
			"${CMAKE_INSTALL_PREFIX}/share/obs/obs-plugins/obs-transition-matrix/locale")
	else()
		install(TARGETS obs-transition-matrix LIBRARY DESTINATION
			"${CMAKE_INSTALL_PREFIX}/rundir/Debug/obs-plugins/64bit")
		install(FILES ${locale_files} DESTINATION
			"${CMAKE_INSTALL_PREFIX}/rundir/Debug/data/obs-plugins/obs-transition-matrix/locale")
	endif()
endif()
# --- End of section ---

# -- OS X specific build settings and tasks --
# -- Help with OS X build is need as i don't --
# -- have any hardware to test. --
if(APPLE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -fvisibility=default")

	set_target_properties(obs-transition-matrix PROPERTIES PREFIX "")
endif()
# -- End of section --
