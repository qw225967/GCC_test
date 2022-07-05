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
#include "rtp_packet.h"
#include "rtcp_packet.h"
#include "helper.h"

namespace cls {

const uint32_t InitialAvailableBitrate{600000u};
const size_t MtuSize{1400u};

GCCServer::GCCServer(const std::vector<std::string> &ips,
                     std::vector<uint16_t> ports,
                     uint64_t crude_timer_interval_ms) {
  transportCcFeedbackPacket_ =
      std::make_shared<RTC::RTCP::FeedbackRtpTransportPacket>(0u, 0u);

  udp_server_ =
      std::make_shared<UDPServer>(ips, ports, this, crude_timer_interval_ms);
}

GCCServer::~GCCServer() {
  transportCcFeedbackPacket_.reset();
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
  case PT_H264:
    rtp_handler(udp_packet);
    break;
  default:
    break;
  }
}
// 基础定时器回调
void GCCServer::crude_timer_cb(uint64_t tick_ms) {}

void GCCServer::rtcp_handler(UDPPacketPtr udp_packet) {}

void GCCServer::rtp_handler(UDPPacketPtr udp_packet) {
  if (!udp_packet.get()) {
    return;
  }

  RTPPacketPtr rtp_packet = std::make_shared<RTPPacket>();
  if (!rtp_packet->parse(udp_packet->mutable_buffer(), udp_packet->length())) {
    return;
  }

  transportCcFeedbackSenderSsrc_ = 0u;
  transportCcFeedbackMediaSsrc_ = rtp_packet->header()->get_ssrc();

  transportCcFeedbackPacket_->SetSenderSsrc(0u);
  transportCcFeedbackPacket_->SetMediaSsrc((rtp_packet->header()->ssrc));

  auto wideSeqNumber = rtp_packet->header()->get_sequence_number();
  auto nowMs = GetCurrentStamp64();

  auto result = transportCcFeedbackPacket_->AddPacket(wideSeqNumber, nowMs,
                                                      MtuSize);

  switch (result) {
  case RTC::RTCP::FeedbackRtpTransportPacket::AddPacketResult::SUCCESS: {
    // If the feedback packet is full, send it now.
    if (transportCcFeedbackPacket_->IsFull()) {
      SendTransportCcFeedback(udp_packet->mutable_endpoint());
    }

    break;
  }

  case RTC::RTCP::FeedbackRtpTransportPacket::AddPacketResult::
      MAX_SIZE_EXCEEDED: {
    // Send ongoing feedback packet and add the new packet info to the
    // regenerated one.
    SendTransportCcFeedback(udp_packet->mutable_endpoint());

    transportCcFeedbackPacket_->AddPacket(wideSeqNumber, nowMs, MtuSize);

    break;
  }

  case RTC::RTCP::FeedbackRtpTransportPacket::AddPacketResult::FATAL: {
    // Create a new feedback packet.
    transportCcFeedbackPacket_.reset(new RTC::RTCP::FeedbackRtpTransportPacket(
        this->transportCcFeedbackSenderSsrc_,
        this->transportCcFeedbackMediaSsrc_));

    // Use current packet count.
    // NOTE: Do not increment it since the previous ongoing feedback
    // packet was not sent.
    transportCcFeedbackPacket_->SetFeedbackPacketCount(
        transportCcFeedbackPacketCount_);

    break;
  }
  }
}
void GCCServer::SendTransportCcFeedback(UDPEndpoint &ep) {

  if (!transportCcFeedbackPacket_->IsSerializable())
    return;

  auto latestWideSeqNumber =
      transportCcFeedbackPacket_->GetLatestSequenceNumber();
  auto latestTimestamp = transportCcFeedbackPacket_->GetLatestTimestamp();

//  std::cout << "get:" << transportCcFeedbackPacket_.get() << ", result len:"
//            << transportCcFeedbackPacket_->GetPacketResults().size()
//            << std::endl;
  UDPPacketPtr pkt = std::make_shared<UDPPacket>(1);
  size_t len = 0;
  if(transportCcFeedbackPacket_->BuildClsRTCPPacket(pkt->mutable_buffer(), len)) {
    // test
    boost::asio::ip::address send_addr = boost::asio::ip::address::from_string("172.16.27.59");
    cls::UDPEndpoint send_endpoint(send_addr, 8001);
    pkt->mod_length(len);
//    std::cout << cls::Helper::bytes_to_hex(pkt->mutable_buffer(), len) << std::endl;
    udp_server_->async_send_to(0 ,pkt, send_endpoint);
  }


  // Create a new feedback packet.
  transportCcFeedbackPacket_.reset(new RTC::RTCP::FeedbackRtpTransportPacket(
      transportCcFeedbackSenderSsrc_, transportCcFeedbackMediaSsrc_));

  // Increment packet count.
  transportCcFeedbackPacket_->SetFeedbackPacketCount(
      ++transportCcFeedbackPacketCount_);

  // Pass the latest packet info (if any) as pre base for the new feedback
  // packet.
  if (latestTimestamp > 0u) {
    transportCcFeedbackPacket_->AddPacket(latestWideSeqNumber, latestTimestamp,
                                          MtuSize);
  }

  //  UDPPacketPtr packet = std::make_shared<UDPPacket>(1);
  //
  //
  //
  //
  //
  //  udp_server_->async_send_to(0,,);
}

} // namespace cls