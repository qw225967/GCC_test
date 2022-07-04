/*
 *   Time : 2021/5/27 下午6:14 
 *   Author : fangruiqian
 *   File : rtp_packet
 *   Software: CLion
 *   Email: fangruiqian@inke.cn
 */

#pragma once

#include <string>
#include <stdint.h>
#include <arpa/inet.h>
#include "rtc_common.hpp"
#include "../util/chef_constructor_magic.hpp"
#include "../util/log_adapter.hpp"

namespace cls {

#ifndef RTCP_TEST_RTP_PACKET_H
#define RTCP_TEST_RTP_PACKET_H

    struct _RTPHeader {
#ifdef CLS_BIG_ENDIAN__
        uint8_t  version :2;
  uint8_t  padding :1;
  uint8_t  extension :1;
  uint8_t  csrc_count :4;
#else
        uint8_t  csrc_count :4;
        uint8_t  extension :1;
        uint8_t  padding :1;
        uint8_t  version :2;
#endif
#ifdef CLS_BIG_ENDIAN__
        uint8_t  marker :1;
  uint8_t  packet_type :7;
#else
        uint8_t  packet_type :7;
        uint8_t  marker :1;
#endif
        uint16_t sequence_number;
        uint32_t timestamp;
        uint32_t ssrc;

        uint16_t get_sequence_number() const { return ntohs(sequence_number); }

        uint32_t get_timestamp() const { return ntohl(timestamp); }

        uint32_t calc_timestamp_ms() const {
            if (packet_type == PT_H264 || packet_type == PT_H264M ) {
                return ntohl(timestamp) / H264_CLOCK_RATE;
            } else if (packet_type == PT_H265 || packet_type == PT_H265M) {
                return ntohl(timestamp) / H265_CLOCK_RATE;
            } else if (packet_type == PT_AAC || packet_type == PT_AACM) {
                return ntohl(timestamp) / AAC_CLOCK_RATE;
            } else if (packet_type == PT_OPUS || packet_type == PT_OPUSM) {
                return ntohl(timestamp) / OPUS_CLOCK_RATE;
            }
            CLS_LOG_ERROR << "calc_timestamp_ms type not matched. packet_type:" << (int)packet_type;
            return ntohl(timestamp);
        }

        uint32_t get_ssrc() const { return ntohl(ssrc); }
    }; // struct _RTPHeader


#pragma pack(pop)

    class RTPPacket {
    public:
        RTPPacket();

        // 内部持有但不会拷贝该份内存，所以使用后续接口时需保证<buf>依然有效
        bool parse(uint8_t *buf, std::size_t len);

        _RTPHeader *header() const { return header_; }

    private:
        _RTPHeader *header_;

        CHEF_DISALLOW_COPY_AND_ASSIGN(RTPPacket);

    }; // class RTPPacket


    struct _RTPPayload {
        _RTPHeader header;
        uint8_t    testbuf[1300];
    };

} //namespace cls



#endif //RTCP_TEST_RTP_PACKET_H
