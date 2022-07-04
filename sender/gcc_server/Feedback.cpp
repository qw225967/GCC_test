#define MS_CLASS "RTC::RTCP::Feedback"
// #define MS_LOG_DEV_LEVEL 3

#include "Feedback.hpp"
#include "FeedbackRtpTransport.hpp"
#include "Utils.hpp"
#include <cstring>

namespace RTC {
namespace RTCP {
/* Class methods. */

template <typename T>
const std::string &
FeedbackPacket<T>::MessageType2String(typename T::MessageType type) {
  static const std::string Unknown("UNKNOWN");

  auto it = FeedbackPacket<T>::type2String.find(type);

  if (it == FeedbackPacket<T>::type2String.end())
    return Unknown;

  return it->second;
}

/* Instance methods. */

template <typename T>
FeedbackPacket<T>::FeedbackPacket(CommonHeader *commonHeader)
    : Packet(commonHeader),
      messageType(typename T::MessageType(commonHeader->count)) {
  this->header = reinterpret_cast<Header *>(
      reinterpret_cast<uint8_t *>(commonHeader) + Packet::CommonHeaderSize);
}

template <typename T>
FeedbackPacket<T>::FeedbackPacket(typename T::MessageType messageType,
                                  uint32_t senderSsrc, uint32_t mediaSsrc)
    : Packet(rtcpType), messageType(messageType) {
  this->raw = new uint8_t[HeaderSize];
  this->header = reinterpret_cast<Header *>(this->raw);
  this->header->senderSsrc = uint32_t{htonl(senderSsrc)};
  this->header->mediaSsrc = uint32_t{htonl(mediaSsrc)};
}

template <typename T> FeedbackPacket<T>::~FeedbackPacket<T>() {
  delete[] this->raw;
}

/* Instance methods. */

template <typename T> size_t FeedbackPacket<T>::Serialize(uint8_t *buffer) {

  size_t offset = Packet::Serialize(buffer);

  // Copy the header.
  std::memcpy(buffer + offset, this->header, HeaderSize);

  return offset + HeaderSize;
}

template <typename T> void FeedbackPacket<T>::Dump() const {
}

/* Specialization for Ps class. */

template <> RTCP::Type FeedbackPacket<FeedbackPs>::rtcpType = RTCP::Type::PSFB;

// clang-format off
		template<>
		absl::flat_hash_map<FeedbackPs::MessageType, std::string> FeedbackPacket<FeedbackPs>::type2String =
		{
			{ FeedbackPs::MessageType::PLI,   "PLI"   },
			{ FeedbackPs::MessageType::SLI,   "SLI"   },
			{ FeedbackPs::MessageType::RPSI,  "RPSI"  },
			{ FeedbackPs::MessageType::FIR,   "FIR"   },
			{ FeedbackPs::MessageType::TSTR,  "TSTR"  },
			{ FeedbackPs::MessageType::TSTN,  "TSTN"  },
			{ FeedbackPs::MessageType::VBCM,  "VBCM"  },
			{ FeedbackPs::MessageType::PSLEI, "PSLEI" },
			{ FeedbackPs::MessageType::ROI,   "ROI"   },
			{ FeedbackPs::MessageType::AFB,   "AFB"   },
			{ FeedbackPs::MessageType::EXT,   "EXT"   }
		};
// clang-format on

template <>
FeedbackPacket<FeedbackPs> *
FeedbackPacket<FeedbackPs>::Parse(const uint8_t *data, size_t len) {
  return nullptr;
}

/* Specialization for Rtcp class. */

template <> Type FeedbackPacket<FeedbackRtp>::rtcpType = RTCP::Type::RTPFB;

// clang-format off
		template<>
		absl::flat_hash_map<FeedbackRtp::MessageType, std::string> FeedbackPacket<FeedbackRtp>::type2String =
		{
			{ FeedbackRtp::MessageType::NACK,   "NACK"   },
			{ FeedbackRtp::MessageType::TMMBR,  "TMMBR"  },
			{ FeedbackRtp::MessageType::TMMBN,  "TMMBN"  },
			{ FeedbackRtp::MessageType::SR_REQ, "SR_REQ" },
			{ FeedbackRtp::MessageType::RAMS,   "RAMS"   },
			{ FeedbackRtp::MessageType::TLLEI,  "TLLEI"  },
			{ FeedbackRtp::MessageType::ECN,    "ECN"    },
			{ FeedbackRtp::MessageType::PS,     "PS"     },
			{ FeedbackRtp::MessageType::EXT,    "EXT"    },
			{ FeedbackRtp::MessageType::TCC,    "TCC"    }
		};
// clang-format on

/* Class methods. */

template <>
FeedbackPacket<FeedbackRtp> *
FeedbackPacket<FeedbackRtp>::Parse(const uint8_t *data, size_t len) {

  if (len < Packet::CommonHeaderSize + FeedbackPacket::HeaderSize) {

    return nullptr;
  }

  auto *commonHeader =
      reinterpret_cast<CommonHeader *>(const_cast<uint8_t *>(data));
  FeedbackRtpPacket *packet{nullptr};

  switch (FeedbackRtp::MessageType(commonHeader->count)) {
  case FeedbackRtp::MessageType::TCC:
    packet = FeedbackRtpTransportPacket::Parse(data, len);
    break;

  default:
    break;
  }

  return packet;
}

// Explicit instantiation to have all FeedbackPacket definitions in this file.
template class FeedbackPacket<FeedbackPs>;
template class FeedbackPacket<FeedbackRtp>;
} // namespace RTCP
} // namespace RTC
