cmake_minimum_required(VERSION 3.14)
project(caos-http-web-server)

set(CMAKE_CXX_STANDARD 17)

add_executable(caos-http-web-server main.cpp Config.h Utils.h thread_main.cpp HTTPServer.cpp HTTPServer.h
        ThreadPool.h ThreadPool.cpp Epoll.h Epoll.cpp EpollContext.cpp EpollContext.h)