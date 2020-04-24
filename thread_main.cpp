#include "Epoll.h"
#include "ThreadPool.h"

void *thread_main(void *ptr) {
  ThreadData *thread_data = (ThreadData*)ptr;
  Epoll thread_epoll();

}
