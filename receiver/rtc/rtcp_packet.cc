#include "rtcp_packet.h"
#include "../util/log_adapter.hpp"
#include "../util/helper.h"
#include "rtc_common.hpp"

namespace cls {

RTCPPacket::RTCPPacket() {}

bool RTCPPacket::parse(uint8_t *buf, std::size_t len) {
  if (len < RTCP_HEADER_SIZE_BYTES) {
    return false;
  }

  header_ = (reinterpret_cast<_RTCPHeader *>(buf));

  if (header_->version != RTCP_VERSION) {
    return false;
  }

  if (header_->get_length() > len) {
    // return false;
  }
  if (header_->padding == 1) {
    // TODO impl me
    //CLS_LOG_TRACE << "RTCPPacket parse failed since not support padding yet.";
    //return false;
  }

  payload_ = (reinterpret_cast<_RTCPPayload *>(buf+4));

  return true;
}

}; // namespace cls
