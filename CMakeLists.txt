cmake_minimum_required(VERSION 3.10)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PROJECT_NAME MIB-Viewer)
project(${PROJECT_NAME})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INLCUDE_CURRENT_DIR ON)

set(CMAKE_PREFIX_PATH "$ENV{QtPath_5_15_2}")

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

set(PROJECT_RSC
	src/resource/res.qrc
)

set(PROJECT_UI
	src/ui/MIBViewerWindow.ui
	src/ui/MIBManagerDialog.ui
)

set(PROJECT_FILES
	src/GlobalTypes.h
	src/Module.h
	src/Node.h
	src/Parser.h
	src/TreeModel.h
	src/MIBViewerWindow.h
	src/MIBManagerDialog.h
	src/main.cpp
	src/Module.cpp
	src/Node.cpp
	src/Parser.cpp
	src/TreeModel.cpp
	src/MIBViewerWindow.cpp
	src/MIBManagerDialog.cpp
)

add_executable(
	${PROJECT_NAME} WIN32
	${PROJECT_RSC}
	${PROJECT_UI}
	${PROJECT_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC 
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_link_libraries(${PROJECT_NAME} 
	PUBLIC 
	Qt5::Core
	Qt5::Gui
	Qt5::Widgets
)

#if(WIN32)
#	add_custom_command(
#		TARGET ${PROJECT_NAME} POST_BUILD
#		COMMAND ${CMAKE_COMMAND} -E copy_if_different
#			$<TARGET_FILE:Qt5::Core>
#			$<TARGET_FILE:Qt5::Gui>
#			$<TARGET_FILE:Qt5::Widgets>
#			$<TARGET_FILE_DIR:${PROJECT_NAME}>
#	)
#endif(WIN32)