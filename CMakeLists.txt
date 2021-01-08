cmake_minimum_required(VERSION 3.10)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PROJECT_NAME netsnmp-parser)
project(${PROJECT_NAME})

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else()
	set(CMAKE_CXX_STAANDARD 17)
endif(MSVC)

set(CMAKE_INLCUDE_CURRENT_DIR ON)

set(PROJECT_FILES
	include/GlobalTypes.h
	include/Module.h
	include/Node.h
	include/Parser.h
	include/TCList.h
	include/Tree.h
	#src/main.cpp
	src/Module.cpp
	src/Node.cpp
	src/Parser.cpp
	src/TCList.cpp
	src/Tree.cpp
)

add_executable(
	${PROJECT_NAME} 
	${PROJECT_RESOURCES}
	${PROJECT_UI}
	${PROJECT_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC 
	"${CMAKE_CURRENT_SOURCE_DIR}/include"
)
