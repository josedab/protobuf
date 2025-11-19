// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/streaming_serializer.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <future>
#include <string>
#include <vector>

#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/message_lite.h"

namespace google {
namespace protobuf {

bool StreamingSerializer::SerializeStreaming(
    const MessageLite& message, io::ZeroCopyOutputStream* output,
    size_t chunk_size) {
  // Serialize to string first (for simplicity)
  // A more optimized implementation would serialize directly to chunks
  std::string serialized;
  if (!message.SerializeToString(&serialized)) {
    return false;
  }

  // Write in chunks
  size_t offset = 0;
  while (offset < serialized.size()) {
    void* buffer;
    int buffer_size;
    if (!output->Next(&buffer, &buffer_size)) {
      return false;
    }

    size_t to_write = std::min(
        static_cast<size_t>(buffer_size),
        std::min(chunk_size, serialized.size() - offset));

    std::memcpy(buffer, serialized.data() + offset, to_write);
    offset += to_write;

    // Back up unused buffer space
    if (to_write < static_cast<size_t>(buffer_size)) {
      output->BackUp(buffer_size - static_cast<int>(to_write));
    }
  }

  return true;
}

std::future<std::string> StreamingSerializer::SerializeAsync(
    const MessageLite& message) {
  // Copy the message data for async processing
  std::string serialized;
  message.SerializeToString(&serialized);

  return std::async(std::launch::async,
                    [serialized = std::move(serialized)]() {
                      return serialized;
                    });
}

bool StreamingSerializer::SerializeChunks(
    const MessageLite& message,
    std::function<void(std::string)> chunk_callback,
    size_t chunk_size) {
  std::string serialized;
  if (!message.SerializeToString(&serialized)) {
    return false;
  }

  size_t offset = 0;
  while (offset < serialized.size()) {
    size_t to_write = std::min(chunk_size, serialized.size() - offset);
    chunk_callback(serialized.substr(offset, to_write));
    offset += to_write;
  }

  return true;
}

bool StreamingSerializer::SerializeWithLengthPrefix(
    const MessageLite& message, io::ZeroCopyOutputStream* output) {
  // Get the serialized size
  size_t size = message.ByteSizeLong();
  if (size > static_cast<size_t>(std::numeric_limits<uint32_t>::max())) {
    return false;
  }

  // Write the size as a varint
  if (!WriteVarint(output, static_cast<uint32_t>(size))) {
    return false;
  }

  // Serialize the message
  std::string serialized;
  if (!message.SerializeToString(&serialized)) {
    return false;
  }

  // Write the serialized data
  size_t offset = 0;
  while (offset < serialized.size()) {
    void* buffer;
    int buffer_size;
    if (!output->Next(&buffer, &buffer_size)) {
      return false;
    }

    size_t to_write = std::min(static_cast<size_t>(buffer_size),
                               serialized.size() - offset);
    std::memcpy(buffer, serialized.data() + offset, to_write);
    offset += to_write;

    if (to_write < static_cast<size_t>(buffer_size)) {
      output->BackUp(buffer_size - static_cast<int>(to_write));
    }
  }

  return true;
}

std::vector<std::string> StreamingSerializer::GetChunks(
    const MessageLite& message, size_t chunk_size) {
  std::vector<std::string> chunks;

  SerializeChunks(message,
                  [&chunks](std::string chunk) {
                    chunks.push_back(std::move(chunk));
                  },
                  chunk_size);

  return chunks;
}

bool StreamingSerializer::WriteVarint(io::ZeroCopyOutputStream* output,
                                      uint32_t value) {
  uint8_t buffer[5];  // Max 5 bytes for 32-bit varint
  int size = 0;

  while (value >= 0x80) {
    buffer[size++] = static_cast<uint8_t>(value | 0x80);
    value >>= 7;
  }
  buffer[size++] = static_cast<uint8_t>(value);

  // Write to output
  void* out_buffer;
  int out_size;
  if (!output->Next(&out_buffer, &out_size)) {
    return false;
  }

  if (out_size < size) {
    return false;  // Buffer too small
  }

  std::memcpy(out_buffer, buffer, size);

  if (out_size > size) {
    output->BackUp(out_size - size);
  }

  return true;
}

}  // namespace protobuf
}  // namespace google
