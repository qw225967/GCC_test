/**
 * @file   rtc_common.hpp
 * @author pengrl
 *
 */

#pragma once

#include <map>
#include <string>
#include <stdint.h>

namespace cls {

static constexpr std::size_t RTP_HEADER_SIZE_BYTES  = 12;
static constexpr std::size_t RTCP_HEADER_SIZE_BYTES = 4;
static constexpr std::size_t RTCP_PACKET_MIN_BYTES  = 8;

static constexpr std::size_t STANDARD_SR_SIZE_BYTES = 28;
static constexpr std::size_t SR_WITH_RTT_EXT_SIZE_BYTES = 32;

static constexpr std::size_t PUBLISH_CLS_EXT_SIZE_BYTES = 16;
static constexpr std::size_t PLAY_CLS_EXT_SIZE_BYTES = 24;

static constexpr std::size_t STANDARD_PLAY_SIZE_BYTES = 16;
static constexpr std::size_t PLAY_WITH_SESSION_ID_EXT_SIZE_BYTES = 20;
static constexpr std::size_t PLAY_WITH_FIR_FLAG_EXT_SIZE_BYTES = 24;
static constexpr std::size_t STANDARD_UNPLAY_SIZE_BYTES = 16;
static constexpr std::size_t UNPLAY_WITH_SESSION_ID_EXT_SIZE_BYTES = 20;

static constexpr uint8_t RTCP_VERSION = 2;
static constexpr uint8_t RTP_VERSION  = 2;

static constexpr uint8_t PT_FORWARD = 65;
static constexpr uint8_t PT_FIR = 222;

static constexpr uint8_t PT_SR    = 200;
static constexpr uint8_t PT_RR    = 201;
static constexpr uint8_t PT_SDES  = 202;
static constexpr uint8_t PT_BYE   = 203;
static constexpr uint8_t PT_APP   = 204;
static constexpr uint8_t PT_NACK  = 205;
static constexpr uint8_t PT_REMB  = 206;
static constexpr uint8_t PT_GCCFB = 207;

static constexpr uint8_t PT_H264        = 107;
static constexpr uint8_t PT_H265        = 108;

static constexpr uint8_t PT_OPUS        = 111;
static constexpr uint8_t PT_AAC         = 113;
static constexpr uint8_t PT_RED_AUDIO   = 122;
static constexpr uint8_t PT_FEC         = 124;
static constexpr uint8_t PT_RED_VIDEO   = 125;
static constexpr uint8_t PT_H264M       = 235;
static constexpr uint8_t PT_H265M       = 236;
static constexpr uint8_t PT_OPUSM       = 239;
static constexpr uint8_t PT_AACM        = 241;
static constexpr uint8_t PT_REDM_AUDIO  = 250;
static constexpr uint8_t PT_REDM_VIDEO  = 253;

static constexpr uint8_t APP_SUBTYPE_PUBLISH            = 1;
static constexpr uint8_t APP_SUBTYPE_PUBLISH_ACK        = 1;
static constexpr uint8_t APP_SUBTYPE_PLAY               = 2;
static constexpr uint8_t APP_SUBTYPE_PLAY_ACK           = 2;
static constexpr uint8_t APP_SUBTYPE_UN_PUBLISH         = 3;
static constexpr uint8_t APP_SUBTYPE_UN_PUBLISH_ACK     = 3;
static constexpr uint8_t APP_SUBTYPE_UN_PLAY            = 4;
static constexpr uint8_t APP_SUBTYPE_UN_PLAY_ACK        = 4;
static constexpr uint8_t APP_SUBTYPE_SUB_PUBLISH        = 5;
static constexpr uint8_t APP_SUBTYPE_SUB_PUBLISH_ACK    = 5;
static constexpr uint8_t APP_SUBTYPE_UN_SUB_PUBLISH     = 6;
static constexpr uint8_t APP_SUBTYPE_UN_SUB_PUBLISH_ACK = 6;
static constexpr uint8_t APP_SUBTYPE_PINGPONG           = 7;
/// @NOTICE 7 for ping pong

static constexpr uint8_t APP_SUBTYPE_PUSH        = 16;
static constexpr uint8_t APP_SUBTYPE_PUSH_ACK    = 17;
static constexpr uint8_t APP_SUBTYPE_PULL        = 18;
static constexpr uint8_t APP_SUBTYPE_PULL_ACK    = 19;
static constexpr uint8_t APP_SUBTYPE_UN_PUSH     = 20;
static constexpr uint8_t APP_SUBTYPE_UN_PUSH_ACK = 21;
static constexpr uint8_t APP_SUBTYPE_UN_PULL     = 22;
static constexpr uint8_t APP_SUBTYPE_UN_PULL_ACK = 23;


static constexpr uint32_t H264_CLOCK_RATE = 90;
static constexpr uint32_t H265_CLOCK_RATE = 90;
static constexpr uint32_t OPUS_CLOCK_RATE = 48;
static constexpr uint32_t AAC_CLOCK_RATE  = 48;

static std::map<uint8_t, uint32_t> TYPE_2_CLOCK_RATE = {
  {PT_H264, H264_CLOCK_RATE},
  {PT_H264M, H264_CLOCK_RATE},
  {PT_H265, H265_CLOCK_RATE},
  {PT_H265M, H265_CLOCK_RATE},
  {PT_AAC, AAC_CLOCK_RATE},
  {PT_AACM, AAC_CLOCK_RATE},
  {PT_OPUS, OPUS_CLOCK_RATE},
  {PT_OPUSM, OPUS_CLOCK_RATE}
};

static constexpr uint8_t NALU_TYPE_SLICE = 1;
static constexpr uint8_t NALU_TYPE_IDR   = 5;
static constexpr uint8_t NALU_TYPE_SEI   = 6;
static constexpr uint8_t NALU_TYPE_SPS   = 7;
static constexpr uint8_t NALU_TYPE_PPS   = 8;
static constexpr uint8_t NALU_TYPE_FUA   = 28;

// static constexpr std::size_t RTP_HEADER_LENGTH = RTP_HEADER_SIZE_BYTES;
static constexpr uint8_t NALU_TYPE_MASK = 0x1F;


}; // namespace cls
