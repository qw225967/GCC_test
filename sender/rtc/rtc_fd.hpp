/**
 * @file   rtc_fd.hpp
 * @author pengrl
 *
 */

#ifndef _CLS_RTC_RTC_FD_HPP_
#define _CLS_RTC_RTC_FD_HPP_
#pragma once

#include <memory>

namespace cls {

class Pack;
class RtcHelper;

class NACKGeneratorInterface;
typedef std::shared_ptr<NACKGeneratorInterface> NACKGeneratorInterfacePtr;

class NACKGenerator;
typedef std::shared_ptr<NACKGenerator> NACKGeneratorPtr;

class RRGenerator;
typedef std::shared_ptr<RRGenerator> RRGeneratorPtr;

class RTCServer;
typedef std::shared_ptr<RTCServer> RTCServerPtr;

class RTCPPacket;
typedef std::shared_ptr<RTCPPacket> RTCPPacketPtr;

class RTPPacket;
typedef std::shared_ptr<RTPPacket> RTPPacketPtr;

class RTPQueue;
typedef std::shared_ptr<RTPQueue> RTPQueuePtr;

class RTPQueue;
typedef std::shared_ptr<RTPQueue> RTPQueuePtr;

class RTT;
typedef std::shared_ptr<RTT> RTTPtr;

} // namespace cls

#endif
