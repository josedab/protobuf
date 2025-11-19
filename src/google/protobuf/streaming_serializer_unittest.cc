// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/streaming_serializer.h"

#include <future>
#include <string>
#include <vector>

#include "google/protobuf/unittest.pb.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include <gtest/gtest.h>

namespace google {
namespace protobuf {
namespace {

TEST(StreamingSerializerTest, SerializeStreaming) {
  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(42);
  message.set_optional_string("streaming");

  std::string output;
  io::StringOutputStream stream(&output);

  StreamingSerializer serializer;
  EXPECT_TRUE(serializer.SerializeStreaming(message, &stream));

  // Verify we can parse it back
  protobuf_unittest::TestAllTypes parsed;
  EXPECT_TRUE(parsed.ParseFromString(output));
  EXPECT_EQ(parsed.optional_int32(), 42);
  EXPECT_EQ(parsed.optional_string(), "streaming");
}

TEST(StreamingSerializerTest, SerializeAsync) {
  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(123);

  StreamingSerializer serializer;
  auto future = serializer.SerializeAsync(message);

  std::string data = future.get();
  EXPECT_FALSE(data.empty());

  protobuf_unittest::TestAllTypes parsed;
  EXPECT_TRUE(parsed.ParseFromString(data));
  EXPECT_EQ(parsed.optional_int32(), 123);
}

TEST(StreamingSerializerTest, SerializeChunks) {
  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(456);
  // Add some data to make multiple chunks
  message.set_optional_string(std::string(1000, 'x'));

  std::vector<std::string> chunks;
  StreamingSerializer serializer;
  EXPECT_TRUE(serializer.SerializeChunks(
      message,
      [&chunks](std::string chunk) { chunks.push_back(std::move(chunk)); },
      100));  // Small chunk size

  EXPECT_GT(chunks.size(), 1);

  // Concatenate chunks and parse
  std::string data;
  for (const auto& chunk : chunks) {
    data += chunk;
  }

  protobuf_unittest::TestAllTypes parsed;
  EXPECT_TRUE(parsed.ParseFromString(data));
  EXPECT_EQ(parsed.optional_int32(), 456);
}

TEST(StreamingSerializerTest, GetChunks) {
  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(789);
  message.set_optional_string(std::string(500, 'y'));

  StreamingSerializer serializer;
  auto chunks = serializer.GetChunks(message, 100);

  EXPECT_GT(chunks.size(), 1);

  // Concatenate and verify
  std::string data;
  for (const auto& chunk : chunks) {
    data += chunk;
  }

  protobuf_unittest::TestAllTypes parsed;
  EXPECT_TRUE(parsed.ParseFromString(data));
  EXPECT_EQ(parsed.optional_int32(), 789);
}

TEST(StreamingSerializerTest, SerializeWithLengthPrefix) {
  protobuf_unittest::TestAllTypes message;
  message.set_optional_int32(42);

  std::string output;
  io::StringOutputStream stream(&output);

  StreamingSerializer serializer;
  EXPECT_TRUE(serializer.SerializeWithLengthPrefix(message, &stream));

  // Parse the length prefix
  size_t pos = 0;
  uint32_t size = 0;
  int shift = 0;
  while (pos < output.size()) {
    uint8_t byte = static_cast<uint8_t>(output[pos++]);
    size |= (byte & 0x7F) << shift;
    if ((byte & 0x80) == 0) break;
    shift += 7;
  }

  // Parse the message
  std::string message_bytes = output.substr(pos, size);
  protobuf_unittest::TestAllTypes parsed;
  EXPECT_TRUE(parsed.ParseFromString(message_bytes));
  EXPECT_EQ(parsed.optional_int32(), 42);
}

TEST(StreamingSerializerTest, SerializeMultipleMessages) {
  std::vector<protobuf_unittest::TestAllTypes> messages;
  for (int i = 0; i < 5; ++i) {
    protobuf_unittest::TestAllTypes msg;
    msg.set_optional_int32(i * 10);
    messages.push_back(msg);
  }

  std::string output;
  io::StringOutputStream stream(&output);

  StreamingSerializer serializer;
  EXPECT_TRUE(
      serializer.SerializeMessages(messages.begin(), messages.end(), &stream));

  // Parse back all messages
  size_t pos = 0;
  for (int i = 0; i < 5; ++i) {
    // Read length prefix
    uint32_t size = 0;
    int shift = 0;
    while (pos < output.size()) {
      uint8_t byte = static_cast<uint8_t>(output[pos++]);
      size |= (byte & 0x7F) << shift;
      if ((byte & 0x80) == 0) break;
      shift += 7;
    }

    // Parse message
    std::string message_bytes = output.substr(pos, size);
    pos += size;

    protobuf_unittest::TestAllTypes parsed;
    EXPECT_TRUE(parsed.ParseFromString(message_bytes));
    EXPECT_EQ(parsed.optional_int32(), i * 10);
  }
}

TEST(StreamingSerializerTest, EmptyMessage) {
  protobuf_unittest::TestAllTypes message;  // Empty

  std::string output;
  io::StringOutputStream stream(&output);

  StreamingSerializer serializer;
  EXPECT_TRUE(serializer.SerializeStreaming(message, &stream));

  protobuf_unittest::TestAllTypes parsed;
  EXPECT_TRUE(parsed.ParseFromString(output));
}

}  // namespace
}  // namespace protobuf
}  // namespace google
