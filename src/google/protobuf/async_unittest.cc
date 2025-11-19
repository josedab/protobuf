// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/async.h"

#include <future>
#include <memory>
#include <string>
#include <vector>

#include "google/protobuf/unittest.pb.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include <gtest/gtest.h>

namespace google {
namespace protobuf {
namespace {

// Helper function to create a length-delimited message
std::string CreateDelimitedMessage(const MessageLite& message) {
  std::string serialized;
  message.SerializeToString(&serialized);

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

TEST(AsyncParserTest, ParseAsyncFromString) {
  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(42);
  message.set_optional_string("async");

  std::string data;
  message.SerializeToString(&data);

  AsyncParser parser;
  auto future = parser.ParseAsync<protobuf_unittest::TestAllTypes>(data);

  auto parsed = future.get();
  ASSERT_NE(parsed, nullptr);
  EXPECT_EQ(parsed->optional_int32(), 42);
  EXPECT_EQ(parsed->optional_string(), "async");
}

TEST(AsyncParserTest, ParseAsyncFromStream) {
  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(123);

  std::string data;
  message.SerializeToString(&data);

  io::ArrayInputStream input(data.data(), data.size());

  AsyncParser parser;
  auto future = parser.ParseAsync<protobuf_unittest::TestAllTypes>(&input);

  auto parsed = future.get();
  ASSERT_NE(parsed, nullptr);
  EXPECT_EQ(parsed->optional_int32(), 123);
}

TEST(MessageStreamTest, StreamSingleMessage) {
  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(42);

  std::string data = CreateDelimitedMessage(message);
  io::ArrayInputStream input(data.data(), data.size());

  MessageStream<protobuf_unittest::TestAllTypes> stream(&input);

  EXPECT_TRUE(stream.HasNext());

  auto result = stream.Next();
  ASSERT_TRUE(result.ok());
  EXPECT_EQ(result.value().optional_int32(), 42);

  EXPECT_FALSE(stream.HasNext());
}

TEST(MessageStreamTest, StreamMultipleMessages) {
  std::string data;

  for (int i = 0; i < 10; ++i) {
    protobuf_unittest::TestAllTypes message;
    message.set_optional_int32(i);
    data += CreateDelimitedMessage(message);
  }

  io::ArrayInputStream input(data.data(), data.size());
  MessageStream<protobuf_unittest::TestAllTypes> stream(&input);

  int count = 0;
  stream.ForEach([&count](protobuf_unittest::TestAllTypes msg) {
    EXPECT_EQ(msg.optional_int32(), count);
    count++;
  });

  EXPECT_EQ(count, 10);
  EXPECT_EQ(stream.MessageCount(), 10);
}

TEST(MessageStreamTest, ForEachWhile) {
  std::string data;

  for (int i = 0; i < 10; ++i) {
    protobuf_unittest::TestAllTypes message;
    message.set_optional_int32(i);
    data += CreateDelimitedMessage(message);
  }

  io::ArrayInputStream input(data.data(), data.size());
  MessageStream<protobuf_unittest::TestAllTypes> stream(&input);

  int count = 0;
  stream.ForEachWhile([&count](protobuf_unittest::TestAllTypes msg) {
    count++;
    return count < 5;  // Stop after 5 messages
  });

  EXPECT_EQ(count, 5);
}

TEST(MessageStreamTest, EmptyStream) {
  std::string data;
  io::ArrayInputStream input(data.data(), data.size());
  MessageStream<protobuf_unittest::TestAllTypes> stream(&input);

  EXPECT_FALSE(stream.HasNext());
}

TEST(MessageStreamTest, BytesConsumed) {
  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(42);

  std::string data = CreateDelimitedMessage(message);
  io::ArrayInputStream input(data.data(), data.size());

  MessageStream<protobuf_unittest::TestAllTypes> stream(&input);

  while (stream.HasNext()) {
    stream.Next();
  }

  EXPECT_GT(stream.BytesConsumed(), 0);
}

}  // namespace
}  // namespace protobuf
}  // namespace google
