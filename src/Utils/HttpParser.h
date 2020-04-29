#ifndef CAOS_HTTP_WEB_SERVER_HTTPPARSER_H
#define CAOS_HTTP_WEB_SERVER_HTTPPARSER_H

#include <string>
#include <map>
#include "../Config.h"

struct HttpFirstLine {
    std::string method;
    std::string uri;
};

struct HttpHeaders {
    std::map<std::string, std::string> dict;
};

bool is_substr(const std::string &t, const std::string &s, int pos);

bool is_okay_uri(const std::string &uri);

bool is_ok_http_firstline(const std::string &data, int end_pos,
        std::vector<std::string> &http_methods, HttpFirstLine &first_line);

bool is_ok_header_section(const std::string &data, int start_pos, int end_pos, HttpHeaders &headers);

bool is_http_end_of_line(const std::string &data, int pos);

bool is_http_end_of_headers(const std::string &data, int pos);

bool is_ok_path(const std::string &path);

bool file_exists(const std::string &path);

bool is_gettable_file(const std::string &path, Config &config);

bool is_ok_qvalue_list(const std::string &str, std::vector<std::string> &mime_types);

int file_size(const std::string &path);

std::string file_contents(const std::string &path);

#endif //CAOS_HTTP_WEB_SERVER_HTTPPARSER_H
