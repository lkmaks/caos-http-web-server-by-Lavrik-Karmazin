//
// Created by max on 4/24/20.
//

#include "EpollContext.h"
#include <unistd.h>
#include "Utils/HttpParser.h"
#include "ThreadMain.h"
#include <algorithm>
#include <sstream>
#include <sys/socket.h>
#include <iostream>
#include "Utils/Debug.hpp"



HttpEpollContext::HttpEpollContext(Connection &conn, Epoll &epoll, Config &config) :
        conn_(conn), epoll_(epoll), config_(config) {
  type_ = HTTP;
  is_read_stage_ = true;
  InitReadFields();
}

void HttpEpollContext::InitReadFields() {
  first_line_ready_ = false;
  first_line_end_ = -1;
  data_ptr_ = 0;
  first_line_ = HttpFirstLine();
}

void HttpEpollContext::InitWriteFields(const std::string &write_data) {
  write_data_ = write_data;
  write_data_ptr_ = 0;
}



// ---------------- utility methods ---------------------------------

void HttpEpollContext::DestroyContext() {
  epoll_.RemoveFileDescriptor(conn_.sock);
  close(conn_.sock); // supposedly auto-removes socket from epoll but we ensure that
  delete this;
}

int HttpEpollContext::ReadSome() {
  // reads more bytes from socket into data_
  // returns -1, if no data available
  // otherwise returns number of new bytes (0 indicates that socket has closed)

  int chunk_size = config_.read_chunk_size;
  char buf[chunk_size];
  int cnt = read(conn_.sock, buf, chunk_size);
  if (cnt == -1) {
    return -1;
  }
  else {
    for (int i = 0; i < cnt; ++i) {
      data_.push_back(buf[i]);
    }
    return cnt;
  }
}

void HttpEpollContext::TransitToError(int error_no) {
  if (error_no == 400) {
    TransitToWrite("HTTP/1.1 400 Bad Request\r\n\r\n");
  }
  else if (error_no == 404) {
    TransitToWrite("HTTP/1.1 404 Not Found\r\n\r\n");
  }
  else if (error_no == 405) {
    TransitToWrite("HTTP/1.1 405 Method Not Allowed\r\n\r\n");
  }
  else if (error_no == 501) {
    TransitToWrite("HTTP/1.1 501 Not Implemented\r\n\r\n");
  }
}

void HttpEpollContext::TransitToWrite(const std::string &str) {
  InitWriteFields(str);
  Rearm();
  is_read_stage_ = false;
}

void HttpEpollContext::Rearm() {
  epoll_.Rearm(conn_.sock, EPOLLIN | EPOLLOUT, (EpollContext*)this);
}

// ------------------------------------------------------------------


void HttpEpollContext::HandleRead() {
  if (!is_read_stage_) {
    return;
  }

  if (!first_line_ready_) {
    while (true) {
      while (data_ptr_ < data_.size() && !is_http_end_of_line(data_, data_ptr_)) {
        ++data_ptr_;
      }

      if (data_ptr_ < data_.size()) {
        // found end of first line
        if (is_ok_http_firstline(data_, data_ptr_ - 1, config_.http_methods, first_line_)) {
          // first_line_ is filled by the call
          first_line_ready_ = true;
          first_line_end_ = data_ptr_;
          break;
        }
        else {
          // invalid first line of HTTP query
          TransitToError(400);
          return;
        }
      }

      int cnt = ReadSome();
      if (cnt == -1) {
        // no data available on socket right now
        return;
      }
      else if (cnt == 0) {
        // writing side has closed the connection
        DestroyContext();
        return;
      }
    }
  }



  // first line is ready and correct; first_line_ field is filled

  if (first_line_.method == "GET") {
    while (true) {
      while (data_ptr_ < data_.size() && !is_http_end_of_headers(data_, data_ptr_)) {
        ++data_ptr_;
      }
      if (data_ptr_ < data_.size()) {
        // found end of headers
        if (is_ok_header_section(data_, first_line_end_ + 1, data_ptr_ - 3, request_headers_)) {
          // headers are filled
          break;
        }
        else {
          TransitToError(400);
        }
      }

      int cnt = ReadSome();
      if (cnt == -1) {
        // no data available on socket right now
        return;
      }
      else if (cnt == 0) {
        // writing side has closed the connection
        DestroyContext();
        return;
      }
    }

    // now headers are scanned
    // since it is GET query, no body will be scanned
    // we will parse headers and transit to writing mode
    // with data depending on gathered data from request
    ParseRequestHeaders();
  }
}

void HttpEpollContext::ParseRequestHeaders() {
  // here we should transit to write mode with correct write_data_
  if (!request_headers_.dict.count("Host")) {
    TransitToError(400);
    return;
  }

  std::string host = request_headers_.dict["Host"];
  std::string hostname, port_str;
  auto pos = host.find(':');
  if (pos == std::string::npos) {
    hostname = host;
    port_str = "80";
  }
  else {
    hostname = host.substr(0, pos);
    port_str = host.substr(pos + 1, host.size());
  }

  if (!(is_ok_hostname(port_str) && is_natural_number(port_str))) {
    TransitToError(400);
    return;
  }
  // TODO check length of port_str to avoid overflow leading to possible tolerancy to incorrectness
  uint16_t port = strtol(port_str.c_str(), nullptr, 10);
  auto it = std::find(config_.vhosts.begin(), config_.vhosts.end(), make_pair(hostname, port));
  if (it == config_.vhosts.end()) {
    TransitToError(404);
    return;
  }


  // hostname and port are determined (step 1 finished)

  std::string path = config_.data_dir + "/" + hostname + first_line_.uri;
  while (path.back() == '/') {
    path.pop_back();
  }
  deb("1");
  if (!is_ok_path(path)) {
    TransitToError(400);
    return;
  }
  deb("2");
  deb(path);
  if (!file_exists(path)) {
    TransitToError(404);
    return;
  }
  deb("3");
  if (!is_gettable_file(path, config_)) {
    TransitToError(405); // Method Not Allowed
    return;
  }

  // path leads to a gettable file on server (step 2 finished)

  if (!request_headers_.dict.count("Accept")) {
    TransitToError(400);
    return;
  }

  std::vector<std::string> mime_types;
  if (!is_ok_qvalue_list(request_headers_.dict["Accept"], mime_types)) {
    TransitToError(400);
    return;
  }
  // mime_types is filled
  // for now just check if there is a text/html mime type, since all gettable files
  // are now interpreted as text/html,
  if (std::find(mime_types.begin(), mime_types.end(), "text/html") == mime_types.end()) {
    TransitToError(501); // Not Implemented
  }

  // Accept header contains suitable MIME type (step 3 finished)

  if (request_headers_.dict.count("Accept-Charset")) {
    std::vector<std::string> charsets;
    if (!is_ok_qvalue_list(request_headers_.dict["Accept-Charset"], charsets)) {
      TransitToError(400);
      return;
    }
    // since all gettable files are interpreted as utf-8, check if it is on the list
    if (std::find(charsets.begin(), charsets.end(), "utf-8") == charsets.end() &&
            std::find(charsets.begin(), charsets.end(), "UTF-8") == charsets.end()) {
      TransitToError(501); // Not Implemented
      return;
    }
  }

  // Accept-Charset header is missing or contains appropriate charset (step 4 finished)

  std::string connection;
  if (request_headers_.dict.count("Connection")) {
    connection = request_headers_.dict["Connection"];
    if (connection != "close" && connection != "keep-alive") {
      TransitToError(400);
      return;
    }
  }

  // Connection header contains valid value (step 5 finished)

  // step 6 is for now absent since it is complicated to parse (If-Modified-Since)

  // Now we should form the response
  // Connection would be "close" for now
  std::ostringstream stream;
  stream << "HTTP/1.1 200 OK\r\n";
  stream << "Content-Type: text/html; charset=utf-8\r\n";
  stream << "Content-Length: " << file_size(path) << "\r\n";
  stream << "Connection: close\r\n\r\n";
  stream << file_contents(path);
  TransitToWrite(stream.str());
}

void HttpEpollContext::HandleWrite() {
  if (is_read_stage_) {
    return;
  }

  int chunk_size = config_.write_chunk_size;
  while (write_data_ptr_ < write_data_.size()) {
    int cnt = write(conn_.sock,
            write_data_.c_str() + write_data_ptr_,
            std::min(chunk_size, (int)write_data_.size() - write_data_ptr_));
    if (sigpipe_flag) {
      // write broke down because socket was closed on the other side and we received SIGPIPE
      sigpipe_flag = 0;
      DestroyContext();
    }
    else if (cnt == -1) {
      // can't write more data now
      return;
    }
    else if (cnt == 0) {
      // this is undefined and should not really happen
      // for now, lets rearm socket here and leave
      Rearm();
      return;
    }
    else {
      write_data_ptr_ += cnt;
    }
  }

  // we have written all the data of the request
  // for now (since Connection: close) lets close connection and destroy context
  shutdown(conn_.sock, SHUT_RDWR);
  DestroyContext();
}
