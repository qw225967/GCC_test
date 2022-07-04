#include "pack.h"
#include "../util/log_adapter.hpp"
#include "../util/helper.h"
#include "../util/chef_strings_op.hpp"
#include "udp_packet.hpp"
#include "rtcp_packet.h"
#include "rtp_packet.h"
#include "rtc_common.hpp"

// -----NACK-----
// RFC 4585: Feedback format.
//
// Common packet format:
//
//    0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |V=2|P|   FMT   |       PT      |          length               |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 0 |                  SSRC of packet sender                        |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 4 |                  SSRC of media source                         |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   :            Feedback Control Information (FCI)                 :
//   :                                                               :
//
// Generic NACK (RFC 4585).
//
// FCI:
//    0                   1                   2                   3
//    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//   |            PID                |             BLP               |
//   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


// -----SR-----
//    Sender report (SR) (RFC 3550).
//     0                   1                   2                   3
//     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |V=2|P|    RC   |   PT=SR=200   |             length            |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  0 |                         SSRC of sender                        |
//    +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//  4 |              NTP timestamp, most significant word             |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  8 |             NTP timestamp, least significant word             |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 12 |                         RTP timestamp                         |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 16 |                     sender's packet count                     |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 20 |                      sender's octet count                     |
// 24 +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+


// -----RR-----
// RTCP receiver report (RFC 3550).
//
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |V=2|P|    RC   |   PT=RR=201   |             length            |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  |                     SSRC of packet sender                     |
//  +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//  |                         report block(s)                       |
//  |                            ....                               |


// -----REMB-----
// Receiver Estimated Max Bitrate (REMB) (draft-alvestrand-rmcat-remb).
//
//     0                   1                   2                   3
//     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    |V=2|P| FMT=15  |   PT=206      |             length            |
//    +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
//  0 |                  SSRC of packet sender                        |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  4 |                       Unused = 0                              |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  8 |  Unique identifier 'R' 'E' 'M' 'B'                            |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 12 |  Num SSRC     | BR Exp    |  BR Mantissa                      |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
// 16 |   SSRC feedback                                               |
//    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//    :  ...                                                          :


namespace cls {

static constexpr std::size_t NACK_COMMON_FEEDBACK_BYTES = 8;
static constexpr std::size_t NACK_ITEM_BYTES = 4;
static constexpr std::size_t MAX_NACK_ITEM_NUM =
    (UDPPacket::UDP_PACKET_MAX_BYTES - RTCP_HEADER_SIZE_BYTES - NACK_COMMON_FEEDBACK_BYTES) / NACK_ITEM_BYTES;

UDPPacketPtr Pack::packing_remb(uint32_t sender_ssrc,
                                const std::vector<uint32_t> &media_ssrcs,
                                uint64_t bitrate_bps)
{
  static constexpr std::size_t HEADER_LENGTH = 4;
  static constexpr std::size_t COMMON_FEEDBACK_LENGTH = 8;
  static constexpr std::size_t REMB_FEEDBACK_MESSAGE_TYPE = 15;
  static constexpr uint32_t UNIQUE_IDENTIFIER = 0x52454D42;  // 'R' 'E' 'M' 'B'
  static constexpr uint32_t MAX_MANTISSA = 0x3ffff; // 18 bits.

  int length_in_bytes = HEADER_LENGTH + COMMON_FEEDBACK_LENGTH + (2 + media_ssrcs.size()) * 4;
  int length_in_words = length_in_bytes / 4;
  if (media_ssrcs.empty() || length_in_bytes > UDPPacket::UDP_PACKET_MAX_BYTES) {
  }

  UDPPacketPtr udp_packet = std::make_shared<UDPPacket>(7);
  _RTCPHeader *header = reinterpret_cast<_RTCPHeader *>(udp_packet->mutable_buffer());
  header->version         = RTCP_VERSION;
  header->padding         = 0;
  header->count_or_format = REMB_FEEDBACK_MESSAGE_TYPE;
  header->packet_type     = PT_REMB;
  header->length          = htons(length_in_words - 1);

  _RTCPPayload *payload = reinterpret_cast<_RTCPPayload *>(udp_packet->mutable_buffer() + RTCP_HEADER_SIZE_BYTES);
  payload->remb.sender_ssrc = htonl(sender_ssrc);
  payload->remb.media_ssrc = htonl(media_ssrcs[0]);
  payload->remb.unique_identifier = htonl(UNIQUE_IDENTIFIER);
  payload->remb.ssrcs_size = static_cast<uint8_t>(media_ssrcs.size());

  uint64_t mantissa = bitrate_bps;
  uint8_t exponenta = 0;
  while (mantissa > MAX_MANTISSA) {
    mantissa >>= 1;
    ++exponenta;
  }
  payload->remb.br_dummy = (exponenta << 2) | (mantissa >> 16);
  payload->remb.br_dummy2 = htons(mantissa & 0xffff);
  for (std::size_t i = 0; i < media_ssrcs.size(); i++) {
    payload->remb.ssrcs[i] = htonl(media_ssrcs[i]);
  }

  return udp_packet;
}

UDPPacketPtr Pack::packing_remb(uint32_t sender_ssrc,
                                uint32_t  &media_ssrc,
                                uint64_t bitrate_bps)
{
  return packing_remb(sender_ssrc, std::vector<uint32_t>(1, media_ssrc), bitrate_bps);
}

UDPPacketPtr Pack::packing_sr(uint32_t ssrc,
                           uint32_t msw,
                           uint32_t lsw,
                           uint32_t rtp_timestamp,
                           uint32_t packet_count,
                           uint32_t octet_count)
{
  static constexpr uint16_t LEN_IN_WORDS = 7;

  UDPPacketPtr udp_packet = std::make_shared<UDPPacket>(8);
  _RTCPHeader *header = reinterpret_cast<_RTCPHeader *>(udp_packet->mutable_buffer());
  header->version         = RTCP_VERSION;
  header->padding         = 0;
  header->count_or_format = 1;
  header->packet_type     = PT_SR;
  header->length          = htons(LEN_IN_WORDS - 1);

  _RTCPPayload *payload = reinterpret_cast<_RTCPPayload *>(udp_packet->mutable_buffer() + RTCP_HEADER_SIZE_BYTES);
  payload->sr.ssrc = htonl(ssrc);
  payload->sr.msw = htonl(msw);
  payload->sr.lsw = htonl(lsw);
  payload->sr.rtp_timestamp = htonl(rtp_timestamp);
  payload->sr.packet_count  = htonl(packet_count);
  payload->sr.octet_count   = htonl(octet_count);

  //CLS_LOG_TRACE << "Packing sr. ssrc:" << ssrc
  //              << ", msw:" << msw
  //              << ", lsw:" << lsw;

  udp_packet->mod_length(LEN_IN_WORDS * 4);

  return udp_packet;
}

UDPPacketPtr Pack::packing_hack_sr(uint32_t ssrc,
                                   uint32_t msw,
                                   uint32_t lsw,
                                   uint32_t rtp_timestamp,
                                   uint32_t packet_count,
                                   uint32_t octet_count,
                                   uint32_t rtt_ms)
{
  UDPPacketPtr udp_packet = Pack::packing_sr(ssrc, msw, lsw, rtp_timestamp, packet_count, octet_count);

  static constexpr uint16_t LEN_IN_WORDS = 8;

  _RTCPHeader *header = reinterpret_cast<_RTCPHeader *>(udp_packet->mutable_buffer());
  header->padding = 1;
  header->length  = htons(LEN_IN_WORDS - 1);

  _RTCPPayload *payload = reinterpret_cast<_RTCPPayload *>(udp_packet->mutable_buffer() + RTCP_HEADER_SIZE_BYTES);
  payload->sr.rtt = htonl(rtt_ms);

  //CLS_LOG_INFO << "sr:" << Helper::bytes_to_hex(udp_packet->mutable_buffer(), 32);

  udp_packet->mod_length(LEN_IN_WORDS * 4);

  return udp_packet;
}

UDPPacketPtr Pack::packing_rr(uint32_t sender_ssrc,
                              uint32_t media_ssrc,
                              uint8_t fractionlost,
                              uint32_t lost,
                              uint16_t cycles,
                              uint16_t highestseqnum,
                              uint32_t jitter,
                              uint32_t lastsr,
                              uint32_t delay_since_lastsr_ms)
{
  static constexpr uint16_t LEN_IN_WORDS = 8;

  UDPPacketPtr udp_packet = std::make_shared<UDPPacket>(9);

  _RTCPHeader *header = reinterpret_cast<_RTCPHeader *>(udp_packet->mutable_buffer());
  header->version         = RTCP_VERSION;
  header->padding         = 0;
  header->count_or_format = 1;
  header->packet_type     = PT_RR;
  header->length          = htons(LEN_IN_WORDS - 1);

  _RTCPPayload *payload = reinterpret_cast<_RTCPPayload *>(udp_packet->mutable_buffer() + RTCP_HEADER_SIZE_BYTES);
  payload->rr.sender_ssrc        = htonl(sender_ssrc);
  payload->rr.media_ssrc         = htonl(media_ssrc);
  payload->rr.fractionlost       = fractionlost;
  payload->rr.lost               = htonl(lost) >> 8;
  payload->rr.cycles             = htons(cycles);
  payload->rr.highestseqnum      = htons(highestseqnum);
  payload->rr.jitter             = htonl(jitter);
  payload->rr.lastsr             = htonl(lastsr);
  payload->rr.delay_since_lastsr = htonl(delay_since_lastsr_ms * 65536 / 1000);

  udp_packet->mod_length(LEN_IN_WORDS * 4);

  return udp_packet;
}

UDPPacketPtr Pack::packing_forward(uint32_t ssrc, uint32_t rtt_ms) {
  static constexpr uint16_t LEN_IN_WORDS = 3;

  UDPPacketPtr udp_packet = std::make_shared<UDPPacket>(10);

  _RTCPHeader *header = reinterpret_cast<_RTCPHeader *>(udp_packet->mutable_buffer());
  header->version         = RTCP_VERSION;
  header->padding         = 0;
  header->count_or_format = 1;
  header->packet_type     = PT_FORWARD;
  header->length          = htons(LEN_IN_WORDS - 1);

  _RTCPPayload *payload = reinterpret_cast<_RTCPPayload *>(udp_packet->mutable_buffer() + RTCP_HEADER_SIZE_BYTES);
  payload->forward.ssrc = htonl(ssrc);
  payload->forward.rtt = htonl(rtt_ms);

  udp_packet->mod_length(LEN_IN_WORDS * 4);

  return udp_packet;
}

UDPPacketPtr Pack::packing_fir(uint32_t ssrc) {
  static constexpr uint16_t LEN_IN_WORDS = 2;

  UDPPacketPtr udp_packet = std::make_shared<UDPPacket>(11);

  _RTCPHeader *header = reinterpret_cast<_RTCPHeader *>(udp_packet->mutable_buffer());
  header->version         = RTCP_VERSION;
  header->padding         = 0;
  header->count_or_format = 1;
  header->packet_type     = PT_FIR;
  header->length          = htons(LEN_IN_WORDS - 1);

  _RTCPPayload *payload = reinterpret_cast<_RTCPPayload *>(udp_packet->mutable_buffer() + RTCP_HEADER_SIZE_BYTES);
  payload->fir.ssrc = htonl(ssrc);

  udp_packet->mod_length(LEN_IN_WORDS * 4);

  return udp_packet;
}

bool Pack::unpacking_nack(RTCPPacketPtr rtcp_packet, std::vector<uint16_t> &sequence_vector) {
  // assumes header is already parsed and validated.
  assert(rtcp_packet->header()->packet_type == PT_NACK);

  uint16_t len = rtcp_packet->header()->get_length();

  // at least one nack item.
  if (len < RTCP_HEADER_SIZE_BYTES + NACK_COMMON_FEEDBACK_BYTES) {
    return false;
  }
  std::size_t nack_item = (len - (RTCP_HEADER_SIZE_BYTES + NACK_COMMON_FEEDBACK_BYTES)) / NACK_ITEM_BYTES;

  for (std::size_t i = 0; i < nack_item; i++) {
    const _FCI *fci = rtcp_packet->payload()->nack.fci + i;
    uint16_t pid = fci->get_pid();
    uint16_t blp = fci->get_blp();

    sequence_vector.push_back(pid++);
    for (uint16_t bitmask = blp; bitmask != 0; bitmask >>= 1, ++pid) {
      if (bitmask & 1) {
        sequence_vector.push_back(pid);
      }
    }
  }

//  CLS_LOG_TRACE << "nack_item:" << nack_item << ", sequence vector size:" << sequence_vector.size();

  return true;
}

UDPPacketPtr Pack::packing_nack(uint32_t sender_ssrc, uint32_t media_ssrc, const std::vector<uint16_t> &sequence_vector) {
  UDPPacketPtr udp_packet = std::make_shared<UDPPacket>(12);

  _RTCPHeader *header = reinterpret_cast<_RTCPHeader *>(udp_packet->mutable_buffer());
  header->version         = RTCP_VERSION;
  header->padding         = 0;
  header->count_or_format = 1;
  header->packet_type     = PT_NACK;

  _RTCPPayload *payload = reinterpret_cast<_RTCPPayload *>(udp_packet->mutable_buffer() + RTCP_HEADER_SIZE_BYTES);
  payload->nack.sender_ssrc = htonl(sender_ssrc);
  payload->nack.media_ssrc  = htonl(media_ssrc);

  std::size_t nack_item_count = 0;
  auto iter = sequence_vector.begin();
  auto end = sequence_vector.end();
  while (iter != end) {
    _FCI &fci = payload->nack.fci[nack_item_count];
    fci.pid = *iter++;
    fci.blp = 0;
    while (iter != end) {
      uint16_t shift = static_cast<uint16_t>(*iter - fci.pid - 1);
      if (shift <= 15) {
        fci.blp |= (1 << shift);
        ++iter;
      } else {
        break;
      }
    }
    fci.pid = htons(fci.pid);
    fci.blp = htons(fci.blp);
    nack_item_count++;
    if (nack_item_count == MAX_NACK_ITEM_NUM) {
      break;
    }
  }

  uint16_t length = static_cast<uint16_t>(RTCP_HEADER_SIZE_BYTES + NACK_COMMON_FEEDBACK_BYTES + NACK_ITEM_BYTES * nack_item_count);

  header->length = htons((length / 4) - 1);
  udp_packet->mod_length(length);

  return udp_packet;
}


static void _packing_app_header(UDPPacketPtr udp_packet, uint16_t len_in_word, uint8_t subtype) {
  _RTCPHeader *header = reinterpret_cast<_RTCPHeader *>(udp_packet->mutable_buffer());
  header->version         = RTCP_VERSION;
  header->padding         = 0;
  header->count_or_format = subtype;
  header->packet_type     = PT_APP;
  header->length          = htons(len_in_word - 1);
}

// 打包自定义app包的帮助函数
static UDPPacketPtr _packing_app_helper(uint16_t len_in_word, uint8_t subtype, uint32_t ssrc, _RTCPPayload *&payload) {
  UDPPacketPtr udp_packet = std::make_shared<UDPPacket>(13);

  _packing_app_header(udp_packet, len_in_word, subtype);

  payload = reinterpret_cast<_RTCPPayload *>(udp_packet->mutable_buffer() + RTCP_HEADER_SIZE_BYTES);
  payload->app.ssrc = htonl(ssrc);
  memset(payload->app.name, 0x0, sizeof(uint32_t));

  udp_packet->mod_length(len_in_word * 4);

  return udp_packet;
}

UDPPacketPtr Pack::packing_subpublish_ack(uint32_t ssrc, const std::string &ip) {
  _RTCPPayload *payload = nullptr;
  UDPPacketPtr udp_packet = _packing_app_helper(4, APP_SUBTYPE_SUB_PUBLISH_ACK, ssrc, payload);

  struct in_addr inp;
  inet_aton(ip.c_str(), &inp);
  payload->app.payload.publish_ack.ip = inp.s_addr;

  return udp_packet;
}

UDPPacketPtr Pack::packing_publish_ack(uint32_t ssrc, const std::string &ip) {
  _RTCPPayload *payload = nullptr;
  UDPPacketPtr udp_packet = _packing_app_helper(4, APP_SUBTYPE_PUBLISH_ACK, ssrc, payload);

  struct in_addr inp;
  inet_aton(ip.c_str(), &inp);
  payload->app.payload.publish_ack.ip = inp.s_addr;

  return udp_packet;
}

UDPPacketPtr Pack::packing_play(uint32_t ssrc, uint32_t target_ssrc) {
  _RTCPPayload *payload = nullptr;
  UDPPacketPtr udp_packet = _packing_app_helper(4, APP_SUBTYPE_PLAY, ssrc, payload);

  payload->app.payload.play.target_ssrc = htonl(target_ssrc);

  return udp_packet;
}

UDPPacketPtr Pack::packing_play_ack(uint32_t ssrc, uint32_t target_ssrc) {
  _RTCPPayload *payload = nullptr;
  UDPPacketPtr udp_packet = _packing_app_helper(4, APP_SUBTYPE_PLAY_ACK, ssrc, payload);

  payload->app.payload.play_ack.target_ssrc = htonl(target_ssrc);

  return udp_packet;
}

UDPPacketPtr Pack::packing_unpublish_ack(uint32_t ssrc) {
  _RTCPPayload *payload = nullptr;
  UDPPacketPtr udp_packet = _packing_app_helper(4, APP_SUBTYPE_UN_PUBLISH_ACK, ssrc, payload);

  payload->app.payload.unpublish_ack.dummy[0] = 0x0;

  return udp_packet;
}

UDPPacketPtr Pack::packing_unplay_ack(uint32_t ssrc, uint32_t target_ssrc) {
  _RTCPPayload *payload = nullptr;
  UDPPacketPtr udp_packet = _packing_app_helper(4, APP_SUBTYPE_UN_PLAY_ACK, ssrc, payload);

  payload->app.payload.unplay_ack.target_ssrc = htonl(target_ssrc);

  return udp_packet;
}

UDPPacketPtr Pack::packing_pull(uint32_t ssrc, uint32_t target_ssrc) {
  _RTCPPayload *payload = nullptr;
  UDPPacketPtr udp_packet = _packing_app_helper(4, APP_SUBTYPE_PULL, ssrc, payload);

  payload->app.payload.pull.target_ssrc = htonl(target_ssrc);

  return udp_packet;
}

UDPPacketPtr Pack::packing_publish(uint32_t ssrc, uint32_t target_ssrc) {
    _RTCPPayload *payload = nullptr;
    UDPPacketPtr udp_packet = _packing_app_helper(4, APP_SUBTYPE_PUBLISH, ssrc, payload);

    payload->app.payload.push.target_ssrc = htonl(target_ssrc);

    return udp_packet;
}

UDPPacketPtr Pack::packing_push(uint32_t ssrc, uint32_t target_ssrc) {
  _RTCPPayload *payload = nullptr;
  UDPPacketPtr udp_packet = _packing_app_helper(4, APP_SUBTYPE_PUSH, ssrc, payload);

  payload->app.payload.push.target_ssrc = htonl(target_ssrc);

  return udp_packet;
}

UDPPacketPtr Pack::packing_pull_ack(uint32_t local_ssrc,
                                    uint32_t remote_ssrc,
                                    uint32_t target_ssrc)
{
  _RTCPPayload *payload = nullptr;
  UDPPacketPtr udp_packet = _packing_app_helper(5, APP_SUBTYPE_PULL_ACK, local_ssrc, payload);

  payload->app.payload.pull_ack.target_ssrc = htonl(target_ssrc);
  payload->app.payload.pull_ack.remote_ssrc = htonl(remote_ssrc);

  return udp_packet;
}

UDPPacketPtr Pack::packing_push_ack(uint32_t local_ssrc,
                                    uint32_t remote_ssrc,
                                    uint32_t target_ssrc)
{
  _RTCPPayload *payload = nullptr;
  UDPPacketPtr udp_packet = _packing_app_helper(5, APP_SUBTYPE_PUSH_ACK, local_ssrc, payload);

  payload->app.payload.push_ack.target_ssrc = htonl(target_ssrc);
  payload->app.payload.push_ack.remote_ssrc = htonl(remote_ssrc);

  return udp_packet;
}

UDPPacketPtr Pack::packing_unpull(uint32_t ssrc, uint32_t target_ssrc) {
  _RTCPPayload *payload = nullptr;
  UDPPacketPtr udp_packet = _packing_app_helper(4, APP_SUBTYPE_UN_PULL, ssrc, payload);

  payload->app.payload.unpull.target_ssrc = htonl(target_ssrc);

  return udp_packet;
}

UDPPacketPtr Pack::packing_unpush(uint32_t ssrc, uint32_t target_ssrc) {
  _RTCPPayload *payload = nullptr;
  UDPPacketPtr udp_packet = _packing_app_helper(4, APP_SUBTYPE_UN_PUSH, ssrc, payload);

  payload->app.payload.unpush.target_ssrc = htonl(target_ssrc);

  return udp_packet;
}

UDPPacketPtr Pack::packing_unpull_ack(uint32_t local_ssrc,
                                    uint32_t remote_ssrc,
                                    uint32_t target_ssrc)
{
  _RTCPPayload *payload = nullptr;
  UDPPacketPtr udp_packet = _packing_app_helper(5, APP_SUBTYPE_UN_PULL_ACK, local_ssrc, payload);

  payload->app.payload.unpull_ack.target_ssrc = htonl(target_ssrc);
  payload->app.payload.unpull_ack.remote_ssrc = htonl(remote_ssrc);

  return udp_packet;
}

UDPPacketPtr Pack::packing_unpush_ack(uint32_t local_ssrc,
                                    uint32_t remote_ssrc,
                                    uint32_t target_ssrc)
{
  _RTCPPayload *payload = nullptr;
  UDPPacketPtr udp_packet = _packing_app_helper(5, APP_SUBTYPE_UN_PUSH_ACK, local_ssrc, payload);

  payload->app.payload.unpush_ack.target_ssrc = htonl(target_ssrc);
  payload->app.payload.unpush_ack.remote_ssrc = htonl(remote_ssrc);

  return udp_packet;
}


    UDPPacketPtr Pack::packing_pull_new(uint32_t ssrc, uint32_t target_ssrc, const char *extra_data, int16_t extra_length) {
        _RTCPPayload *payload = nullptr;
        UDPPacketPtr udp_packet;
        if (extra_data || extra_length == 0) {
            udp_packet = _packing_app_helper(4, APP_SUBTYPE_PULL, ssrc, payload);
            payload->app.payload.pull.target_ssrc = htonl(target_ssrc);
        } else {
            udp_packet = std::make_shared<UDPPacket>(13);

            uint16_t len_in_word = (extra_length + 20)/4;
            _packing_app_header(udp_packet, len_in_word, APP_SUBTYPE_PULL);

            payload = reinterpret_cast<_RTCPPayload *>(udp_packet->mutable_buffer() + RTCP_HEADER_SIZE_BYTES);
            payload->app.ssrc = htonl(ssrc);
            memset(payload->app.name, 0x0, sizeof(uint32_t));
            udp_packet->mod_length(len_in_word * 4);
            payload->app.payload.pull.target_ssrc = htonl(target_ssrc);
            payload->app.payload.pull.cls_ext_length = htons(extra_length);
            memcpy(payload->app.payload.pull.cls_ext_data,extra_data,extra_length);
        }

        return udp_packet;
    }

    UDPPacketPtr Pack::packing_push_new(uint32_t ssrc, uint32_t target_ssrc, const char *extra_data, int16_t extra_length) {
        _RTCPPayload *payload = nullptr;
        UDPPacketPtr udp_packet;
        if (extra_data || extra_length == 0) {
            udp_packet = _packing_app_helper(4, APP_SUBTYPE_PUSH, ssrc, payload);
            payload->app.payload.push.target_ssrc = htonl(target_ssrc);
        } else {
            udp_packet = std::make_shared<UDPPacket>(13);

            uint16_t len_in_word = (extra_length + 20)/4;
            _packing_app_header(udp_packet, len_in_word, APP_SUBTYPE_PUSH);

            payload = reinterpret_cast<_RTCPPayload *>(udp_packet->mutable_buffer() + RTCP_HEADER_SIZE_BYTES);
            payload->app.ssrc = htonl(ssrc);
            memset(payload->app.name, 0x0, sizeof(uint32_t));
            udp_packet->mod_length(len_in_word * 4);
            payload->app.payload.push.target_ssrc = htonl(target_ssrc);
            payload->app.payload.push.cls_ext_length = htons(extra_length);
            memcpy(payload->app.payload.push.cls_ext_data,extra_data,extra_length);
        }


        return udp_packet;
    }

    UDPPacketPtr Pack::packing_pull_ack_new(uint32_t local_ssrc,
                                        uint32_t remote_ssrc,
                                        uint32_t target_ssrc,
                                        const char *extra_data,
                                        int16_t extra_length)
    {
        _RTCPPayload *payload = nullptr;
        UDPPacketPtr udp_packet;
        if (extra_data || extra_length == 0){
            udp_packet = _packing_app_helper(5, APP_SUBTYPE_PULL_ACK, local_ssrc, payload);
            payload->app.payload.pull_ack.target_ssrc = htonl(target_ssrc);
            payload->app.payload.pull_ack.remote_ssrc = htonl(remote_ssrc);
        } else {
            udp_packet = std::make_shared<UDPPacket>(13);
            uint16_t len_in_word = (extra_length + 24)/4;
            _packing_app_header(udp_packet, len_in_word, APP_SUBTYPE_PULL_ACK);

            payload = reinterpret_cast<_RTCPPayload *>(udp_packet->mutable_buffer() + RTCP_HEADER_SIZE_BYTES);
            payload->app.ssrc = htonl(local_ssrc);
            memset(payload->app.name, 0x0, sizeof(uint32_t));
            udp_packet->mod_length(len_in_word * 4);
            payload->app.payload.pull_ack.cls_ext_length = htons(extra_length);
            memcpy(payload->app.payload.pull_ack.cls_ext_data,extra_data,extra_length);
            payload->app.payload.pull_ack.target_ssrc = htonl(target_ssrc);
            payload->app.payload.pull_ack.remote_ssrc = htonl(remote_ssrc);
        }

        return udp_packet;
    }

    UDPPacketPtr Pack::packing_push_ack_new(uint32_t local_ssrc,
                                        uint32_t remote_ssrc,
                                        uint32_t target_ssrc,
                                        const char *extra_data,
                                        int16_t extra_length)
    {
        _RTCPPayload *payload = nullptr;
        UDPPacketPtr udp_packet;
        if (extra_data || extra_length == 0){
            udp_packet = _packing_app_helper(5, APP_SUBTYPE_PUSH_ACK, local_ssrc, payload);
            payload->app.payload.push_ack.target_ssrc = htonl(target_ssrc);
            payload->app.payload.push_ack.remote_ssrc = htonl(remote_ssrc);
        } else {
            udp_packet = std::make_shared<UDPPacket>(13);
            uint16_t len_in_word = (extra_length + 24)/4;
            _packing_app_header(udp_packet, len_in_word, APP_SUBTYPE_PUSH_ACK);

            payload = reinterpret_cast<_RTCPPayload *>(udp_packet->mutable_buffer() + RTCP_HEADER_SIZE_BYTES);
            payload->app.ssrc = htonl(local_ssrc);
            memset(payload->app.name, 0x0, sizeof(uint32_t));
            udp_packet->mod_length(len_in_word * 4);
            payload->app.payload.push_ack.cls_ext_length = htons(extra_length);
            memcpy(payload->app.payload.push_ack.cls_ext_data,extra_data,extra_length);
            payload->app.payload.push_ack.target_ssrc = htonl(target_ssrc);
            payload->app.payload.push_ack.remote_ssrc = htonl(remote_ssrc);
        }

        return udp_packet;
    }

    static void _packing_rtp_header(UDPPacketPtr udp_packet, uint16_t sequence_number, uint32_t timestamp, uint32_t ssrc) {
        _RTPHeader *header = reinterpret_cast<_RTPHeader *>(udp_packet->mutable_buffer());
        header->version         = RTP_VERSION;
        header->padding         = 0;
        header->csrc_count      = 0;
        header->extension       = 0;
        header->packet_type     = PT_H264;
        header->marker          = 0;
        header->sequence_number = htons(sequence_number);
        header->timestamp       = htonl(timestamp);
        header->ssrc            = htonl(ssrc);
    }

    UDPPacketPtr Pack::packing_rtp_test(uint16_t sequence, uint32_t timestamp, uint32_t ssrc){
        UDPPacketPtr udp_packet;
        udp_packet = std::make_shared<UDPPacket>(13);
        _packing_rtp_header(udp_packet, sequence, timestamp, ssrc);
        _RTPPayload *payload = nullptr;
        payload = reinterpret_cast<_RTPPayload *>(udp_packet->mutable_buffer() + RTP_HEADER_SIZE_BYTES);
        memset(payload->testbuf, 0x0, sizeof(uint8_t));
        udp_packet->mod_length(RTP_HEADER_SIZE_BYTES + 1300);

        return udp_packet;
    }

}; // namespace cls
