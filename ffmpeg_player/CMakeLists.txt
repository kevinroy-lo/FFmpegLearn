cmake_minimum_required(VERSION 3.4.1)

find_library( log-lib
              log )

set(distribution_DIR ${CMAKE_SOURCE_DIR}/libs)

add_library( ffmpeg-player
             SHARED
             src/main/cpp/ffmpeg-player.cpp )

add_library( avcodec
             SHARED
             IMPORTED)
set_target_properties( avcodec
                       PROPERTIES IMPORTED_LOCATION
                       ${distribution_DIR}/armeabi/libavcodec-56.so)

add_library( avfilter
             SHARED
             IMPORTED)
set_target_properties( avfilter
                       PROPERTIES IMPORTED_LOCATION
                       ${distribution_DIR}/armeabi/libavfilter-5.so)

add_library( avformat
             SHARED
             IMPORTED)
set_target_properties( avformat
                       PROPERTIES IMPORTED_LOCATION
                       ${distribution_DIR}/armeabi/libavformat-56.so)

add_library( avutil
             SHARED
             IMPORTED)
set_target_properties( avutil
                       PROPERTIES IMPORTED_LOCATION
                       ${distribution_DIR}/armeabi/libavutil-54.so)

add_library( swresample
             SHARED
             IMPORTED)
set_target_properties( swresample
                       PROPERTIES IMPORTED_LOCATION
                       ${distribution_DIR}/armeabi/libswresample-1.so)

add_library( swscale
             SHARED
             IMPORTED)
set_target_properties( swscale
                       PROPERTIES IMPORTED_LOCATION
                       ${distribution_DIR}/armeabi/libswscale-3.so)

add_library( libyuv
             SHARED
             IMPORTED)
set_target_properties( libyuv
                       PROPERTIES IMPORTED_LOCATION
                       ${distribution_DIR}/armeabi/libyuv.so)

include_directories(libs/include/ffmpeg)

include_directories(libs/include/yuv)

target_link_libraries( ffmpeg-player
                       avcodec
                       avfilter
                       avformat
                       avutil
                       swresample
                       swscale
                       libyuv
                       -landroid
                       ${log-lib} )