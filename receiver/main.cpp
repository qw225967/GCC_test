#include <iostream>
#include "rtc/pack.h"
#include "rtc/udp_server.h"
#include "gcc_server.h"
#include <pthread.h>
#include <string>
#include <vector>

void test() {}


int main() {
  boost::asio::ip::address local_addr = boost::asio::ip::address::from_string("0.0.0.0");
  cls::UDPEndpoint local_endpoint(local_addr, 8002);

  boost::asio::io_service io_service;
  boost::asio::ip::udp::socket UDPSocket(io_service);
  UDPSocket.open(local_endpoint.protocol());

  std::string ip("0.0.0.0");
  std::vector<std::string> vec_ip;
  vec_ip.push_back(ip);

  std::vector<uint16_t> vec_port;
  vec_port.push_back(8002);

  cls::GCCServer gcc_server(vec_ip, vec_port, 100);
  gcc_server.run();




  std::cout << "Hello, World!" << std::endl;

  return 0;
}