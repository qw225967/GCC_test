
#include "Packet.hpp"
#include "Feedback.hpp"

namespace RTC {
namespace RTCP {
/* Namespace variables. */

uint8_t Buffer[BufferSize];

/* Class variables. */

// clang-format off
		absl::flat_hash_map<Type, std::string> Packet::type2String =
		{
			{ Type::SR,    "SR"    },
			{ Type::RR,    "RR"    },
			{ Type::SDES,  "SDES"  },
			{ Type::BYE,   "BYE"   },
			{ Type::APP,   "APP"   },
			{ Type::RTPFB, "RTPFB" },
			{ Type::PSFB,  "PSFB"  },
			{ Type::XR,    "XR"    }
		};
// clang-format on

/* Class methods. */

Packet *Packet::Parse(const uint8_t *data, size_t len) {

  // First, Currently parsing and Last RTCP packets in the compound packet.
  Packet *first{nullptr};
  Packet *current{nullptr};
  Packet *last{nullptr};

  while (len > 0u) {
    if (!Packet::IsRtcp(data, len)) {
      return first;
    }

    auto *header = const_cast<CommonHeader *>(
        reinterpret_cast<const CommonHeader *>(data));
    size_t packetLen = static_cast<size_t>(ntohs(header->length) + 1) * 4;

    if (len < packetLen) {
      return first;
    }

    switch (Type(header->packetType)) {
    case Type::RTPFB: {
      current = FeedbackRtpPacket::Parse(data, packetLen);

      break;
    }
    default: {
      current = nullptr;
    }
    }

    if (!current) {
      std::string packetType = Type2String(Type(header->packetType));

      if (Type(header->packetType) == Type::PSFB) {
        packetType += " " + FeedbackPsPacket::MessageType2String(
                                FeedbackPs::MessageType(header->count));
      } else if (Type(header->packetType) == Type::RTPFB) {
        packetType += " " + FeedbackRtpPacket::MessageType2String(
                                FeedbackRtp::MessageType(header->count));
      }

      return first;
    }

    data += packetLen;
    len -= packetLen;

    if (!first)
      first = current;
    else
      last->SetNext(current);

    last = current->GetNext() != nullptr ? current->GetNext() : current;
  }

  return first;
}

const std::string &Packet::Type2String(Type type) {

  static const std::string Unknown("UNKNOWN");

  auto it = Packet::type2String.find(type);

  if (it == Packet::type2String.end())
    return Unknown;

  return it->second;
}

/* Instance methods. */

size_t Packet::Serialize(uint8_t *buffer) {

  this->header = reinterpret_cast<CommonHeader *>(buffer);

  size_t length = (GetSize() / 4) - 1;

  // Fill the common header.
  this->header->version = 2;
  this->header->padding = 0;
  this->header->count = static_cast<uint8_t>(GetCount());
  this->header->packetType = static_cast<uint8_t>(GetType());
  this->header->length = uint16_t{htons(length)};

  return CommonHeaderSize;
}
} // namespace RTCP
} // namespace RTC
