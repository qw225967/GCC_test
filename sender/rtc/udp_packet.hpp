/**
 * @file   udp_packet.hpp
 * @author pengrl
 *
 */

#pragma once

#include <string>
#include <stdint.h>
#include "/opt/homebrew/Cellar/boost/1.76.0/include/boost/asio.hpp"
#include "asio_adapter.hpp"
#include "../util/chef_constructor_magic.hpp"

#if defined(CLS_UDP_PACKET_POOL)
#include "udp_packet_pool.hpp"
#endif // CLS_UDP_PACKET_POOL

namespace cls {

#if defined(CLS_UDP_PACKET_POOL)
template<class Dummy>
struct UDPPacket_static {
  static UDPPacketPool *pool_instance_;
};

template<class Dummy>
UDPPacketPool *UDPPacket_static<Dummy>::pool_instance_ = NULL;
#endif  // CLS_UDP_PACKET_POOL


#if defined(CLS_UDP_PACKET_POOL)
class UDPPacket : private UDPPacket_static<void> {
#else
class UDPPacket {
#endif // UDP_PACKET_MAX_BYTES
  public:
    static constexpr std::size_t UDP_PACKET_MAX_BYTES = 1400;
    static constexpr std::size_t UDP_PACKET_MIN_BYTES = 4;

  public:
    UDPPacket(int countflag) : capacity_(UDP_PACKET_MAX_BYTES) {
      length_ = 0;
#if defined(CLS_UDP_PACKET_POOL)
      buf_ = pool_instance_ ? pool_instance_->malloc(countflag) : (new uint8_t[UDP_PACKET_MAX_BYTES]);
#else
      buf_ = new uint8_t[UDP_PACKET_MAX_BYTES];
#endif // CLS_UDP_PACKET_POOL
      memset(buf_, 0x0, UDP_PACKET_MAX_BYTES);
      arrival_ms_ = 0;
    }

    ~UDPPacket() {
#if defined(CLS_UDP_PACKET_POOL)
      if (pool_instance_) {
        pool_instance_->free(buf_);
      } else {
        delete []buf_;
      }
#else
      delete []buf_;
#endif // CLS_UDP_PACKET_POOL
      buf_ = NULL;
    }

    const uint8_t *const_buffer() const { return buf_; }
    uint8_t *mutable_buffer() { return buf_; }

    std::size_t length() const { return length_; }
    void mod_length(std::size_t l) { length_ = l; }

    std::size_t capacity() const { return capacity_; }

    UDPEndpoint &mutable_endpoint() { return endpoint_; }

    uint64_t arrival_ms() const { return arrival_ms_; }
    void mod_arrival_ms(uint64_t am) { arrival_ms_ = am; }

#if defined(CLS_UDP_PACKET_POOL)
    // @NOTICE 使用前唯一初始化，否则退化成不使用pool
    static void init_pool(uint32_t init_capacity, uint32_t shrink_capacity, uint32_t item_bytes=UDP_PACKET_MAX_BYTES) {
      if (pool_instance_ == NULL) {
        pool_instance_ = new UDPPacketPool(init_capacity, shrink_capacity, item_bytes);
      }
    }
#endif // CLS_UDP_PACKET_POOL

#if defined(CLS_UDP_PACKET_POOL)
    static UDPPacketPool *pool_instance() { return pool_instance_; }
#endif // CLS_UDP_PACKET_POOL

  private:
    const std::size_t capacity_;
    std::size_t       length_;
    uint8_t           *buf_;
    UDPEndpoint       endpoint_;
    uint64_t          arrival_ms_;

  CHEF_DISALLOW_COPY_AND_ASSIGN(UDPPacket);

}; // class UDPPacket

}; // namespace cls
