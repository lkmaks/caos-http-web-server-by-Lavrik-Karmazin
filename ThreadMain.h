#ifndef CAOS_HTTP_WEB_SERVER_THREADMAIN_H
#define CAOS_HTTP_WEB_SERVER_THREADMAIN_H

#include <queue>
#include "Epoll.h"
#include "ThreadPool.h"
#include "ConnectionQueue.h"
#include "Config.h"
#include "HttpServer.h"
#include <string.h>
#include <signal.h>


extern thread_local sig_atomic_t sigpipe_flag;

void sigpipe_signal_handler(int signum);

void sigpipe_protection();


class HttpServer;

Config &GetConf(ThreadData *thread_data);


void HandleEvent(EpollEvent &event);

void *thread_main(void *ptr);

#endif //CAOS_HTTP_WEB_SERVER_THREADMAIN_H
