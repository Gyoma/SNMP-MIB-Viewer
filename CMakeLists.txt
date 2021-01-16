cmake_minimum_required(VERSION 3.10)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PROJECT_NAME netsnmp-parser)
project(${PROJECT_NAME})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INLCUDE_CURRENT_DIR ON)

set(CMAKE_PREFIX_PATH "$ENV{QtPath}")

find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

set(PROJECT_UI
	src/ui/MIBBrowserWindow.ui
	src/ui/MIBManagmentDialog.ui
)

set(PROJECT_FILES
	include/GlobalTypes.h
	include/Module.h
	include/Node.h
	include/Parser.h
	include/TCList.h
	include/TreeModel.h
	include/MIBBrowserWindow.h
	include/MIBManagmentDialog.h
	src/main.cpp
	src/Module.cpp
	src/Node.cpp
	src/Parser.cpp
	src/TCList.cpp
	src/TreeModel.cpp
	src/MIBBrowserWindow.cpp
	src/MIBManagmentDialog.cpp
)

add_executable(
	${PROJECT_NAME}
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
