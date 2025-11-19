// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// IncrementalParser: State machine for incremental/streaming message parsing.
// This allows parsing protocol buffer messages as bytes arrive incrementally,
// without requiring the entire message to be available in memory.

#ifndef GOOGLE_PROTOBUF_INCREMENTAL_PARSER_H__
#define GOOGLE_PROTOBUF_INCREMENTAL_PARSER_H__

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "google/protobuf/message_lite.h"
#include "google/protobuf/port.h"

// Must be included last.
#include "google/protobuf/port_def.inc"

namespace google {
namespace protobuf {

// IncrementalParser provides a state machine for parsing protocol buffer
// messages incrementally as bytes become available. This is useful for:
// - Network streaming where data arrives in chunks
// - Processing large files without loading them entirely into memory
// - Non-blocking I/O patterns
//
// Example usage:
//   IncrementalParser parser;
//   while (true) {
//     auto bytes = network.Read();
//     auto state = parser.Feed(bytes);
//     if (state == IncrementalParser::State::kMessageReady) {
//       auto msg = parser.TakeMessage<Person>();
//       ProcessMessage(msg);
//     }
//   }
class PROTOBUF_EXPORT IncrementalParser {
 public:
  // Parser state after feeding bytes
  enum class State {
    kNeedMoreData,   // Parser needs more bytes to complete message
    kMessageReady,   // A complete message is ready to be taken
    kError,          // An error occurred during parsing
    kDone            // Parsing is complete (no more messages expected)
  };

  // Configuration options for the parser
  struct Options {
    // Maximum message size allowed (default 64MB)
    size_t max_message_size = 64 * 1024 * 1024;

    // Whether to expect length-delimited messages
    bool length_delimited = true;

    // Initial buffer capacity
    size_t initial_buffer_capacity = 4096;
  };

  IncrementalParser();
  explicit IncrementalParser(const Options& options);
  ~IncrementalParser();

  // Non-copyable but movable
  IncrementalParser(const IncrementalParser&) = delete;
  IncrementalParser& operator=(const IncrementalParser&) = delete;
  IncrementalParser(IncrementalParser&&) noexcept;
  IncrementalParser& operator=(IncrementalParser&&) noexcept;

  // Feed bytes to the parser incrementally.
  // Returns the current state after processing the bytes.
  State Feed(absl::Span<const char> bytes);
  State Feed(absl::string_view bytes);

  // Get the current parser state
  State GetState() const { return state_; }

  // Returns true if a complete message is ready to be taken
  bool HasMessage() const { return state_ == State::kMessageReady; }

  // Take the parsed message. The message must be ready (HasMessage() == true).
  // After calling this, the parser state will reset for the next message.
  // Returns nullptr if no message is ready or if parsing failed.
  template <typename Message>
  std::unique_ptr<Message> TakeMessage();

  // Parse into an existing message object. Returns false if no message ready.
  template <typename Message>
  bool TakeMessage(Message* message);

  // Get the number of bytes consumed so far
  size_t BytesConsumed() const { return bytes_consumed_; }

  // Get the number of bytes remaining in the buffer
  size_t BytesPending() const { return buffer_.size() - buffer_pos_; }

  // Get the last error message (if state is kError)
  const std::string& GetErrorMessage() const { return error_message_; }

  // Reset the parser to initial state
  void Reset();

  // Mark parsing as done (no more messages expected)
  void Finish();

 private:
  // Try to parse a message from the current buffer
  bool TryParse();

  // Read a varint length prefix from the buffer
  bool ReadVarint(uint32_t* value);

  Options options_;
  State state_;
  std::vector<char> buffer_;
  size_t buffer_pos_;
  size_t bytes_consumed_;
  size_t message_size_;
  bool have_message_size_;
  std::string error_message_;
  std::string message_bytes_;
};

// Template implementations

template <typename Message>
std::unique_ptr<Message> IncrementalParser::TakeMessage() {
  if (state_ != State::kMessageReady) {
    return nullptr;
  }

  auto message = std::make_unique<Message>();
  if (!message->ParseFromString(message_bytes_)) {
    state_ = State::kError;
    error_message_ = "Failed to parse message";
    return nullptr;
  }

  message_bytes_.clear();
  have_message_size_ = false;

  // Check if there's more data to parse
  if (buffer_pos_ < buffer_.size()) {
    TryParse();
  } else {
    state_ = State::kNeedMoreData;
  }

  return message;
}

template <typename Message>
bool IncrementalParser::TakeMessage(Message* message) {
  if (state_ != State::kMessageReady || message == nullptr) {
    return false;
  }

  if (!message->ParseFromString(message_bytes_)) {
    state_ = State::kError;
    error_message_ = "Failed to parse message";
    return false;
  }

  message_bytes_.clear();
  have_message_size_ = false;

  // Check if there's more data to parse
  if (buffer_pos_ < buffer_.size()) {
    TryParse();
  } else {
    state_ = State::kNeedMoreData;
  }

  return true;
}

}  // namespace protobuf
}  // namespace google

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_INCREMENTAL_PARSER_H__
