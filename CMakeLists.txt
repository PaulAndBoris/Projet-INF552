CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

PROJECT(Graphcut_Textures)

SET(OpenCV_STATIC OFF)
FIND_PACKAGE(OpenCV REQUIRED)
INCLUDE_DIRECTORIES(${OpenCV_INCLUDE_DIRS})

ADD_EXECUTABLE(gc_textures main.cpp Patcher.cpp Patcher.h RandomOffsetChooser.cpp RandomOffsetChooser.h maxflow/graph.cpp)

TARGET_LINK_LIBRARIES(gc_textures ${OpenCV_LIBS})
