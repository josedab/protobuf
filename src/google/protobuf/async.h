// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Async/Streaming API for Protocol Buffers
//
// This header provides asynchronous and streaming interfaces for parsing
// and serializing protocol buffer messages. This enables:
// - Non-blocking parsing with futures
// - Streaming message processing
// - Memory-efficient handling of large messages
// - Integration with async frameworks

#ifndef GOOGLE_PROTOBUF_ASYNC_H__
#define GOOGLE_PROTOBUF_ASYNC_H__

#include <cstddef>
#include <functional>
#include <future>
#include <memory>
#include <utility>

#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/incremental_parser.h"
#include "google/protobuf/io/zero_copy_stream.h"
#include "google/protobuf/message_lite.h"
#include "google/protobuf/port.h"

// Must be included last.
#include "google/protobuf/port_def.inc"

namespace google {
namespace protobuf {

// Forward declarations
template <typename Message>
class MessageStream;

// AsyncParser provides asynchronous parsing capabilities for protocol buffers.
// It supports both future-based async parsing and incremental parsing.
//
// Example usage:
//   AsyncParser parser;
//   std::future<Person> future = parser.ParseAsync<Person>(input);
//   // Do other work...
//   Person person = future.get();
class PROTOBUF_EXPORT AsyncParser {
 public:
  AsyncParser() = default;
  ~AsyncParser() = default;

  // Non-copyable
  AsyncParser(const AsyncParser&) = delete;
  AsyncParser& operator=(const AsyncParser&) = delete;

  // Parse a message asynchronously using a future.
  // The parsing is done in a separate thread.
  template <typename Message>
  std::future<std::unique_ptr<Message>> ParseAsync(
      io::ZeroCopyInputStream* input);

  // Parse a message asynchronously from a string.
  template <typename Message>
  std::future<std::unique_ptr<Message>> ParseAsync(const std::string& data);

  // Parse incrementally with a state machine.
  // Returns the parsed message when complete, or an error status.
  template <typename Message>
  absl::StatusOr<std::unique_ptr<Message>> ParseIncremental(
      absl::string_view bytes, IncrementalParser* state);

  // Create a message stream for parsing multiple messages from a stream.
  template <typename Message>
  MessageStream<Message> ParseStream(io::ZeroCopyInputStream* input);
};

// MessageStream provides iteration over a stream of protocol buffer messages.
// It supports both blocking iteration and callback-based async iteration.
//
// Example usage:
//   MessageStream<LogEntry> stream = parser.ParseStream<LogEntry>(input);
//   while (stream.HasNext()) {
//     LogEntry entry = stream.Next().value();
//     ProcessEntry(entry);
//   }
template <typename Message>
class MessageStream {
 public:
  explicit MessageStream(io::ZeroCopyInputStream* input);
  ~MessageStream();

  // Move-only
  MessageStream(MessageStream&&) noexcept;
  MessageStream& operator=(MessageStream&&) noexcept;
  MessageStream(const MessageStream&) = delete;
  MessageStream& operator=(const MessageStream&) = delete;

  // Check if more messages are available.
  // This may block while reading from the input stream.
  bool HasNext();

  // Get the next message from the stream.
  // Returns an error status if parsing fails or stream is exhausted.
  absl::StatusOr<Message> Next();

  // Iterate over all messages with a callback.
  // The callback is invoked for each message until the stream is exhausted.
  void ForEach(std::function<void(Message)> callback);

  // Iterate with a callback that can stop iteration.
  // Return false from the callback to stop iteration.
  void ForEachWhile(std::function<bool(Message)> callback);

  // Get the number of messages successfully parsed so far.
  size_t MessageCount() const { return message_count_; }

  // Get the total bytes consumed from the input stream.
  size_t BytesConsumed() const { return bytes_consumed_; }

  // Check if an error occurred during parsing.
  bool HasError() const { return has_error_; }

  // Get the last error message.
  const std::string& GetErrorMessage() const { return error_message_; }

 private:
  bool ReadNextMessage();

  io::ZeroCopyInputStream* input_;
  IncrementalParser parser_;
  std::unique_ptr<Message> next_message_;
  size_t message_count_;
  size_t bytes_consumed_;
  bool has_next_;
  bool at_end_;
  bool has_error_;
  std::string error_message_;
};

// Template implementations for AsyncParser

template <typename Message>
std::future<std::unique_ptr<Message>> AsyncParser::ParseAsync(
    io::ZeroCopyInputStream* input) {
  return std::async(std::launch::async, [input]() {
    auto message = std::make_unique<Message>();

    std::string data;
    const void* buffer;
    int size;
    while (input->Next(&buffer, &size)) {
      data.append(static_cast<const char*>(buffer), size);
    }

    if (!message->ParseFromString(data)) {
      return std::unique_ptr<Message>(nullptr);
    }
    return message;
  });
}

template <typename Message>
std::future<std::unique_ptr<Message>> AsyncParser::ParseAsync(
    const std::string& data) {
  return std::async(std::launch::async, [data]() {
    auto message = std::make_unique<Message>();
    if (!message->ParseFromString(data)) {
      return std::unique_ptr<Message>(nullptr);
    }
    return message;
  });
}

template <typename Message>
absl::StatusOr<std::unique_ptr<Message>> AsyncParser::ParseIncremental(
    absl::string_view bytes, IncrementalParser* state) {
  state->Feed(bytes);

  if (state->GetState() == IncrementalParser::State::kError) {
    return absl::InvalidArgumentError(state->GetErrorMessage());
  }

  if (state->GetState() == IncrementalParser::State::kMessageReady) {
    return state->TakeMessage<Message>();
  }

  // Need more data
  return absl::UnavailableError("Need more data");
}

template <typename Message>
MessageStream<Message> AsyncParser::ParseStream(
    io::ZeroCopyInputStream* input) {
  return MessageStream<Message>(input);
}

// Template implementations for MessageStream

template <typename Message>
MessageStream<Message>::MessageStream(io::ZeroCopyInputStream* input)
    : input_(input),
      message_count_(0),
      bytes_consumed_(0),
      has_next_(false),
      at_end_(false),
      has_error_(false) {}

template <typename Message>
MessageStream<Message>::~MessageStream() = default;

template <typename Message>
MessageStream<Message>::MessageStream(MessageStream&&) noexcept = default;

template <typename Message>
MessageStream<Message>& MessageStream<Message>::operator=(
    MessageStream&&) noexcept = default;

template <typename Message>
bool MessageStream<Message>::HasNext() {
  if (has_next_) {
    return true;
  }
  if (at_end_ || has_error_) {
    return false;
  }
  return ReadNextMessage();
}

template <typename Message>
absl::StatusOr<Message> MessageStream<Message>::Next() {
  if (!HasNext()) {
    if (has_error_) {
      return absl::InvalidArgumentError(error_message_);
    }
    return absl::OutOfRangeError("No more messages");
  }

  Message result = std::move(*next_message_);
  next_message_.reset();
  has_next_ = false;
  message_count_++;
  return result;
}

template <typename Message>
void MessageStream<Message>::ForEach(std::function<void(Message)> callback) {
  while (HasNext()) {
    auto result = Next();
    if (result.ok()) {
      callback(std::move(result.value()));
    }
  }
}

template <typename Message>
void MessageStream<Message>::ForEachWhile(
    std::function<bool(Message)> callback) {
  while (HasNext()) {
    auto result = Next();
    if (result.ok()) {
      if (!callback(std::move(result.value()))) {
        break;
      }
    }
  }
}

template <typename Message>
bool MessageStream<Message>::ReadNextMessage() {
  const void* buffer;
  int size;

  while (input_->Next(&buffer, &size)) {
    if (size == 0) continue;

    auto state = parser_.Feed(
        absl::Span<const char>(static_cast<const char*>(buffer), size));
    bytes_consumed_ += size;

    if (state == IncrementalParser::State::kError) {
      has_error_ = true;
      error_message_ = parser_.GetErrorMessage();
      return false;
    }

    if (state == IncrementalParser::State::kMessageReady) {
      next_message_ = parser_.TakeMessage<Message>();
      if (next_message_) {
        has_next_ = true;
        return true;
      } else {
        has_error_ = true;
        error_message_ = "Failed to parse message";
        return false;
      }
    }
  }

  // End of stream
  at_end_ = true;
  return false;
}

}  // namespace protobuf
}  // namespace google

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_ASYNC_H__
