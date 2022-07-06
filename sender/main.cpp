#include "gcc_server.h"
#include "rtc/pack.h"
#include "rtc/udp_packet.hpp"
#include "rtc/udp_server.h"
#include <iostream>
#include <map>
#include <pthread.h>
#include <string>
#include <vector>

std::string ip("0.0.0.0");
std::vector<std::string> vec_ip;

std::vector<uint16_t> vec_port;

cls::GCCServer *gcc_server = nullptr;

void *test(void *args) {
  boost::asio::ip::address local_addr =
      boost::asio::ip::address::from_string("0.0.0.0");
  cls::UDPEndpoint local_endpoint(local_addr, 8003);

  boost::asio::io_service io_service;
  boost::asio::ip::udp::socket UDPSocket(io_service);
  UDPSocket.open(local_endpoint.protocol());

  boost::asio::ip::address send_addr =
      boost::asio::ip::address::from_string("172.16.10.127");
  cls::UDPEndpoint send_endpoint(send_addr, 8002);

  std::string flag = "start";
  uint16_t seq = 1;
  uint64_t pre_tick = 0;
  uint64_t send_tick = cls::GetCurrentStamp64();
  uint32_t pacing_bitrates = 0;
  uint32_t count = 0;
  std::map<uint16_t, cls::UDPPacketPtr> send_packets;

  if (flag == "start") {
    uint16_t pre = 0;
    for (; seq < 100000; seq++) {
      uint64_t tick = cls::GetCurrentStamp64();
      if (pre_tick == 0) {
        pre_tick = cls::GetCurrentStamp64();
      }
      if (pacing_bitrates == 0 || tick - pre_tick > 1000) {
        if (gcc_server) {
          pacing_bitrates = gcc_server->GetavailableBitrate() / 5;
        }
      }
      if (tick - pre_tick > 1000) {
        pre_tick = tick;
        std::cout << "send packet count:" << count << std::endl;
        count = 0;
      }

      cls::UDPPacketPtr udp_packet;
      udp_packet = cls::Pack::packing_rtp_test(seq, tick, 123123);

      webrtc::RtpPacketSendInfo packetInfo;
      webrtc::PacedPacketInfo pacingInfo(1, 0, 600000);
      pacingInfo.send_bitrate_bps = udp_packet->length();
      packetInfo.ssrc = 123123;
      packetInfo.transport_sequence_number = seq;
      packetInfo.has_rtp_sequence_number = true;
      packetInfo.rtp_sequence_number = seq;
      packetInfo.length = udp_packet->length();
      packetInfo.pacing_info = pacingInfo;
      if (gcc_server) {
        gcc_server->OnPacketSend(packetInfo, tick);
        send_packets[seq] = udp_packet;
//        UDPSocket.send_to(boost::asio::buffer(udp_packet->const_buffer(),
//                                              udp_packet->length()),
//                          send_endpoint);
//        pacing_bitrates -= udp_packet->length();
//        count++;
      }



      if (tick - send_tick > 5) {
        send_tick = tick;
        for (auto iter = send_packets.begin();
             iter != send_packets.end() &&
             pacing_bitrates > iter->second->length();) {
          UDPSocket.send_to(boost::asio::buffer(iter->second->const_buffer(),
                                                iter->second->length()),
                            send_endpoint);
          pacing_bitrates -= iter->second->length();
          count++;
          iter = send_packets.erase(iter);
        }
      }

      if (seq == 65536)
        seq = 0;

      usleep(1000);
      //      uint64_t now = cls::GetCurrentStamp64();

      //      if (now - tick > 1000) {
      //        std::cout << seq - pre << std::endl;
      //        tick = now;
      //        pre = seq;
      //      }
    }
  }
}

int main() {
  pthread_t tids;
  int ret = pthread_create(&tids, NULL, test, NULL);
  if (ret != 0) {
    std::cout << "pthread error" << std::endl;
  }

  vec_ip.push_back(ip);
  vec_port.push_back(8001);

  gcc_server = new cls::GCCServer(vec_ip, vec_port, 100);
  gcc_server->run();

  delete gcc_server;
  std::cout << "Hello, World!" << std::endl;

  return 0;
}