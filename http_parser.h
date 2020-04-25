//
// Created by max on 4/25/20.
//

#ifndef CAOS_HTTP_WEB_SERVER_HTTP_PARSER_H
#define CAOS_HTTP_WEB_SERVER_HTTP_PARSER_H

#include <string>

struct HttpFirstLine {
    std::string method;
    std::string uri;
};

bool is_substr(const std::string &t, const std::string &s, int pos) {
  // is t a susbtr of s from position pos in s
  if (pos < 0 || pos >= s.size()) {
    return false;
  }
  if (pos + t.size() > s.size()) {
    return false;
  }

  for (int i = 0; i < t.size(); ++i) {
    if (t[i] != s[pos + i]) {
      return false;
    }
  }

  return true;
}

bool is_okay_uri(const std::string &uri) {
  return true; // for now
}

bool is_ok_http_firstline(const std::string &data, int end_pos,
        std::vector<std::string> &http_methods, HttpFirstLine &first_line) {
  bool ok_start = false;
  std::string right_method;
  for (auto &method : http_methods) {
    if (is_substr(method, data, 0)) {
      right_method = method;
    }
  }
  if (right_method.empty()) {
    return false;
  }

  std::string ending("HTTP/1.1");
  if (!is_substr(ending, data, (int)data.size() - (int)ending.size())) {
    return false;
  }

  int uri_len = end_pos - (int)(right_method.size() + ending.size() + 2);
  if (uri_len <= 0 ||
      data[right_method.size()] != ' ' ||
      data[data.size() - ending.size() - 1] != ' ') {
    return false;
  }

  std::string uri = data.substr(right_method.size() + 1, end_pos - ending.size());
  if (!is_okay_uri(uri)) {
    return false;
  }

  first_line.method = right_method;
  first_line.uri = uri;

  return true;
}


#endif //CAOS_HTTP_WEB_SERVER_HTTP_PARSER_H
