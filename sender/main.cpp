#include <iostream>
#include "rtc/pack.h"
#include "rtc/udp_server.h"
#include "rtc/udp_packet.hpp"
#include "gcc_server.h"
#include <pthread.h>
#include <string>
#include <vector>

void * test(void* args) {
  boost::asio::ip::address local_addr = boost::asio::ip::address::from_string("0.0.0.0");
  cls::UDPEndpoint local_endpoint(local_addr, 8001);

  boost::asio::io_service io_service;
  boost::asio::ip::udp::socket UDPSocket(io_service);
  UDPSocket.open(local_endpoint.protocol());


  boost::asio::ip::address send_addr = boost::asio::ip::address::from_string("172.16.27.59");
  cls::UDPEndpoint send_endpoint(send_addr, 8002);
  cls::UDPPacketPtr rtcp_packet;
  rtcp_packet = cls::Pack::packing_publish(11111, 11111);
  UDPSocket.send_to(boost::asio::buffer(rtcp_packet->const_buffer(), rtcp_packet->length()), send_endpoint);
  std::string flag = "start";
  uint16_t seq = 1;
  uint32_t time = 1234000;
  if (flag == "start") {
    cls::UDPPacketPtr rtcp_packet;
    rtcp_packet = cls::Pack::packing_publish(11111, 11111);
    UDPSocket.send_to(boost::asio::buffer(rtcp_packet->const_buffer(), rtcp_packet->length()), send_endpoint);
  } else {
    uint64_t tick = cls::GetCurrentStamp64();
    uint16_t pre = 0;
    for (; seq < 100000; seq++) {
      cls::UDPPacketPtr udp_packet;
      udp_packet = cls::Pack::packing_rtp_test(seq, time, 11111);
      UDPSocket.send_to(boost::asio::buffer(udp_packet->const_buffer(), udp_packet->length()), send_endpoint);
      time++;
      if (seq == 65536)
        seq = 0;

      usleep(600);
      uint64_t now = cls::GetCurrentStamp64();
      if (now - tick > 1000) {
        std::cout << seq - pre << std::endl;
        tick = now;
        pre = seq;
      }
    }
  }
}


int main() {
  pthread_t tids;
  int ret = pthread_create(&tids, NULL, test, NULL);
  if (ret != 0) {
    std::cout << "pthread error" << std::endl;
  }

  std::string ip("0.0.0.0");
  std::vector<std::string> vec_ip;
  vec_ip.push_back(ip);

  std::vector<uint16_t> vec_port;
  vec_port.push_back(8001);

  cls::GCCServer gcc_server(vec_ip, vec_port, 100);
  gcc_server.run();

  std::cout << "Hello, World!" << std::endl;

  return 0;
}