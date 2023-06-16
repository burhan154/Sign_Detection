cmake_minimum_required(VERSION 3.2)

project(Project)

set(PROJECT_DIR ${CMAKE_SOURCE_DIR})
set(PROJECT_INCLUDE_DIR ${PROJECT_DIR}/include)
set(PROJECT_SOURCE_DIR ${PROJECT_DIR}/src)

set(PROJECT_SOURCE_DIR ${PROJECT_DIR}/src)
file(GLOB SOURCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)

set(PROJECT_SOURCES 
                    ${SOURCE_FILES}
		            ${PROJECT_SOURCE_DIR}/vc.c
                    ${PROJECT_SOURCE_DIR}/processes.cpp  
                    ${PROJECT_SOURCE_DIR}/CvControl.cpp     
                    )


find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

include_directories(${PROJECT_INCLUDE_DIR})

add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})