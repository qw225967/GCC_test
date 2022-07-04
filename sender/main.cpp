#include <iostream>
#include "rtc/pack.h"
#include "rtc/udp_server.h"
#include "rtc/udp_packet.hpp"
#include <pthread.h>

void test() {}


int main() {
  boost::asio::ip::address local_addr = boost::asio::ip::address::from_string("0.0.0.0");
  cls::UDPEndpoint local_endpoint(local_addr, 8001);

  boost::asio::io_service io_service;
  boost::asio::ip::udp::socket UDPSocket(io_service);
  UDPSocket.open(local_endpoint.protocol());


  boost::asio::ip::address send_addr = boost::asio::ip::address::from_string("192.168.25.123");
  cls::UDPEndpoint send_endpoint(send_addr, 10014);
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
  std::cout << "Hello, World!" << std::endl;

  return 0;
}