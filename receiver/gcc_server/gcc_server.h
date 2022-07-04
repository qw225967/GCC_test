/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/6/29 9:06 下午
 * @mail        : qw225967@github.com
 * @project     : rtcp_test_send_single_n
 * @file        : gcc_server.h
 * @description : TODO
 *******************************************************/

#ifndef RTCP_TEST_SEND_SINGLE_N_GCC_SERVER_H
#define RTCP_TEST_SEND_SINGLE_N_GCC_SERVER_H

#include "api/transport/goog_cc_factory.h"
#include "api/transport/network_types.h"
#include "call/rtp_transport_controller_send.h"
#include "modules/pacing/packet_router.h"
#include "rtcp_packet.h"

#include "udp_server.h"

namespace cls {
typedef std::shared_ptr<RTCPPacket> RTCPPacketPtr;
typedef std::shared_ptr<webrtc::NetworkControllerFactoryInterface> NetworkControllerFactoryInterfacePtr;
typedef std::shared_ptr<webrtc::RtpTransportControllerSend> RtpTransportControllerSendPtr;

class GCCServer : public webrtc::PacketRouter,
                  public webrtc::TargetTransferRateObserver,
                  public UDPServerObserver {
public:
  GCCServer();
  virtual ~GCCServer();

public:
  // 接收到udp包的回调
  virtual void recv_udp_cb(UDPPacketPtr udp_packet, int addr_index) = 0;
  // 基础定时器回调
  virtual void crude_timer_cb(uint64_t tick_ms) = 0;

private:
  void rtcp_handler(UDPPacketPtr udp_packet);
  void rtp_handler(UDPPacketPtr udp_packet){}

private:
  NetworkControllerFactoryInterfacePtr controllerFactory_;
  RtpTransportControllerSendPtr rtpTransportControllerSend_;
};

} // namespace cls

#endif // RTCP_TEST_SEND_SINGLE_N_GCC_SERVER_H
