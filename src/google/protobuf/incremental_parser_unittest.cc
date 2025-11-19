// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/incremental_parser.h"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "google/protobuf/unittest.pb.h"
#include <gtest/gtest.h>
#include "absl/strings/string_view.h"

namespace google {
namespace protobuf {
namespace {

// Helper function to create a length-delimited message
std::string CreateDelimitedMessage(const MessageLite& message) {
  std::string serialized;
  message.SerializeToString(&serialized);

  // Create varint length prefix
  std::string result;
  uint32_t size = serialized.size();
  while (size >= 0x80) {
    result.push_back(static_cast<char>(size | 0x80));
    size >>= 7;
  }
  result.push_back(static_cast<char>(size));

  result.append(serialized);
  return result;
}

TEST(IncrementalParserTest, ParseSingleMessage) {
  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(42);
  message.set_optional_string("hello");

  std::string data = CreateDelimitedMessage(message);

  IncrementalParser parser;
  auto state = parser.Feed(data);

  EXPECT_EQ(state, IncrementalParser::State::kMessageReady);
  EXPECT_TRUE(parser.HasMessage());

  auto parsed = parser.TakeMessage<protobuf_unittest::TestAllTypes>();
  ASSERT_NE(parsed, nullptr);
  EXPECT_EQ(parsed->optional_int32(), 42);
  EXPECT_EQ(parsed->optional_string(), "hello");
}

TEST(IncrementalParserTest, ParseInChunks) {
  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(123);
  message.set_optional_string("chunked");

  std::string data = CreateDelimitedMessage(message);

  IncrementalParser parser;

  // Feed one byte at a time
  for (size_t i = 0; i < data.size() - 1; ++i) {
    auto state = parser.Feed(absl::string_view(data.data() + i, 1));
    EXPECT_EQ(state, IncrementalParser::State::kNeedMoreData);
    EXPECT_FALSE(parser.HasMessage());
  }

  // Feed the last byte
  auto state = parser.Feed(absl::string_view(data.data() + data.size() - 1, 1));
  EXPECT_EQ(state, IncrementalParser::State::kMessageReady);

  auto parsed = parser.TakeMessage<protobuf_unittest::TestAllTypes>();
  ASSERT_NE(parsed, nullptr);
  EXPECT_EQ(parsed->optional_int32(), 123);
  EXPECT_EQ(parsed->optional_string(), "chunked");
}

TEST(IncrementalParserTest, ParseMultipleMessages) {
  std::string data;

  // Create multiple messages
  for (int i = 0; i < 5; ++i) {
    protobuf_unittest::TestAllTypes message;
    message.set_optional_int32(i * 10);
    data += CreateDelimitedMessage(message);
  }

  IncrementalParser parser;

  // Feed all data at once
  parser.Feed(data);

  // Parse all messages
  for (int i = 0; i < 5; ++i) {
    EXPECT_TRUE(parser.HasMessage());
    auto parsed = parser.TakeMessage<protobuf_unittest::TestAllTypes>();
    ASSERT_NE(parsed, nullptr);
    EXPECT_EQ(parsed->optional_int32(), i * 10);
  }

  EXPECT_FALSE(parser.HasMessage());
}

TEST(IncrementalParserTest, ParseIntoExistingMessage) {
  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(999);

  std::string data = CreateDelimitedMessage(message);

  IncrementalParser parser;
  parser.Feed(data);

  protobuf_unittest::TestAllTypes parsed;
  EXPECT_TRUE(parser.TakeMessage(&parsed));
  EXPECT_EQ(parsed.optional_int32(), 999);
}

TEST(IncrementalParserTest, MaxMessageSizeEnforced) {
  IncrementalParser::Options options;
  options.max_message_size = 100;

  IncrementalParser parser(options);

  // Create a message larger than the limit
  protobuf_unittest::TestAllTypes message;
  std::string large_string(200, 'x');
  message.set_optional_string(large_string);

  std::string data = CreateDelimitedMessage(message);

  auto state = parser.Feed(data);
  EXPECT_EQ(state, IncrementalParser::State::kError);
  EXPECT_FALSE(parser.GetErrorMessage().empty());
}

TEST(IncrementalParserTest, Reset) {
  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(42);

  std::string data = CreateDelimitedMessage(message);

  IncrementalParser parser;

  // Parse partial data
  parser.Feed(absl::string_view(data.data(), data.size() / 2));

  // Reset
  parser.Reset();
  EXPECT_EQ(parser.GetState(), IncrementalParser::State::kNeedMoreData);
  EXPECT_EQ(parser.BytesConsumed(), 0);

  // Parse complete message
  parser.Feed(data);
  EXPECT_TRUE(parser.HasMessage());
}

TEST(IncrementalParserTest, EmptyFeed) {
  IncrementalParser parser;
  auto state = parser.Feed(absl::string_view());
  EXPECT_EQ(state, IncrementalParser::State::kNeedMoreData);
}

TEST(IncrementalParserTest, BytesConsumed) {
  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(42);

  std::string data = CreateDelimitedMessage(message);

  IncrementalParser parser;
  parser.Feed(data);

  // Bytes consumed should include the message content (not the varint prefix)
  EXPECT_GT(parser.BytesConsumed(), 0);
}

TEST(IncrementalParserTest, NonDelimitedMode) {
  IncrementalParser::Options options;
  options.length_delimited = false;

  IncrementalParser parser(options);

  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(42);

  std::string data;
  message.SerializeToString(&data);

  parser.Feed(data);
  EXPECT_TRUE(parser.HasMessage());
}

}  // namespace
}  // namespace protobuf
}  // namespace google
