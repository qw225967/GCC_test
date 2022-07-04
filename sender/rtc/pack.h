/**
 * @file   pack.h
 * @author pengrl
 *
 */

#pragma once

#include <memory>
#include <vector>
#include <stdint.h>
#include "rtc_fd.hpp"
#include "../util/chef_constructor_magic.hpp"
#include "io_udp_fd.hpp"

namespace cls {

class Pack {
  public:
    static UDPPacketPtr packing_remb(uint32_t sender_ssrc,
                                     const std::vector<uint32_t> &media_ssrcs,
                                     uint64_t bitrate_bps);

    static UDPPacketPtr packing_remb(uint32_t sender_ssrc,
                                     uint32_t  &media_ssrc,
                                     uint64_t bitrate_bps);

    static UDPPacketPtr packing_nack(uint32_t sender_ssrc,
                                     uint32_t media_ssrc,
                                     const std::vector<uint16_t> &sequence_vector);

    /**
     * @param delay_since_lastsr_ms 对应上次sr过去的时间，单位毫秒
     *
     */
    static UDPPacketPtr packing_rr(uint32_t sender_ssrc,
                                   uint32_t media_ssrc,
                                   uint8_t fractionlost,
                                   uint32_t lost,
                                   uint16_t cycles,
                                   uint16_t highestseqnum,
                                   uint32_t jitter,
                                   uint32_t lastsr,
                                   uint32_t delay_since_lastsr_ms);

    static UDPPacketPtr packing_sr(uint32_t ssrc,
                                   uint32_t msw,
                                   uint32_t lsw,
                                   uint32_t rtp_timestamp,
                                   uint32_t packet_count,
                                   uint32_t octet_count);

    // @NOTICE 暗改sr，下行发送sr后接收rr计算得到的rtt再通过sr的扩展字段发送给播放端
    static UDPPacketPtr packing_hack_sr(uint32_t ssrc,
                                        uint32_t msw,
                                        uint32_t lsw,
                                        uint32_t rtp_timestamp,
                                        uint32_t packet_count,
                                        uint32_t octet_count,
                                        uint32_t rtt_ms);

    static UDPPacketPtr packing_forward(uint32_t ssrc, uint32_t rtt_ms);
    static UDPPacketPtr packing_fir(uint32_t ssrc);

  public:
    static bool unpacking_nack(RTCPPacketPtr rtcp_packet, std::vector<uint16_t> &sequence_vector /*out*/);

  public:
    static UDPPacketPtr packing_publish_ack(uint32_t ssrc, const std::string &ip);
    static UDPPacketPtr packing_play(uint32_t ssrc, uint32_t target_ssrc);
    static UDPPacketPtr packing_play_ack(uint32_t ssrc, uint32_t target_ssrc);
    static UDPPacketPtr packing_unpublish_ack(uint32_t ssrc);
    static UDPPacketPtr packing_unplay_ack(uint32_t ssrc, uint32_t target_ssrc);
    static UDPPacketPtr packing_subpublish_ack(uint32_t ssrc, const std::string &ip);

  public:
    static UDPPacketPtr packing_pull(uint32_t ssrc, uint32_t target_ssrc);
    static UDPPacketPtr packing_pull_ack(uint32_t local_ssrc,
                                         uint32_t remote_ssrc,
                                         uint32_t target_ssrc);
    static UDPPacketPtr packing_unpull(uint32_t ssrc, uint32_t target_ssrc);
    static UDPPacketPtr packing_unpull_ack(uint32_t local_ssrc,
                                           uint32_t remote_ssrc,
                                           uint32_t target_ssrc);

  public:
    static UDPPacketPtr packing_push(uint32_t ssrc, uint32_t target_ssrc);
    static UDPPacketPtr packing_push_ack(uint32_t local_ssrc,
                                         uint32_t remote_ssrc,
                                         uint32_t target_ssrc);
    static UDPPacketPtr packing_unpush(uint32_t ssrc, uint32_t target_ssrc);
    static UDPPacketPtr packing_unpush_ack(uint32_t local_ssrc,
                                           uint32_t remote_ssrc,
                                           uint32_t target_ssrc);
    static UDPPacketPtr packing_pull_new(uint32_t ssrc, uint32_t target_ssrc, const char *extra_data, int16_t extra_length);
    static UDPPacketPtr packing_push_new(uint32_t ssrc, uint32_t target_ssrc, const char *extra_data, int16_t extra_length);
    static UDPPacketPtr packing_pull_ack_new(uint32_t local_ssrc,
                                             uint32_t remote_ssrc,
                                             uint32_t target_ssrc,
                                             const char *extra_data,
                                             int16_t extra_length);
    static UDPPacketPtr packing_push_ack_new(uint32_t local_ssrc,
                                                   uint32_t remote_ssrc,
                                                   uint32_t target_ssrc,
                                                   const char *extra_data,
                                                   int16_t extra_length);


    static UDPPacketPtr packing_rtp_test(uint16_t sequence, uint32_t timestamp, uint32_t ssrc);

    static UDPPacketPtr packing_publish(uint32_t ssrc, uint32_t target_ssrc);

  CHEF_DISALLOW_IMPLICIT_CONSTRUCTORS(Pack);

}; // class Pack

}; // namespace cls
