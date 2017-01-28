cmake_minimum_required(VERSION 2.8)

project( HazeRemoval )
set(OpenCV_DIR "/home/lenovo/opencv-2.4.9/release")
find_package( OpenCV REQUIRED )

include_directories(${PROJECT_SOURCE_DIR}/include)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
add_library(${PROJECT_NAME} SHARED src/HazeRemover.cpp)
target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
add_executable(hazeRemovalBin src/main.cpp)
target_link_libraries( hazeRemovalBin ${OpenCV_LIBS} ${PROJECT_NAME})
