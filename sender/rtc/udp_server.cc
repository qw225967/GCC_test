#include "udp_server.h"
#include "udp_packet.hpp"
#include "rtcp_packet.h"
#include "log_adapter.hpp"

namespace cls {

UDPServer::UDPServer(const std::vector<std::string> &ips,
                     std::vector<uint16_t> ports,
                     UDPServerObserver *observer,
                     uint64_t crude_timer_interval_ms)
    : local_ips_(ips)
    , local_ports_(ports)
    , observer_(observer)
    , crude_timer_interval_ms_(crude_timer_interval_ms)
    , crude_timer_tick_ms_(0)
    , crude_timer_(ios_, PosixTime::milliseconds(static_cast<int64_t>(crude_timer_interval_ms_)))
{
  assert(!local_ips_.empty() && local_ips_.size() == local_ports_.size());

  for (std::size_t i = 0; i < local_ips_.size(); i++) {
    std::shared_ptr<UDPSocket> s = std::make_shared<UDPSocket>(ios_);
    sockets_.push_back(s);
    std::shared_ptr<UDPPacketDeque> d = std::make_shared<UDPPacketDeque>();
    send_packets_.push_back(d);
  }
}

void UDPServer::run() {
  for (std::size_t i = 0; i < sockets_.size(); i++) {
    UDPEndpoint local_endpoint(Address::from_string(local_ips_[i].c_str()), local_ports_[i]);
    sockets_[i]->open(local_endpoint.protocol());
    sockets_[i]->set_option(UDPSocket::reuse_address(true));
    sockets_[i]->bind(local_endpoint);
    do_receive_from(i);
  }

  if (observer_) {
    do_timer(true);
  }

  run_service();
}

void UDPServer::run_service() {
  try {
    CLS_LOG_INFO << "Asio run service.";
    ios_.run();
  } catch (std::exception &e) {
    CLS_LOG_ERROR << "Asio exception: " << e.what() << "\n";
  }
}

void UDPServer::do_receive_from(int index) {
  UDPPacketPtr pkt = std::make_shared<UDPPacket>(6);
  sockets_[index]->async_receive_from(boost::asio::buffer(pkt->mutable_buffer(), pkt->capacity()),
                                      pkt->mutable_endpoint(),
                                      std::bind(&UDPServer::handle_receive_from,
                                                this,
                                                index,
                                                pkt,
                                                std::placeholders::_1,
                                                std::placeholders::_2));
}

void UDPServer::handle_receive_from(int index, UDPPacketPtr pkt, const ErrorCode &ec, std::size_t bytes_recvd) {
  if (ec == boost::asio::error::operation_aborted) {
    CLS_LOG_ERROR << "Asio handle recv operation aborted. ec:" << ec;
    return;
  }
  if (ec || bytes_recvd == 0 || bytes_recvd > UDPPacket::UDP_PACKET_MAX_BYTES) {
    CLS_LOG_ERROR << "Asio recv error. ec:" << ec << ", bytes_recvd:" << bytes_recvd;
    do_receive_from(index);
    return;
  }

  stat_recvd_packet_num_++;
  stat_recv_ep_2_count_[pkt->mutable_endpoint()]++;

//  CLS_LOG_TRACE << "Asio handle recv. bytes_recvd:" << bytes_recvd;
  // CLS_LOG_TRACE << "-----\n" << Helper::bytes_to_hex(pkt->const_buffer(), bytes_recvd < 16 ? bytes_recvd : 16);

  pkt->mod_length(bytes_recvd);
  //pkt->mod_arrival_ms(Helper::now_ms());
  if (observer_) {
    observer_->recv_udp_cb(pkt, index);
  }

  do_receive_from(index);

  //================= test ================
  printf("recv:%d\n",pkt->mutable_buffer()[1]);
  RTCPPacket rtcp_packet;
  rtcp_packet.parse(pkt->mutable_buffer(), pkt->length());
  uint8_t subtype = rtcp_packet.header()->count_or_format;
  printf("subtype:%s, %d\n",showmap[subtype].c_str(),subtype);


}

void UDPServer::async_send_to(int addr_index, UDPPacketPtr pkt, const UDPEndpoint &ep) {
  stat_call_send_packet_num_++;

  ios_.post([this, addr_index, pkt, ep]() {
    bool write_in_progress = !send_packets_[addr_index]->empty();
    send_packets_[addr_index]->push_back(std::make_pair(pkt, ep));
    if (!write_in_progress) {
      do_send_to(addr_index);
    }
  });
}

void UDPServer::do_send_to(int index) {
  UDPPacketPair pair = send_packets_[index]->front();
  sockets_[index]->async_send_to(boost::asio::buffer(pair.first->const_buffer(), pair.first->length()),
                                 pair.second,
                                 std::bind(&UDPServer::handle_send_to,
                                           this,
                                           index,
                                           std::placeholders::_1,
                                           std::placeholders::_2));
}

void UDPServer::handle_send_to(int index, const ErrorCode &ec, std::size_t bytes_sent) {
  if (ec) {
    CLS_LOG_ERROR << "Asio handle send error. ec:" << ec << ", bytes_sent:" << bytes_sent;

    send_packets_[index]->pop_front();
    if (!send_packets_[index]->empty()) {
      do_send_to(index);
    }

    return;
  }

  stat_sent_packet_num_++;

  assert(!send_packets_[index]->empty());
  if (send_packets_[index]->empty()) {
    CLS_LOG_ERROR << "Asio error since send queue empty inside handle send. bytes_sent:" << bytes_sent;
    return;
  }

  UDPPacketPair pair = send_packets_[index]->front();
  assert(pair.first->length() == bytes_sent);
  if (pair.first->length() != bytes_sent) {
    CLS_LOG_ERROR << "Asio handle send error. packet size:" << pair.first->length() << ", bytes_sent:" << bytes_sent;
  }

//  CLS_LOG_TRACE << "Asio handle send. bytes_sent:" << bytes_sent;

  send_packets_[index]->pop_front();
  if (!send_packets_[index]->empty()) {
    do_send_to(index);
  }
}

void UDPServer::do_timer(bool first) {
  if (!first) {
    crude_timer_.expires_at(crude_timer_.expires_at() +
                            PosixTime::milliseconds(static_cast<int64_t>(crude_timer_interval_ms_)));
  }
  crude_timer_.async_wait(std::bind(&UDPServer::handle_crude_timer, this, std::placeholders::_1));
}

void UDPServer::handle_crude_timer(const ErrorCode &ec) {
  if (ec) {
    CLS_LOG_ERROR << "Asio handle crude timer error. ec:" << ec;
    return;
  }

  if (crude_timer_tick_ms_ == 0) {
    //begin_timestamp_ms_ = Helper::now_ms();
  }

  if (crude_timer_tick_ms_ % 10000 == 0) {
    std::ostringstream oss;
    for (auto &item : stat_recv_ep_2_count_) {
      oss << "(" << AsioAdapter::endpoint_to_string(item.first) << ":" << item.second << ")";
    }

    CLS_LOG_INFO << "ASIOSTAT recvd_packet_num:" << stat_recvd_packet_num_
                 << ", call_send_packet_num:" << stat_call_send_packet_num_
                 << ", sent_packet_num:" << stat_sent_packet_num_
                 << ", send queue[0] size:" << send_packets_[0]->size()
                 << ", recv ep count:" << oss.str();

    stat_recvd_packet_num_ = 0;
    stat_call_send_packet_num_ = 0;
    stat_sent_packet_num_ = 0;
    stat_recv_ep_2_count_.clear();
  }

  if (observer_) {
    observer_->crude_timer_cb(crude_timer_tick_ms_);
  }

  crude_timer_tick_ms_ += crude_timer_interval_ms_;

  do_timer(false);
}

void UDPServer::handle_defer_task(const ErrorCode &ec, DeadlineTimerPtr timer, Task task) {
  if (ec) {
    CLS_LOG_ERROR << "Asio handle defer task error. ec:" << ec;
    return;
  }
  task();
}

DeadlineTimerPtr UDPServer::add_defer_task(Task task, uint64_t defer_ms) {
  DeadlineTimerPtr timer =
      std::make_shared<DeadlineTimer>(ios_, PosixTime::milliseconds(static_cast<int64_t>(defer_ms)));
  timer->async_wait(std::bind(&UDPServer::handle_defer_task, this, std::placeholders::_1, timer, task));
  return timer;
}

}; // namespace cls
