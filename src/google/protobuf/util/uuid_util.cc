// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/util/uuid_util.h"

#include <cstring>
#include <random>

#include "absl/strings/str_format.h"
#include "absl/strings/ascii.h"

namespace google {
namespace protobuf {
namespace util {

namespace {

// Converts a hex character to its integer value.
int HexToInt(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

// Converts an integer value (0-15) to a hex character.
char IntToHex(int value) {
  return "0123456789abcdef"[value & 0xF];
}

}  // namespace

std::string UuidUtil::ToString(const Uuid& uuid) {
  uint64_t high = uuid.high();
  uint64_t low = uuid.low();

  return absl::StrFormat(
      "%08x-%04x-%04x-%04x-%012x",
      static_cast<uint32_t>(high >> 32),
      static_cast<uint16_t>(high >> 16),
      static_cast<uint16_t>(high),
      static_cast<uint16_t>(low >> 48),
      low & 0xFFFFFFFFFFFFULL);
}

bool UuidUtil::FromString(absl::string_view value, Uuid* uuid) {
  // Expected format: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx (36 characters)
  if (value.size() != 36) {
    return false;
  }

  // Verify hyphens are in correct positions
  if (value[8] != '-' || value[13] != '-' ||
      value[18] != '-' || value[23] != '-') {
    return false;
  }

  uint64_t high = 0;
  uint64_t low = 0;

  // Parse first 8 hex characters (time_low)
  for (int i = 0; i < 8; ++i) {
    int v = HexToInt(value[i]);
    if (v < 0) return false;
    high = (high << 4) | v;
  }

  // Parse next 4 hex characters (time_mid)
  for (int i = 9; i < 13; ++i) {
    int v = HexToInt(value[i]);
    if (v < 0) return false;
    high = (high << 4) | v;
  }

  // Parse next 4 hex characters (time_hi_and_version)
  for (int i = 14; i < 18; ++i) {
    int v = HexToInt(value[i]);
    if (v < 0) return false;
    high = (high << 4) | v;
  }

  // Parse next 4 hex characters (clock_seq_hi_and_reserved, clock_seq_low)
  for (int i = 19; i < 23; ++i) {
    int v = HexToInt(value[i]);
    if (v < 0) return false;
    low = (low << 4) | v;
  }

  // Parse last 12 hex characters (node)
  for (int i = 24; i < 36; ++i) {
    int v = HexToInt(value[i]);
    if (v < 0) return false;
    low = (low << 4) | v;
  }

  uuid->set_high(high);
  uuid->set_low(low);
  return true;
}

void UuidUtil::ToBytes(const Uuid& uuid, uint8_t bytes[16]) {
  uint64_t high = uuid.high();
  uint64_t low = uuid.low();

  // Big-endian encoding
  for (int i = 7; i >= 0; --i) {
    bytes[i] = static_cast<uint8_t>(high);
    high >>= 8;
  }
  for (int i = 15; i >= 8; --i) {
    bytes[i] = static_cast<uint8_t>(low);
    low >>= 8;
  }
}

void UuidUtil::FromBytes(const uint8_t bytes[16], Uuid* uuid) {
  uint64_t high = 0;
  uint64_t low = 0;

  // Big-endian decoding
  for (int i = 0; i < 8; ++i) {
    high = (high << 8) | bytes[i];
  }
  for (int i = 8; i < 16; ++i) {
    low = (low << 8) | bytes[i];
  }

  uuid->set_high(high);
  uuid->set_low(low);
}

Uuid UuidUtil::Generate() {
  // Use random device and mt19937_64 for random UUID v4 generation
  std::random_device rd;
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<uint64_t> dis;

  uint64_t high = dis(gen);
  uint64_t low = dis(gen);

  // Set version to 4 (random)
  high = (high & 0xFFFFFFFFFFFF0FFFULL) | 0x0000000000004000ULL;

  // Set variant to RFC 4122
  low = (low & 0x3FFFFFFFFFFFFFFFULL) | 0x8000000000000000ULL;

  Uuid uuid;
  uuid.set_high(high);
  uuid.set_low(low);
  return uuid;
}

Uuid UuidUtil::Nil() {
  Uuid uuid;
  uuid.set_high(0);
  uuid.set_low(0);
  return uuid;
}

int UuidUtil::GetVersion(const Uuid& uuid) {
  // Version is in bits 12-15 of the high part (time_hi_and_version field)
  return static_cast<int>((uuid.high() >> 12) & 0xF);
}

int UuidUtil::GetVariant(const Uuid& uuid) {
  // Variant is in the highest bits of clock_seq_hi_and_reserved
  uint64_t variant_bits = (uuid.low() >> 62) & 0x3;

  if ((variant_bits & 0x2) == 0) {
    return 0;  // NCS backward compatibility
  } else if ((variant_bits & 0x3) == 0x2) {
    return 1;  // RFC 4122
  } else if ((variant_bits & 0x3) == 0x3) {
    return 2;  // Microsoft
  }
  return 3;  // Future reserved
}

}  // namespace util
}  // namespace protobuf
}  // namespace google
