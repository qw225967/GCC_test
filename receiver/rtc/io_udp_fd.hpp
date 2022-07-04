/**
 * @file   io_udp_fd.hpp
 * @author pengrl
 *
 */

#ifndef _CLS_IO_UDP_IO_UDP_FD_HPP_
#define _CLS_IO_UDP_IO_UDP_FD_HPP_
#pragma once

#include <memory>

namespace cls {

class UDPPacket;
typedef std::shared_ptr<UDPPacket> UDPPacketPtr;

class UDPServer;
typedef std::shared_ptr<UDPServer> UDPServerPtr;

} // namespace cls

#endif
