cmake_minimum_required(VERSION 3.5)

project(OrbitEngineEditor)

# ==== Qt ==== 
find_package(Qt5 COMPONENTS Core Widgets Gui REQUIRED)

set(CMAKE_AUTOMOC OFF) # For meta object compiler
set(CMAKE_AUTORCC OFF) # Resource files
set(CMAKE_AUTOUIC OFF) # UI files

# OrbitEngine
set(OE_EDITOR_INTERNAL ON CACHE BOOL "" FORCE)

add_subdirectory(../OrbitEngine OrbitEngine/)

# Editor
file(GLOB_RECURSE Editor_SRC "Code/*.cpp")
file(GLOB_RECURSE Editor_HEADERS "Code/*.hpp")
file(GLOB_RECURSE Editor_QT "Code/*.ui" "Resources/*.qrc")

include_directories(
	Code
)

add_executable(Editor ${Editor_SRC} ${Editor_HEADERS} ${Editor_QT})
target_link_libraries(Editor OrbitEngine)

# Copy Binaries
if(OE_BINARIES)
	add_custom_command(TARGET Editor POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy ${OE_BINARIES} ${CMAKE_BINARY_DIR}
	)
endif()

# Resources
OE_add_resources(Resources Editor)

# C++11
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
useCXX11(Editor)

set_target_properties(Editor PROPERTIES AUTOMOC ON AUTORCC ON AUTOUIC ON)
target_link_libraries (Editor Qt5::Widgets)