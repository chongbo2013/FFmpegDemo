# CMake的最低版本
cmake_minimum_required(VERSION 3.4.1)

add_library( JniVideoPlayer
             SHARED
        src/main/cpp/JniVideoPlayer.cpp
             src/main/cpp/cgeVideoDecoder.cpp
             src/main/cpp/cgeVideoPlayer.cpp
        )

find_library( log-lib
              log )

set(JNI_LIBS_DIR ${CMAKE_SOURCE_DIR}/src/main/ffmpeg)

add_library(avutil
            SHARED
            IMPORTED )
set_target_properties(avutil
                      PROPERTIES IMPORTED_LOCATION
                      ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavutil.so )

add_library(swresample
            SHARED
            IMPORTED )
set_target_properties(swresample
                      PROPERTIES IMPORTED_LOCATION
                      ${JNI_LIBS_DIR}/${ANDROID_ABI}/libswresample.so )

add_library(swscale
            SHARED
            IMPORTED )
set_target_properties(swscale
                      PROPERTIES IMPORTED_LOCATION
                      ${JNI_LIBS_DIR}/${ANDROID_ABI}/libswscale.so )

add_library(avcodec
            SHARED
            IMPORTED )
set_target_properties(avcodec
                      PROPERTIES IMPORTED_LOCATION
                      ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavcodec.so )

add_library(avformat
            SHARED
            IMPORTED )
set_target_properties(avformat
                      PROPERTIES IMPORTED_LOCATION
                      ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavformat.so )

add_library(avfilter
            SHARED
            IMPORTED )
set_target_properties(avfilter
                      PROPERTIES IMPORTED_LOCATION
                      ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavfilter.so )

add_library(avdevice
            SHARED
            IMPORTED )
set_target_properties(avdevice
                      PROPERTIES IMPORTED_LOCATION
                      ${JNI_LIBS_DIR}/${ANDROID_ABI}/libavdevice.so )

add_library(yuv
            SHARED
            IMPORTED )
set_target_properties(yuv
                      PROPERTIES IMPORTED_LOCATION
                      ${CMAKE_SOURCE_DIR}/src/main/libyuv/${ANDROID_ABI}/libyuv.so )

include_directories(src/main/ffmpeg/include
                    src/main/libyuv/include)

target_link_libraries(  JniVideoPlayer
                        avutil
                        swresample
                        swscale
                        avcodec
                        avformat
                        avfilter
                        avdevice
                        yuv
                        android
                        ${log-lib})