/**
 * @file   helper.h
 * @author pengrl
 *
 */

#pragma once

#include <map>
#include <string>
#include <stdint.h>
#include "chef_constructor_magic.hpp"

namespace cls {

struct HTTPResponse {
  std::string content_;
  std::string status_code;
};


class Helper {
  public:
    // 可用于debug时打印
    static std::string bytes_to_hex(const uint8_t *buf, std::size_t len, std::size_t num_per_line=8);

    static uint64_t now_ms();

    static std::string host_name();

    static bool get_or_post(const std::string &url,
                 const std::map<std::string, std::string> *headers,
                 const std::map<std::string, std::string> *cookies,
                 const char *post_data,
                 int timeout_ms,
                 int retry_times,
                 HTTPResponse &resp);

  static void report_falcon(std::string event, std::string end, int code);

  static std::string UrlEncode(const std::string& s_str);
  static std::string UrlDecode(const std::string& s_str);
  static std::map<std::string, std::string> ParseKeyValue(const char* input);


  CHEF_DISALLOW_IMPLICIT_CONSTRUCTORS(Helper);

}; // class Helper

}; // namespace cls
