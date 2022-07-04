/**
 * @file   rtcp_packet.h
 * @author pengrl
 *
 */

#pragma once

#include <string>
#include <stdint.h>
#include <arpa/inet.h>
#include "../util/chef_constructor_magic.hpp"

namespace cls {

#pragma pack(push)
#pragma pack(1)


// publish/play ext info,
// 扩展字段，需要检查是否存在
    struct _APP_CLS_EXT {
        int16_t  json_length;
        char     json_data[1];

        int16_t  get_json_length() const { return ntohs(json_length); }
        const char*     get_json_data() const { return json_data; }
    };

    struct _CACHE_EXT_PAYLOAD {
        uint32_t target_ssrc;
        int16_t  cls_ext_length;
        char     cls_ext_data[1];

        uint32_t get_target_ssrc() const { return ntohl(target_ssrc); }
        int16_t  get_json_length() const { return ntohs(cls_ext_length); }
        const char*     get_json_data() const { return cls_ext_data; }
    };

    struct _APP_PUBLISH_PAYLOAD {
        uint32_t      dummy;
        _APP_CLS_EXT  cls_ext;
    };

    struct _APP_PAYLOAD_DUMMY {
        uint32_t dummy[1];
    };

    struct _APP_PAYLOAD_IP {
        uint32_t ip;

        //uint32_t get_ip() const { return ip; }
    };

    struct _APP_PAYLOAD_TARGET {
        uint32_t target_ssrc;

        uint32_t get_target_ssrc() const { return ntohl(target_ssrc); }
    };

    struct _ACK_PAYLOAD_TARGET_REMOTE {
        uint32_t target_ssrc;
        uint32_t remote_ssrc;
        int16_t  cls_ext_length;
        char     cls_ext_data[1];

        uint32_t get_target_ssrc() const { return ntohl(target_ssrc); }
        uint32_t get_remote_ssrc() const { return ntohl(remote_ssrc); }
        int16_t  get_json_length() const { return ntohs(cls_ext_length); }
        const char*     get_json_data() const { return cls_ext_data; }
    };


    struct _APP_PAYLOAD_TARGET_REMOTE {
        uint32_t target_ssrc;
        uint32_t remote_ssrc;

        uint32_t get_target_ssrc() const { return ntohl(target_ssrc); }
        uint32_t get_remote_ssrc() const { return ntohl(remote_ssrc); }
    };

    struct _PLAY_PAYLOAD {
        uint32_t target_ssrc;
        uint32_t session_id;
        uint32_t fir_flag;

        _APP_CLS_EXT  cls_ext;

        uint32_t get_target_ssrc() const { return ntohl(target_ssrc); }
        uint32_t get_session_id() const { return ntohl(session_id); }
        uint32_t get_fir_flag() const { return ntohl(fir_flag); }
    };


    struct _UNPLAY_PAYLOAD {
        uint32_t target_ssrc;
        uint32_t session_id;
        uint32_t fir_flag;

        uint32_t get_target_ssrc() const { return ntohl(target_ssrc); }
        uint32_t get_session_id() const { return ntohl(session_id); }
        uint32_t get_fir_flag() const { return ntohl(fir_flag); }
    };

    union _RTCPAppPayload {
        _PLAY_PAYLOAD         play;
        _UNPLAY_PAYLOAD       unplay;
        _APP_PUBLISH_PAYLOAD  publish;
        _APP_PAYLOAD_IP       publish_ack;
        _APP_PAYLOAD_TARGET   play_ack;
        _APP_PAYLOAD_DUMMY    unpublish;
        _APP_PAYLOAD_DUMMY    unpublish_ack;
        _APP_PAYLOAD_TARGET   unplay_ack;

        _CACHE_EXT_PAYLOAD         pull;
        _ACK_PAYLOAD_TARGET_REMOTE pull_ack;
        _APP_PAYLOAD_TARGET        unpull;
        _APP_PAYLOAD_TARGET_REMOTE unpull_ack;

        _CACHE_EXT_PAYLOAD         push;
        _ACK_PAYLOAD_TARGET_REMOTE push_ack;
        _APP_PAYLOAD_TARGET        unpush;
        _APP_PAYLOAD_TARGET_REMOTE unpush_ack;


    }; // union _RTCPAppPayload

    struct _FCI {
        uint16_t pid;
        uint16_t blp;

        uint16_t get_pid() const { return ntohs(pid); }
        uint16_t get_blp() const { return ntohs(blp); }

    }; // struct _FCI

    union _RTCPPayload {
        struct {
            uint32_t ssrc;
            uint32_t rtt;

            uint32_t get_rtt() const { return ntohl(rtt); }
        } forward;

        struct {
            uint32_t ssrc;
        } fir;

        struct {
            uint32_t ssrc;
            uint32_t msw;
            uint32_t lsw;
            uint32_t rtp_timestamp;
            uint32_t packet_count;
            uint32_t octet_count;
            uint32_t rtt; // 扩展字段，需要检查是否存在

            uint32_t get_middle_ntp() const {
                uint64_t ntp = ntohl(msw);
                ntp = (ntp << 32) | ntohl(lsw);
                return ((ntp << 16) >> 32);
            }

            uint32_t get_rtp_timestamp() const { return ntohl(rtp_timestamp); }
            uint32_t get_packet_count() const { return ntohl(packet_count); }
            uint32_t get_octet_count() const { return ntohl(octet_count); }
            uint32_t get_rtt() const { return ntohl(rtt); }
        } sr;

        struct {
            uint32_t sender_ssrc;
            uint32_t media_ssrc;
            uint32_t fractionlost :8;
            uint32_t lost :24;
            uint32_t cycles :16;
            uint32_t highestseqnum :16;
            uint32_t jitter;
            uint32_t lastsr;
            uint32_t delay_since_lastsr;

            uint32_t get_sender_ssrc() const { return ntohl(sender_ssrc); }
            uint32_t get_media_ssrc() const { return ntohl(media_ssrc); }
            uint32_t get_lastsr() const { return ntohl(lastsr); }
            uint32_t get_delay_since_lastsr() const { return ntohl(delay_since_lastsr); }
            uint32_t get_delay_since_lastsr_ms() const { return (ntohl(delay_since_lastsr) * 1000 / 65536); }
        } rr;

        struct {
            uint32_t sender_ssrc;
            uint32_t media_ssrc;
            _FCI     fci[1];

            uint32_t get_sender_ssrc() const { return ntohl(sender_ssrc); }
            uint32_t get_media_ssrc() const { return ntohl(media_ssrc); }
        } nack;

        struct {
            uint32_t sender_ssrc;
            uint32_t media_ssrc; // unused
            uint32_t unique_identifier;
            uint8_t  ssrcs_size;
            uint8_t  br_dummy; // 占位
            uint16_t br_dummy2;
            uint32_t ssrcs[1];
        } remb;

        struct {
            uint32_t        ssrc;
            uint8_t         name[4];
            _RTCPAppPayload payload;
        } app;
    }; // union _RTCPPayload

    struct _RTCPHeader {
#ifdef CLS_BIG_ENDIAN__
        uint8_t  version :2;
  uint8_t  padding :1;
  uint8_t  count_or_format :5;
#else
        uint8_t  count_or_format :5;
        uint8_t  padding :1;
        uint8_t  version :2;
#endif
        uint8_t  packet_type;
        uint16_t length;

        uint16_t get_length() const { return (ntohs(length)+1) * 4; }
    }; // struct _RTCPHeader

#pragma pack(pop)

    class RTCPPacket {
    public:
        RTCPPacket();

        // 内部持有但不会拷贝该份内存，所以使用后续接口时需保证<buf>依然有效
        bool parse(uint8_t *buf, std::size_t len);

        uint32_t ssrc() const { return ntohl(payload_->sr.ssrc); }
        _RTCPHeader *header() { return header_; }
        _RTCPPayload *payload() const { return payload_; }

    private:
        _RTCPHeader  *header_;
        _RTCPPayload *payload_;

        CHEF_DISALLOW_COPY_AND_ASSIGN(RTCPPacket);

    }; // class RTCPPackage

}; // namespace cls

