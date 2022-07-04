/**
 * @file   log_adapter.hpp
 * @author pengrl
 *
 */

#pragma once

#include "chef_log.hpp"
#include <iostream>

//namespace cls {
//
//class DummyLog {
//public:
//  template<typename T>
//  DummyLog &operator<< (const T &) {
//    return *this;
//  }
//}; // class DummyLog
//
//}; // namespace cls

//#ifdef  NDEBUG
//#define CLS_LOG(level)  std::cerr
//#define CLS_LOG_INFO    cls::DummyLog()

#define CLS_LOG(level)  CHEF_LOG(level)
#define CLS_LOG_TRACE   CHEF_LOG(trace)
#define CLS_LOG_DEBUG   CHEF_LOG(debug)

#define CLS_LOG_INFO    CHEF_LOG(info)
#define CLS_LOG_WARNING CHEF_LOG(warning)
#define CLS_LOG_ERROR   CHEF_LOG(error)
#define CLS_LOG_FATAL   CHEF_LOG(fatal)

