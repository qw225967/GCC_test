/**
 * @file   udp_server.h
 * @author pengrl
 *
 */

#pragma once

#include <deque>
#include <memory>
#include <functional>
#include "asio_adapter.hpp"
#include "io_udp_fd.hpp"
#include "chef_constructor_magic.hpp"

namespace cls {
static uint64_t GetCurrentStamp64() {
  boost::posix_time::ptime epoch(boost::gregorian::date(1970, boost::gregorian::Jan, 1));
  boost::posix_time::time_duration time_from_epoch =
      boost::posix_time::microsec_clock::universal_time() - epoch;
  return time_from_epoch.total_microseconds() / 1000;
}

class UDPServerObserver {
  public:
    virtual ~UDPServerObserver() {}

    // 接收到udp包的回调
    virtual void recv_udp_cb(UDPPacketPtr udp_packet , int addr_index) = 0;
    // 基础定时器回调
    virtual void crude_timer_cb(uint64_t tick_ms) = 0;

}; // class UDPServerObserver

class UDPServer {
  public:
    typedef std::function<void(void)> Task;

  public:
    /**
     * @param ips                     监听ip数组
     * @param ports                   监听端口数组，ip和端口下标一一对应
     * @param observer                观察者回调
     * @param crude_timer_interval_ms 基础定时器时间间隔，单位毫秒
     *
     */
    UDPServer(const std::vector<std::string> &ips,
              std::vector<uint16_t> ports,
              UDPServerObserver *observer,
              uint64_t crude_timer_interval_ms=100);

    /**
     * @brief 开启服务，阻塞函数
     *
     */
    void run();

    /**
     * @brief 异步发送udp包
     *
     * @param addr_index 本地地址编号
     * @param pkt        待发送的包
     * @param ep         远端地址
     *
     */
    void async_send_to(int addr_index, UDPPacketPtr pkt, const UDPEndpoint &ep);

    /**
     * @brief 添加延时任务，如果需要定时器功能，可以在task的结尾处再次添加该任务
     *
     */
    DeadlineTimerPtr add_defer_task(Task task, uint64_t defer_ms);

    /**
     * @function current_crude_tick_ms      获取当前tick值，首次为0，精度受定时器间隔及事件循环忙碌程度影响
     * @function current_crude_timestamp_ms 获取当前时间戳
     *
     */
    uint64_t current_crude_tick_ms() const { return crude_timer_tick_ms_; }
    uint64_t current_crude_timestamp_ms() const { return begin_timestamp_ms_ + crude_timer_tick_ms_; }

  private:
    void do_receive_from(int index);
    void handle_receive_from(int index, UDPPacketPtr pkt, const ErrorCode &ec, std::size_t bytes_recvd);
    void do_send_to(int index);
    void handle_send_to(int index, const ErrorCode &ec, std::size_t bytes_sent);
    void do_timer(bool first);
    void handle_crude_timer(const ErrorCode &ec);
    void handle_defer_task(const ErrorCode &ec, DeadlineTimerPtr timer, Task task);
    void run_service();

  private:
    typedef std::pair<UDPPacketPtr, UDPEndpoint> UDPPacketPair;
    typedef std::deque<UDPPacketPair>            UDPPacketDeque;

  private:
    IOService                                     ios_;
    std::vector<std::string>                      local_ips_;
    std::vector<uint16_t>                         local_ports_;
    std::vector<std::shared_ptr<UDPSocket> >      sockets_;
    std::vector<std::shared_ptr<UDPPacketDeque> > send_packets_;
    UDPServerObserver                             *observer_;
    const uint64_t                                crude_timer_interval_ms_;
    uint64_t                                      crude_timer_tick_ms_;
    uint64_t                                      begin_timestamp_ms_;
    DeadlineTimer                                 crude_timer_;
    uint64_t                                      stat_recvd_packet_num_=0;
    uint64_t                                      stat_call_send_packet_num_=0;
    uint64_t                                      stat_sent_packet_num_=0;
    std::map<UDPEndpoint, uint64_t>               stat_recv_ep_2_count_;


  public:
    std::map<uint8_t,std::string>                 showmap;

  CHEF_DISALLOW_COPY_AND_ASSIGN(UDPServer);

}; // class UDPServer

}; // namespace cls
