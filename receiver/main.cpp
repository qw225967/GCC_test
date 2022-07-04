#include <iostream>
#include "rtc/pack.h"
#include "rtc/udp_server.h"
#include "rtc/udp_packet.hpp"
#include <pthread.h>

void test() {}


int main() {
  boost::asio::ip::address local_addr = boost::asio::ip::address::from_string("0.0.0.0");
  cls::UDPEndpoint local_endpoint(local_addr, 8002);

  boost::asio::io_service io_service;
  boost::asio::ip::udp::socket UDPSocket(io_service);
  UDPSocket.open(local_endpoint.protocol());

  std::cout << "Hello, World!" << std::endl;

  return 0;
}