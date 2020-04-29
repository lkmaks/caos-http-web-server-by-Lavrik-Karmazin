cmake_minimum_required(VERSION 3.10)
project(caos-http-web-server)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -DLOCAL")

add_executable(caos-http-web-server src/main.cpp src/Config.h src/Utils/GeneralUtils.h src/HttpServer.cpp
        src/HttpServer.h src/Utils/ThreadPool.h src/Utils/ThreadPool.cpp src/Utils/Epoll.h src/Utils/Epoll.cpp
        src/EpollContext.cpp src/EpollContext.h src/Utils/ConnectionQueue.hpp src/Utils/GeneralUtils.cpp
        src/Utils/HttpParser.h src/ThreadMain.cpp src/ThreadMain.h src/Utils/HttpParser.cpp src/Utils/Debug.hpp)
