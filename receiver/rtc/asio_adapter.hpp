/**
 * @file   asio_adapter.hpp
 * @author pengrl
 *
 */

#pragma once

#include <string>
#include "/opt/homebrew/Cellar/boost/1.76.0/include/boost/asio.hpp"
#include <sstream>
#include "../util/chef_constructor_magic.hpp"

namespace cls {

typedef boost::asio::ip::udp::endpoint UDPEndpoint;
typedef boost::asio::ip::udp::socket   UDPSocket;
typedef boost::asio::ip::address_v4    Address;
typedef boost::asio::io_service        IOService;
typedef boost::asio::deadline_timer    DeadlineTimer;
typedef boost::system::error_code      ErrorCode;
namespace PosixTime = boost::posix_time;
typedef std::shared_ptr<DeadlineTimer> DeadlineTimerPtr;

class AsioAdapter {
  public:
    static std::string endpoint_to_string(const UDPEndpoint &ep) {
      std::ostringstream ss;
      ss << ep.address().to_string() << ":" << ep.port();
      return ss.str();
    }

  CHEF_DISALLOW_IMPLICIT_CONSTRUCTORS(AsioAdapter);

}; // class AsioAdapter

}; // namespace cls
