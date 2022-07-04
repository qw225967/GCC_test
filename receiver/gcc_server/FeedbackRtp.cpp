
#include "FeedbackRtp.hpp"

namespace RTC {
namespace RTCP {
/* Class methods. */

template <typename Item>
FeedbackRtpItemsPacket<Item> *
FeedbackRtpItemsPacket<Item>::Parse(const uint8_t *data, size_t len) {

  if (len < Packet::CommonHeaderSize + FeedbackPacket::HeaderSize) {
    return nullptr;
  }

  // NOLINTNEXTLINE(llvm-qualified-auto)
  auto *commonHeader =
      const_cast<CommonHeader *>(reinterpret_cast<const CommonHeader *>(data));

  std::unique_ptr<FeedbackRtpItemsPacket<Item>> packet(
      new FeedbackRtpItemsPacket<Item>(commonHeader));

  size_t offset = Packet::CommonHeaderSize + FeedbackPacket::HeaderSize;

  while (len > offset) {
    auto *item = FeedbackItem::Parse<Item>(data + offset, len - offset);

    if (item) {
      packet->AddItem(item);
      offset += item->GetSize();
    } else {
      break;
    }
  }

  return packet.release();
}

/* Instance methods. */

template <typename Item>
size_t FeedbackRtpItemsPacket<Item>::Serialize(uint8_t *buffer) {

  size_t offset = FeedbackPacket::Serialize(buffer);

  for (auto *item : this->items) {
    offset += item->Serialize(buffer + offset);
  }

  return offset;
}

template <typename Item> void FeedbackRtpItemsPacket<Item>::Dump() const {
}
} // namespace RTCP
} // namespace RTC
