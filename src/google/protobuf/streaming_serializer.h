// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// StreamingSerializer: Serialize protocol buffer messages in chunks for
// streaming scenarios. This enables serializing large messages without
// holding the entire serialized output in memory.

#ifndef GOOGLE_PROTOBUF_STREAMING_SERIALIZER_H__
#define GOOGLE_PROTOBUF_STREAMING_SERIALIZER_H__

#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/message_lite.h"
#include "google/protobuf/port.h"

// Must be included last.
#include "google/protobuf/port_def.inc"

namespace google {
namespace protobuf {

// StreamingSerializer provides streaming serialization capabilities for
// protocol buffer messages. It supports:
// - Serializing messages in chunks for streaming I/O
// - Async serialization with futures
// - Callback-based chunk processing
//
// Example usage:
//   StreamingSerializer serializer;
//   serializer.SerializeChunks(large_message, [](std::string chunk) {
//     network.Send(chunk);
//   });
class PROTOBUF_EXPORT StreamingSerializer {
 public:
  // Default chunk size for streaming serialization (64KB)
  static constexpr size_t kDefaultChunkSize = 64 * 1024;

  StreamingSerializer() = default;
  ~StreamingSerializer() = default;

  // Non-copyable
  StreamingSerializer(const StreamingSerializer&) = delete;
  StreamingSerializer& operator=(const StreamingSerializer&) = delete;

  // Serialize a message to an output stream in streaming fashion.
  // The message is written in chunks to enable flow control and reduce
  // memory pressure.
  bool SerializeStreaming(const MessageLite& message,
                          io::ZeroCopyOutputStream* output,
                          size_t chunk_size = kDefaultChunkSize);

  // Serialize a message asynchronously using a future.
  // Returns a future that resolves to the serialized bytes.
  std::future<std::string> SerializeAsync(const MessageLite& message);

  // Serialize a message and invoke a callback for each chunk.
  // This enables streaming the serialized output as it becomes available.
  // The callback receives each chunk of serialized bytes.
  // Returns true if serialization was successful.
  bool SerializeChunks(const MessageLite& message,
                       std::function<void(std::string)> chunk_callback,
                       size_t chunk_size = kDefaultChunkSize);

  // Serialize a message with length prefix for streaming multiple messages.
  // The length prefix allows the receiver to know where one message ends
  // and the next begins.
  bool SerializeWithLengthPrefix(const MessageLite& message,
                                 io::ZeroCopyOutputStream* output);

  // Serialize multiple messages to a stream with length prefixes.
  template <typename Iterator>
  bool SerializeMessages(Iterator begin, Iterator end,
                         io::ZeroCopyOutputStream* output);

  // Get all serialized chunks as a vector.
  // This is less memory-efficient but convenient for testing.
  std::vector<std::string> GetChunks(const MessageLite& message,
                                     size_t chunk_size = kDefaultChunkSize);

 private:
  // Write a varint to the output stream
  bool WriteVarint(io::ZeroCopyOutputStream* output, uint32_t value);
};

// Template implementations

template <typename Iterator>
bool StreamingSerializer::SerializeMessages(
    Iterator begin, Iterator end, io::ZeroCopyOutputStream* output) {
  for (auto it = begin; it != end; ++it) {
    if (!SerializeWithLengthPrefix(*it, output)) {
      return false;
    }
  }
  return true;
}

}  // namespace protobuf
}  // namespace google

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_STREAMING_SERIALIZER_H__
