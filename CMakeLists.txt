cmake_minimum_required(VERSION 3.24)
set(PROJECT_NAME MediaServer)
project(MediaServer)
set(CMAKE_CXX_STANDARD 20)

set(INC_DIR
        D:/MediaServer/ffmpeg/include
        D:/MediaServer/SDL2/x86_64-w64-mingw32/include
        D:/MediaServer/boost)

set(LINK_DIR
        D:/MediaServer/ffmpeg/lib
        D:/MediaServer/SDL2/x86_64-w64-mingw32/lib
        D:/MediaServer/boost/stage/lib)

include_directories(${INC_DIR})
link_directories(${LINK_DIR})

#link_libraries(boost_json-vc143-mt-gd-x64-1_85)

add_executable(MediaServer main.cpp MediaDecoder.cpp MediaDecoder.h CommonException.cpp CommonException.h YuvRender.cpp YuvRender.h Audio.cpp Audio.h Timer.cpp Timer.h)

target_link_libraries(MediaServer mingw32 SDL2main SDL2 avdevice avcodec avformat swscale avutil swresample)
