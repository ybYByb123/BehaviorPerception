cmake_minimum_required(VERSION 3.10)
project(CabinBehaviorPerception)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED) # locate OpenCV in system
include_directories(OpenCV_INCLUDE_DIRS)

file(GLOB SRC_FILES "*.h" "*.cpp")
add_executable(${PROJECT_NAME} ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} -lpthread -lm)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})