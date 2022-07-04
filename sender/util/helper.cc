#include "helper.h"
#include <sstream>
#include <iomanip>
#include <chrono>
#include <stdio.h>
#include <unistd.h>
#include "chef_strings_op.hpp"
#include "log_adapter.hpp"
#include "falcon_cc.hpp"


namespace cls {

std::string Helper::bytes_to_hex(const uint8_t *buf, std::size_t len, std::size_t num_per_line) {
  if (buf == NULL || len == 0 || num_per_line == 0) {
    return std::string();
  }
  std::ostringstream oss;
  for (std::size_t i = 0; i < len; i++) {
    oss << std::right << std::setw(3) << std::hex << static_cast<int>(buf[i]);
    if ((i+1) % num_per_line == 0) {
      oss << '\n';
    }
  }
  if (len % num_per_line != 0) {
    oss << '\n';
  }
  return oss.str();
}

uint64_t Helper::now_ms() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string Helper::host_name() {
  char name[128] = {0};
  int ret = gethostname(name, sizeof(name));
  return (ret == 0) ? std::string(name) : std::string();
}


bool Helper::get_or_post(const std::string &url,
                 const std::map<std::string, std::string> *headers,
                 const std::map<std::string, std::string> *cookies,
                 const char *post_data,
                 int timeout_ms,
                 int retry_times,
                 HTTPResponse &resp)
{
//  while(retry_times--){
//
//    try {
//      std::string url_without_scheme = chef::strings_op::trim_prefix(url, "http://");
//
//      std::string host_port;
//      std::string path_with_query;
//      auto pos = url_without_scheme.find("/");
//      if (pos == std::string::npos) {
//        host_port = url_without_scheme;
//        path_with_query = "/";
//      } else {
//        host_port = url_without_scheme.substr(0, pos);
//        path_with_query = url_without_scheme.substr(pos);
//      }
//      //CLS_LOG_TRACE << "host_port:" << host_port << ", path_with_query:" << path_with_query;
//
//      SimpleWeb::Client<SimpleWeb::HTTP> client(host_port);
//      client.config.timeout = timeout_ms / 1000;
//      std::shared_ptr<SimpleWeb::Client<SimpleWeb::HTTP>::Response> sw_resp;
//      if (post_data) {
//        sw_resp = client.request("POST", path_with_query, post_data);
//      } else {
//        sw_resp = client.request("GET", path_with_query);
//      }
//      resp.content_ = sw_resp->content.string();
//      resp.status_code = sw_resp->status_code;
//
//      return true;
//    } catch(const SimpleWeb::system_error &e) {
//      CLS_LOG_ERROR << "Report:get_or_post error, url:" << url << ", data:" << post_data << ", err:" << e.what();
//    }
//  }
  return false;
}

void Helper::report_falcon(std::string event, std::string end, int code) {
//    StatEntry se(event);
//    se.End(end, code);
}



#define XDIGIT_TO_NUM(h) ((h) < 'A' ? (h) - '0' : toupper (h) - 'A' + 10)
#define X2DIGITS_TO_NUM(h1, h2) ((XDIGIT_TO_NUM (h1) << 4) + XDIGIT_TO_NUM (h2))

static void Trim(std::string& str) {
    std::string t = str;
    t.erase(0, t.find_first_not_of(" \t\n\r"));
    t.erase(t.find_last_not_of(" \t\n\r") + 1);
    str = t;
}


static char* unescape(char* s) {
    char* t = s;                  /* t - tortoise */
    char* h = s;                  /* h - hare     */

    for (; *h; h++, t++) {
        if (*h != '%') {
        copychar:
            *t = (*h == '+') ? ' ' : *h;
        } else {
            char c;

            /* Do nothing if '%' is not followed by two hex digits. */
            if (!h[1] || !h[2] || !(isxdigit(h[1]) && isxdigit(h[2])))
                goto copychar;

            c = X2DIGITS_TO_NUM(h[1], h[2]);

            /* Don't unescape %00 because there is no way to insert it
             *              into a C string without effectively truncating it. */
            if (c == '\0')
                goto copychar;

            *t = c;
            h += 2;
        }
    }

    *t = '\0';

    return s;
}

std::string Helper::UrlEncode(const std::string& source_str) {
    int size = source_str.size();

    if (size == 0) {
        return std::string();
    }

    const char* ptr = source_str.c_str();

    //boost::shared_array<char> buf(new char[size * 3 + 1]);
    char* buf = new char[size * 3 + 1];
    const char* const pstart = &buf[0];
    char* wp = &buf[0];

    for (int i = 0; i < size; i++) {
        int c = ((unsigned char*)ptr)[i];

        if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
                (c >= '0' && c <= '9') || (c != '\0' && strchr("_-.!~*'()", c))) {
            *(wp++) = c;
        } else {
            wp += sprintf(wp, "%%%02X", c);
        }
    }

    *wp = '\0';
    std::string output_str(pstart);
    //dest_str = pstart;
    delete [] buf;
    return output_str;
}

std::string Helper::UrlDecode(const std::string& url) {
    char* buf = new char[url.size() + 1];
    strcpy(buf, url.c_str());
    unescape(buf);
    //strResult = buf;
    std::string output_str(buf);
    delete[] buf;
    return output_str;
}

std::map<std::string, std::string> Helper::ParseKeyValue(const char* input) {
    std::map<std::string, std::string> args_map;

    if (!input) {
        return args_map;
    }

    char buff[4096];
    strncpy(buff, input, 4096);
    char* inputs = buff;
    char* tmp_str = NULL;
    char* ptr = NULL;

    while ((tmp_str = strtok_r(inputs, "&", &ptr)) != NULL) {
        char* pdimiter = strchr(tmp_str, '=');

        if (pdimiter != NULL) {
            *pdimiter = '\0';
            std::string key = tmp_str;
            std::string value = pdimiter + 1;
            //value需要去除换行，空格
            Trim(value);
            args_map[key] = value;
        }

        inputs = NULL;
    }

    return args_map;
}



}; // namespace cls
