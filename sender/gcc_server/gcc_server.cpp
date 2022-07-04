/*******************************************************
 * @author      : dog head
 * @date        : Created in 2022/6/30 3:56 下午
 * @mail        : qw225967@github.com
 * @project     : sender
 * @file        : gcc_server.cpp
 * @description : TODO
 *******************************************************/

#include "gcc_server.h"
#include "FeedbackRtpTransport.hpp"
#include "rtc_common.hpp"
#include "udp_packet.hpp"

namespace cls {

const uint32_t InitialAvailableBitrate{600000u};

GCCServer::GCCServer(const std::vector<std::string> &ips,
                     std::vector<uint16_t> ports,
                     uint64_t crude_timer_interval_ms) {
  udp_server_ = std::make_shared<UDPServer>(ips, ports, this, crude_timer_interval_ms);
  // 初始化网络控制工程
  webrtc::GoogCcFactoryConfig config;
  config.feedback_only = true;
  controllerFactory_ = std::make_shared<webrtc::GoogCcNetworkControllerFactory>(
      std::move(config));

  // 初始化传输控制
  webrtc::BitrateConstraints bitrateConfig;
  bitrateConfig.start_bitrate_bps = static_cast<int>(InitialAvailableBitrate);
  rtpTransportControllerSend_ =
      std::make_shared<webrtc::RtpTransportControllerSend>(
          this, nullptr, controllerFactory_.get(), bitrateConfig);
}

GCCServer::~GCCServer() {
  controllerFactory_.reset();
  rtpTransportControllerSend_.reset();
  udp_server_.reset();
}

// 接收到udp包的回调
void GCCServer::recv_udp_cb(UDPPacketPtr udp_packet, int addr_index) {
  if (!udp_packet.get()) {
    return;
  }

  // 取第一个字符为 payload type （自定义）
  const uint8_t *buf = udp_packet->const_buffer();
  uint8_t pt = buf[1];

  switch (pt) {
  case PT_GCCFB:
    rtcp_handler(udp_packet);
    break;
  default:
    break;
  }
}
// 基础定时器回调
void GCCServer::crude_timer_cb(uint64_t tick_ms) {}

void GCCServer::rtcp_handler(UDPPacketPtr udp_packet) {
  if (!udp_packet.get()) {
    return;
  }

  RTCPPacketPtr rtcp_packet = std::make_shared<RTCPPacket>();
  if (!rtcp_packet->parse(udp_packet->mutable_buffer(), udp_packet->length())) {
    return;
  }
  auto pt = rtcp_packet->header()->packet_type;
  switch (pt) {
  case PT_GCCFB: {
    RTC::RTCP::FeedbackRtpTransportPacket  feedback(123123, 123123);
    if (!feedback.Parse(udp_packet->mutable_buffer(), udp_packet->length())) {
      return;
    }
    rtpTransportControllerSend_->OnTransportFeedback(feedback);
    break;
  }
  default:
    break;
  }
}

} // namespace cls