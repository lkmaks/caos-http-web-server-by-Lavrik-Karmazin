#include <string>
#include <vector>
#include <map>
#include "HttpParser.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "GeneralUtils.h"
#include "Debug.hpp"

typedef std::string::size_type size_type;


bool is_substr(const std::string &t, const std::string &s, int pos) {
  // is t a susbtr of s from position pos in s
  // pos is correct position in s

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
  if (!is_substr(ending, data, (int)end_pos - (int)ending.size())) {
    return false;
  }

  int uri_len = end_pos - (int)(right_method.size() + ending.size() + 2);
  if (uri_len <= 0 ||
      data[right_method.size()] != ' ' ||
      data[end_pos - ending.size() - 1] != ' ') {
    return false;
  }

  std::string uri = data.substr(right_method.size() + 1, uri_len);
  if (!is_okay_uri(uri)) {
    return false;
  }

  first_line.method = right_method;
  first_line.uri = uri;

  return true;
}


bool is_ok_header_section(const std::string &data, int start_pos, int end_pos, HttpHeaders &headers) {
  std::map<std::string, std::string> result;
  size_type i = start_pos;
  while (i < end_pos) {
    size_type j = data.find("\r\n", i);
    if (j == std::string::npos) {
      j = data.size();
    }
    std::string cur_str = data.substr(i, j - i);
    size_type pos = cur_str.find(": ");
    if (pos == std::string::npos) {
      return false;
    }
    std::string left = cur_str.substr(0, pos);
    std::string right = cur_str.substr(pos + 2, cur_str.size() - (pos + 2));
    result[left] = right;
    i = j + 2;
  }
  headers.dict = std::move(result);
  return true;
}


bool is_http_end_of_line(const std::string &data, int pos) {
  if (pos == 0) {
    return false;
  }
  return data.substr(pos - 1, 2) == "\r\n";
}

bool is_http_end_of_headers(const std::string &data, int pos) {
  if (pos < 3) {
    return false;
  }
  return data.substr(pos - 3, 4) == "\r\n\r\n";
}


bool is_ok_path(const std::string &path) {
  // check if path is of template:
  // /{s_1}/.../{s_n}, s_i does not contain '/', is not '.' or '..', is not empty

  auto arr = split(path, '/');
  if (arr.empty()) {
    return false;
  }
  if (arr[0] != "") {
    return false;
  }

  for (int i = 1; i < arr.size(); ++i) {
    if (arr[i] == "" || arr[i] == "." || arr[i] == "..") {
      return false;
    }
  }

  return true;
}

bool file_exists(const std::string &path) {
  // assume is_ok_path(path)
  FILE *file = fopen(path.c_str(), "r");
  if (!file) {
    return false;
  }
  fclose(file);
  return true;
}

bool is_gettable_file(const std::string &path, Config &config) {
  // assume is_ok_path(path),
  // file_exists(path) == true
  struct stat statbuf;
  int status = lstat(path.c_str(), &statbuf);
  if (!(status == 0 && S_ISREG(statbuf.st_mode))) {
    return false;
  }
  int pos = config.data_dir.size() + 2; // after / after data_dir
  auto pos2 = path.find("/", pos); // that will be '/' after hostname dir
  return is_substr("static/", path, pos2 + 1); // have to go through static dir (dir_1 == static)
}

bool is_ok_qvalue_list(const std::string &str, std::vector<std::string> &mime_types) {
  auto vec = split(str, ',');
  for (auto &part : vec) {
    mime_types.push_back(part.substr(0, part.find(';')));
  }
  return true; // tolerancy, for now
}


int file_size(const std::string &path) {
  // assume is_ok_path(path), file_exists(path)
  struct stat statbuf;
  lstat(path.c_str(), &statbuf);
  return statbuf.st_size;
}

std::string file_contents(const std::string &path) {
  // assume is_ok_path(path), file_exists(path)
  FILE *file = fopen(path.c_str(), "r");
  int chunk_size = 256;
  char buf[chunk_size];
  std::string res;
  int cnt;
  while ((cnt = fread(buf, 1, chunk_size, file)) > 0) {
    for (int i = 0; i < cnt; ++i) {
      res.push_back(buf[i]);
    }
  }
  return res;
}