// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Tests for the canonical API aliases in MessageLite.

#include "google/protobuf/unittest.pb.h"

#include <string>

#include <gtest/gtest.h>
#include "google/protobuf/message_lite.h"

namespace google {
namespace protobuf {
namespace {

// Test that serialize() produces the same output as SerializeAsString()
TEST(CanonicalApiTest, SerializeEquivalence) {
  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(123);
  message.set_optional_string("test");

  std::string canonical = message.serialize();
  std::string original = message.SerializeAsString();

  EXPECT_EQ(canonical, original);
  EXPECT_FALSE(canonical.empty());
}

// Test that parse() works the same as ParseFromString()
TEST(CanonicalApiTest, ParseEquivalence) {
  protobuf_unittest::TestAllTypes original;
  original.set_optional_int32(456);
  original.set_optional_string("hello");

  std::string data = original.serialize();

  // Test canonical parse()
  protobuf_unittest::TestAllTypes parsed_canonical;
  EXPECT_TRUE(parsed_canonical.parse(data));
  EXPECT_EQ(parsed_canonical.optional_int32(), 456);
  EXPECT_EQ(parsed_canonical.optional_string(), "hello");

  // Test original ParseFromString()
  protobuf_unittest::TestAllTypes parsed_original;
  EXPECT_TRUE(parsed_original.ParseFromString(data));
  EXPECT_EQ(parsed_original.optional_int32(), 456);
  EXPECT_EQ(parsed_original.optional_string(), "hello");
}

// Test that clear() works the same as Clear()
TEST(CanonicalApiTest, ClearEquivalence) {
  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(789);
  message.set_optional_string("world");

  EXPECT_TRUE(message.has_optional_int32());
  EXPECT_TRUE(message.has_optional_string());

  message.clear();

  EXPECT_FALSE(message.has_optional_int32());
  EXPECT_FALSE(message.has_optional_string());
}

// Test that isInitialized() works the same as IsInitialized()
TEST(CanonicalApiTest, IsInitializedEquivalence) {
  protobuf_unittest::TestAllTypes message;

  // TestAllTypes has no required fields in proto3
  EXPECT_EQ(message.isInitialized(), message.IsInitialized());
  EXPECT_TRUE(message.isInitialized());

  message.set_optional_int32(100);
  EXPECT_EQ(message.isInitialized(), message.IsInitialized());
  EXPECT_TRUE(message.isInitialized());
}

// Test that getSerializedSize() works the same as ByteSizeLong()
TEST(CanonicalApiTest, GetSerializedSizeEquivalence) {
  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(42);
  message.set_optional_string("size test");

  size_t canonical_size = message.getSerializedSize();
  size_t original_size = message.ByteSizeLong();

  EXPECT_EQ(canonical_size, original_size);
  EXPECT_GT(canonical_size, 0u);
}

// Test that mergeFrom() works the same as MergeFromString()
TEST(CanonicalApiTest, MergeFromEquivalence) {
  protobuf_unittest::TestAllTypes source;
  source.set_optional_int32(111);

  std::string data = source.serialize();

  // Test canonical mergeFrom()
  protobuf_unittest::TestAllTypes target_canonical;
  target_canonical.set_optional_string("existing");
  EXPECT_TRUE(target_canonical.mergeFrom(data));
  EXPECT_EQ(target_canonical.optional_int32(), 111);
  EXPECT_EQ(target_canonical.optional_string(), "existing");

  // Test original MergeFromString()
  protobuf_unittest::TestAllTypes target_original;
  target_original.set_optional_string("existing");
  EXPECT_TRUE(target_original.MergeFromString(data));
  EXPECT_EQ(target_original.optional_int32(), 111);
  EXPECT_EQ(target_original.optional_string(), "existing");
}

// Test round-trip: serialize -> parse
TEST(CanonicalApiTest, RoundTrip) {
  protobuf_unittest::TestAllTypes original;
  original.set_optional_int32(12345);
  original.set_optional_int64(67890);
  original.set_optional_float(3.14f);
  original.set_optional_string("round trip test");
  original.add_repeated_int32(1);
  original.add_repeated_int32(2);
  original.add_repeated_int32(3);

  std::string data = original.serialize();

  protobuf_unittest::TestAllTypes restored;
  EXPECT_TRUE(restored.parse(data));

  EXPECT_EQ(restored.optional_int32(), 12345);
  EXPECT_EQ(restored.optional_int64(), 67890);
  EXPECT_FLOAT_EQ(restored.optional_float(), 3.14f);
  EXPECT_EQ(restored.optional_string(), "round trip test");
  EXPECT_EQ(restored.repeated_int32_size(), 3);
  EXPECT_EQ(restored.repeated_int32(0), 1);
  EXPECT_EQ(restored.repeated_int32(1), 2);
  EXPECT_EQ(restored.repeated_int32(2), 3);
}

// Test empty message
TEST(CanonicalApiTest, EmptyMessage) {
  protobuf_unittest::TestAllTypes empty;

  std::string data = empty.serialize();
  EXPECT_TRUE(data.empty());

  protobuf_unittest::TestAllTypes parsed;
  EXPECT_TRUE(parsed.parse(data));
  EXPECT_TRUE(parsed.isInitialized());
  EXPECT_EQ(parsed.getSerializedSize(), 0u);
}

}  // namespace
}  // namespace protobuf
}  // namespace google
