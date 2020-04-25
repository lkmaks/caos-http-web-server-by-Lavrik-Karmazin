//
// Created by max on 4/24/20.
//

#include "EpollContext.h"
#include <unistd.h>
#include "http_parser.h"


HttpEpollContext* InitialHttpEpollContext::HandleReadEvent() {
  int old_data_size = data_.size();

  int chunk_size = config_.read_chunk_size;
  char buf[chunk_size];
  int cnt_read = read(fd_, buf, chunk_size);
  while (cnt_read != -1) {
    for (int i = 0; i < cnt_read; ++i) {
      data_.push_back(buf[i]);
    }
    cnt_read = read(fd_, buf, chunk_size);
  }
  auto pos = data_.find("\r\n", std::max(0, old_data_size));
  if (pos != data_.npos) {
    HttpFirstLine first_line;
    if (is_ok_http_firstline(data_, pos, config_.http_methods, first_line)) {
      // method is already checked to be on the list of possible methods
      if (first_line.method == "GET") {
        // transition into GET Epoll context
        auto *context = new GetHttpEpollContext(fd_, config_, first_line.uri, data_, pos);
        return context;
      }
    }
  }
  return this;
}

HttpEpollContext* InitialHttpEpollContext::HandleWriteEvent() {
  // do nothing since we are just reading first line
}

HttpEpollContext* GetHttpEpollContext::HandleReadEvent() {

}

HttpEpollContext* GetHttpEpollContext::HandleWriteEvent(){

}