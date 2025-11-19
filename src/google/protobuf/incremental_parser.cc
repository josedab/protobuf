// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/incremental_parser.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <utility>

#include "absl/strings/string_view.h"
#include "absl/types/span.h"

namespace google {
namespace protobuf {

IncrementalParser::IncrementalParser()
    : IncrementalParser(Options{}) {}

IncrementalParser::IncrementalParser(const Options& options)
    : options_(options),
      state_(State::kNeedMoreData),
      buffer_pos_(0),
      bytes_consumed_(0),
      message_size_(0),
      have_message_size_(false) {
  buffer_.reserve(options_.initial_buffer_capacity);
}

IncrementalParser::~IncrementalParser() = default;

IncrementalParser::IncrementalParser(IncrementalParser&&) noexcept = default;
IncrementalParser& IncrementalParser::operator=(IncrementalParser&&) noexcept = default;

IncrementalParser::State IncrementalParser::Feed(absl::Span<const char> bytes) {
  if (state_ == State::kError || state_ == State::kDone) {
    return state_;
  }

  // Append bytes to buffer
  if (!bytes.empty()) {
    buffer_.insert(buffer_.end(), bytes.begin(), bytes.end());
  }

  // Try to parse a message
  TryParse();

  return state_;
}

IncrementalParser::State IncrementalParser::Feed(absl::string_view bytes) {
  return Feed(absl::Span<const char>(bytes.data(), bytes.size()));
}

bool IncrementalParser::TryParse() {
  if (options_.length_delimited) {
    // Parse length-delimited messages
    if (!have_message_size_) {
      // Try to read the message size
      uint32_t size;
      if (!ReadVarint(&size)) {
        state_ = State::kNeedMoreData;
        return false;
      }

      if (size > options_.max_message_size) {
        state_ = State::kError;
        error_message_ = "Message size exceeds maximum allowed size";
        return false;
      }

      message_size_ = size;
      have_message_size_ = true;
    }

    // Check if we have enough bytes for the message
    size_t available = buffer_.size() - buffer_pos_;
    if (available < message_size_) {
      state_ = State::kNeedMoreData;
      return false;
    }

    // Extract the message bytes
    message_bytes_.assign(buffer_.data() + buffer_pos_, message_size_);
    buffer_pos_ += message_size_;
    bytes_consumed_ += message_size_;

    // Compact buffer if we've consumed a lot
    if (buffer_pos_ > buffer_.size() / 2 && buffer_pos_ > 4096) {
      buffer_.erase(buffer_.begin(), buffer_.begin() + buffer_pos_);
      buffer_pos_ = 0;
    }

    state_ = State::kMessageReady;
    return true;
  } else {
    // Non-length-delimited: parse entire buffer as a single message
    if (buffer_.empty()) {
      state_ = State::kNeedMoreData;
      return false;
    }

    message_bytes_.assign(buffer_.begin() + buffer_pos_, buffer_.end());
    bytes_consumed_ += message_bytes_.size();
    buffer_pos_ = buffer_.size();
    state_ = State::kMessageReady;
    return true;
  }
}

bool IncrementalParser::ReadVarint(uint32_t* value) {
  size_t pos = buffer_pos_;
  uint32_t result = 0;
  int shift = 0;

  while (pos < buffer_.size()) {
    uint8_t byte = static_cast<uint8_t>(buffer_[pos]);
    result |= static_cast<uint32_t>(byte & 0x7F) << shift;
    pos++;

    if ((byte & 0x80) == 0) {
      // Varint complete
      bytes_consumed_ += (pos - buffer_pos_);
      buffer_pos_ = pos;
      *value = result;
      return true;
    }

    shift += 7;
    if (shift >= 35) {
      // Varint too long
      state_ = State::kError;
      error_message_ = "Varint is too long";
      return false;
    }
  }

  // Need more data
  return false;
}

void IncrementalParser::Reset() {
  state_ = State::kNeedMoreData;
  buffer_.clear();
  buffer_pos_ = 0;
  bytes_consumed_ = 0;
  message_size_ = 0;
  have_message_size_ = false;
  error_message_.clear();
  message_bytes_.clear();
}

void IncrementalParser::Finish() {
  if (state_ == State::kNeedMoreData && buffer_pos_ >= buffer_.size()) {
    state_ = State::kDone;
  }
}

}  // namespace protobuf
}  // namespace google
