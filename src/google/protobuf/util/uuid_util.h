// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Defines utilities for the Uuid well known type.

#ifndef GOOGLE_PROTOBUF_UTIL_UUID_UTIL_H__
#define GOOGLE_PROTOBUF_UTIL_UUID_UTIL_H__

#include <cstdint>
#include <ostream>
#include <string>

#include "google/protobuf/uuid.pb.h"
#include "absl/strings/string_view.h"

// Must be included last.
#include "google/protobuf/port_def.inc"

namespace google {
namespace protobuf {
namespace util {

// Utility functions for Uuid.
class PROTOBUF_EXPORT UuidUtil {
  typedef google::protobuf::Uuid Uuid;

 public:
  // The nil UUID (all zeros).
  static constexpr uint64_t kNilHigh = 0;
  static constexpr uint64_t kNilLow = 0;

  // Checks if the UUID is valid (non-nil).
  static bool IsValid(const Uuid& uuid) {
    return uuid.high() != 0 || uuid.low() != 0;
  }

  // Checks if the UUID is the nil UUID.
  static bool IsNil(const Uuid& uuid) {
    return uuid.high() == 0 && uuid.low() == 0;
  }

  // Converts UUID to/from string format.
  // String format: "xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx" (36 characters)
  // Example: "550e8400-e29b-41d4-a716-446655440000"
  static std::string ToString(const Uuid& uuid);
  static bool FromString(absl::string_view value, Uuid* uuid);

  // Converts UUID to/from bytes (16 bytes, big-endian).
  static void ToBytes(const Uuid& uuid, uint8_t bytes[16]);
  static void FromBytes(const uint8_t bytes[16], Uuid* uuid);

  // Generates a random UUID v4 (random).
  static Uuid Generate();

  // Creates a nil UUID (all zeros).
  static Uuid Nil();

  // Gets the UUID version (1-5) from the version field.
  static int GetVersion(const Uuid& uuid);

  // Gets the UUID variant (RFC 4122, Microsoft, etc.).
  static int GetVariant(const Uuid& uuid);
};

}  // namespace util
}  // namespace protobuf
}  // namespace google

namespace google {
namespace protobuf {

// Relational operators for Uuid.
inline bool operator==(const Uuid& u1, const Uuid& u2) {
  return u1.high() == u2.high() && u1.low() == u2.low();
}

inline bool operator!=(const Uuid& u1, const Uuid& u2) {
  return !(u1 == u2);
}

inline bool operator<(const Uuid& u1, const Uuid& u2) {
  if (u1.high() == u2.high()) {
    return u1.low() < u2.low();
  }
  return u1.high() < u2.high();
}

inline bool operator>(const Uuid& u1, const Uuid& u2) {
  return u2 < u1;
}

inline bool operator<=(const Uuid& u1, const Uuid& u2) {
  return !(u2 < u1);
}

inline bool operator>=(const Uuid& u1, const Uuid& u2) {
  return !(u1 < u2);
}

inline std::ostream& operator<<(std::ostream& out, const Uuid& uuid) {
  out << google::protobuf::util::UuidUtil::ToString(uuid);
  return out;
}

}  // namespace protobuf
}  // namespace google

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_UTIL_UUID_UTIL_H__
