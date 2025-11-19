// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/util/uuid_util.h"

#include <cstdint>
#include <set>

#include "google/protobuf/uuid.pb.h"
#include "google/protobuf/testing/googletest.h"
#include <gtest/gtest.h>

namespace google {
namespace protobuf {
namespace util {

using google::protobuf::Uuid;

namespace {

TEST(UuidUtilTest, StringConversion) {
  Uuid uuid;

  // Test parsing a standard UUID string
  EXPECT_TRUE(UuidUtil::FromString("550e8400-e29b-41d4-a716-446655440000", &uuid));
  EXPECT_EQ("550e8400-e29b-41d4-a716-446655440000", UuidUtil::ToString(uuid));

  // Test round-trip conversion
  Uuid uuid2;
  EXPECT_TRUE(UuidUtil::FromString(UuidUtil::ToString(uuid), &uuid2));
  EXPECT_EQ(uuid.high(), uuid2.high());
  EXPECT_EQ(uuid.low(), uuid2.low());

  // Test case insensitivity
  EXPECT_TRUE(UuidUtil::FromString("550E8400-E29B-41D4-A716-446655440000", &uuid));
  EXPECT_EQ("550e8400-e29b-41d4-a716-446655440000", UuidUtil::ToString(uuid));

  // Test invalid strings
  EXPECT_FALSE(UuidUtil::FromString("", &uuid));
  EXPECT_FALSE(UuidUtil::FromString("not-a-uuid", &uuid));
  EXPECT_FALSE(UuidUtil::FromString("550e8400-e29b-41d4-a716-4466554400", &uuid));  // Too short
  EXPECT_FALSE(UuidUtil::FromString("550e8400-e29b-41d4-a716-44665544000000", &uuid));  // Too long
  EXPECT_FALSE(UuidUtil::FromString("550e8400e29b-41d4-a716-446655440000", &uuid));  // Missing hyphen
  EXPECT_FALSE(UuidUtil::FromString("550g8400-e29b-41d4-a716-446655440000", &uuid));  // Invalid hex
}

TEST(UuidUtilTest, ByteConversion) {
  uint8_t bytes[16] = {
    0x55, 0x0e, 0x84, 0x00, 0xe2, 0x9b, 0x41, 0xd4,
    0xa7, 0x16, 0x44, 0x66, 0x55, 0x44, 0x00, 0x00
  };

  Uuid uuid;
  UuidUtil::FromBytes(bytes, &uuid);
  EXPECT_EQ("550e8400-e29b-41d4-a716-446655440000", UuidUtil::ToString(uuid));

  // Test round-trip
  uint8_t bytes2[16];
  UuidUtil::ToBytes(uuid, bytes2);
  for (int i = 0; i < 16; ++i) {
    EXPECT_EQ(bytes[i], bytes2[i]);
  }
}

TEST(UuidUtilTest, NilUuid) {
  Uuid nil = UuidUtil::Nil();
  EXPECT_EQ(0u, nil.high());
  EXPECT_EQ(0u, nil.low());
  EXPECT_TRUE(UuidUtil::IsNil(nil));
  EXPECT_FALSE(UuidUtil::IsValid(nil));
  EXPECT_EQ("00000000-0000-0000-0000-000000000000", UuidUtil::ToString(nil));
}

TEST(UuidUtilTest, Generate) {
  // Generate multiple UUIDs and verify they're unique
  std::set<std::string> uuids;
  for (int i = 0; i < 100; ++i) {
    Uuid uuid = UuidUtil::Generate();
    EXPECT_TRUE(UuidUtil::IsValid(uuid));
    EXPECT_FALSE(UuidUtil::IsNil(uuid));

    std::string str = UuidUtil::ToString(uuid);
    EXPECT_EQ(36u, str.size());
    EXPECT_TRUE(uuids.insert(str).second) << "Duplicate UUID generated: " << str;

    // Verify version 4
    EXPECT_EQ(4, UuidUtil::GetVersion(uuid));

    // Verify RFC 4122 variant
    EXPECT_EQ(1, UuidUtil::GetVariant(uuid));
  }
}

TEST(UuidUtilTest, Comparison) {
  Uuid u1, u2, u3;
  UuidUtil::FromString("00000000-0000-0000-0000-000000000001", &u1);
  UuidUtil::FromString("00000000-0000-0000-0000-000000000002", &u2);
  UuidUtil::FromString("00000000-0000-0000-0000-000000000001", &u3);

  EXPECT_EQ(u1, u3);
  EXPECT_NE(u1, u2);
  EXPECT_LT(u1, u2);
  EXPECT_GT(u2, u1);
  EXPECT_LE(u1, u2);
  EXPECT_LE(u1, u3);
  EXPECT_GE(u2, u1);
  EXPECT_GE(u1, u3);
}

TEST(UuidUtilTest, Version) {
  Uuid uuid;

  // Version 1 (time-based)
  UuidUtil::FromString("550e8400-e29b-11d4-a716-446655440000", &uuid);
  EXPECT_EQ(1, UuidUtil::GetVersion(uuid));

  // Version 4 (random)
  UuidUtil::FromString("550e8400-e29b-41d4-a716-446655440000", &uuid);
  EXPECT_EQ(4, UuidUtil::GetVersion(uuid));
}

TEST(UuidUtilTest, StreamOutput) {
  Uuid uuid;
  UuidUtil::FromString("550e8400-e29b-41d4-a716-446655440000", &uuid);

  std::ostringstream oss;
  oss << uuid;
  EXPECT_EQ("550e8400-e29b-41d4-a716-446655440000", oss.str());
}

}  // namespace

}  // namespace util
}  // namespace protobuf
}  // namespace google
