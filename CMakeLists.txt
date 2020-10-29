cmake_minimum_required(VERSION 3.1)
project( sample )
find_package( OpenCV REQUIRED )

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

configure_file(libuvc/libuvc_config.h.in
  libuvc/libuvc_config.h
  @ONLY
)

 find_package(Threads)
 find_package(LibUSB)
 find_package(JpegPkg QUIET)

include_directories( ${OpenCV_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/libuvc)
add_executable( sample sample.cpp )


target_link_libraries( sample ${OpenCV_LIBS} ${PROJECT_SOURCE_DIR}/libuvc/libuvc.a Threads::Threads LibUSB::LibUSB JPEG::JPEG)
