#include "rtp_packet.h"
#include "../util/log_adapter.hpp"
#include "rtc_common.hpp"

namespace cls {

RTPPacket::RTPPacket() {}

bool RTPPacket::parse(uint8_t *buf, std::size_t len) {
  if (len < RTP_HEADER_SIZE_BYTES) {
    CLS_LOG_ERROR << "RTPPacket parse fail since truncated. len:" << len;
    return false;
  }

  header_ = (reinterpret_cast<_RTPHeader *>(buf));

  if (header_->version != RTP_VERSION) {
    CLS_LOG_ERROR << "RTPPacket parse fail since version invalid. version:" << (uint8_t)header_->version;
    return false;
  }

  return true;
}

}; // namespace cls
