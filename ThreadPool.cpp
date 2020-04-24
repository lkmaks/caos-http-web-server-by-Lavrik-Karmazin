#include "ThreadPool.h"
#include "Utils.h"

ThreadPool::ThreadPool(int cnt_threads) {
  threads_.resize(cnt_threads);
  pthread_attr_t thread_attr;
  pthread_attr_init(&thread_attr);
  for (int i = 0; i < cnt_threads; ++i) {
    pthread_create(&threads_[i].thread, &thread_attr, thread_main, &threads_[i]);
  }
  pthread_attr_destroy(&thread_attr);
  srand(99);
}

void ThreadPool::SendNewConn(int thread_id, const Connection &new_conn){
  write_all(threads_[thread_id].channel[0], (void*)&new_conn, sizeof(new_conn));
}

void ThreadPool::SendNewConnToAnyThread(const Connection &new_conn) {
  SendNewConn(rand() % threads_.size(), new_conn);
}