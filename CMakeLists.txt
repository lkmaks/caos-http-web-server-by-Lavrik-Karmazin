cmake_minimum_required(VERSION 3.10)
project(caos-http-web-server)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -DLOCAL")

add_executable(caos-http-web-server main.cpp Config.h Utils.h HttpServer.cpp HttpServer.h
        ThreadPool.h ThreadPool.cpp Epoll.h Epoll.cpp EpollContext.cpp EpollContext.h ConnectionQueue.h Utils.cpp HttpParser.h ThreadMain.cpp ThreadMain.h HttpParser.cpp debug.h)
