// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#ifndef GOOGLE_PROTOBUF_ZERO_COPY_ZERO_COPY_STRING_H__
#define GOOGLE_PROTOBUF_ZERO_COPY_ZERO_COPY_STRING_H__

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "google/protobuf/zero_copy/parse_buffer.h"
#include "google/protobuf/port_def.inc"

namespace google {
namespace protobuf {
namespace zero_copy {

// ZeroCopyString provides a string-like type that can either reference
// data in a ParseBuffer (zero-copy) or own its own string data.
//
// Key features:
// - Provides zero-copy access via view() when buffer is valid
// - Automatically materializes to owned string on demand
// - Thread-safe reference counting via shared_ptr to buffer
//
// Usage:
//   // Create from buffer reference
//   auto buffer = ParseBuffer::Create(data);
//   ZeroCopyString str(buffer, offset, length);
//
//   // Fast zero-copy access
//   absl::string_view sv = str.view();
//
//   // Materialize to owned string if needed
//   std::string owned = str.ToString();
//
class PROTOBUF_EXPORT ZeroCopyString {
 public:
  // Default constructor creates an empty string.
  ZeroCopyString() : offset_(0), size_(0) {}

  // Construct from a ParseBuffer reference.
  ZeroCopyString(std::shared_ptr<const ParseBuffer> buffer,
                 size_t offset, size_t size)
      : buffer_(std::move(buffer)), offset_(offset), size_(size) {}

  // Construct from an owned string (makes a copy).
  explicit ZeroCopyString(std::string value)
      : materialized_(std::move(value)), offset_(0), size_(0) {}

  // Construct from string_view (makes a copy).
  explicit ZeroCopyString(absl::string_view value)
      : materialized_(value), offset_(0), size_(0) {}

  // Copy constructor.
  ZeroCopyString(const ZeroCopyString& other) = default;

  // Move constructor.
  ZeroCopyString(ZeroCopyString&& other) noexcept = default;

  // Copy assignment.
  ZeroCopyString& operator=(const ZeroCopyString& other) = default;

  // Move assignment.
  ZeroCopyString& operator=(ZeroCopyString&& other) noexcept = default;

  // Get a string_view of the data (zero-copy if buffer is valid).
  absl::string_view view() const {
    if (!materialized_.empty()) {
      return materialized_;
    }
    if (buffer_ && buffer_->IsValid()) {
      return buffer_->Substr(offset_, size_);
    }
    return absl::string_view();
  }

  // Implicit conversion to string_view for convenience.
  operator absl::string_view() const { return view(); }  // NOLINT

  // Get the data as an owned string (materializes if necessary).
  std::string ToString() const {
    if (!materialized_.empty()) {
      return materialized_;
    }
    if (buffer_ && buffer_->IsValid()) {
      return std::string(buffer_->Substr(offset_, size_));
    }
    return std::string();
  }

  // Materialize the data into an owned string and return a reference.
  // This is useful when you need to modify the string or ensure it
  // outlives the buffer.
  const std::string& Materialize() {
    if (materialized_.empty() && buffer_ && buffer_->IsValid()) {
      materialized_ = std::string(buffer_->Substr(offset_, size_));
      // Clear buffer reference after materialization
      buffer_.reset();
    }
    return materialized_;
  }

  // Check if the buffer reference is still valid.
  bool IsValid() const {
    return !materialized_.empty() || (buffer_ && buffer_->IsValid());
  }

  // Check if data is zero-copy (not yet materialized).
  bool IsZeroCopy() const {
    return materialized_.empty() && buffer_ != nullptr;
  }

  // Check if data has been materialized.
  bool IsMaterialized() const {
    return !materialized_.empty();
  }

  // Returns the size of the string.
  size_t size() const {
    if (!materialized_.empty()) {
      return materialized_.size();
    }
    return size_;
  }

  // Returns true if the string is empty.
  bool empty() const {
    return size() == 0;
  }

  // Returns the data pointer.
  const char* data() const {
    return view().data();
  }

  // Comparison operators
  bool operator==(absl::string_view other) const {
    return view() == other;
  }

  bool operator!=(absl::string_view other) const {
    return view() != other;
  }

  bool operator<(absl::string_view other) const {
    return view() < other;
  }

  // Clear the string.
  void Clear() {
    buffer_.reset();
    materialized_.clear();
    offset_ = 0;
    size_ = 0;
  }

 private:
  std::shared_ptr<const ParseBuffer> buffer_;
  std::string materialized_;  // Lazily populated
  size_t offset_;
  size_t size_;
};

// ZeroCopyBytes provides similar functionality for bytes fields.
// It returns absl::Span<const uint8_t> instead of string_view.
class PROTOBUF_EXPORT ZeroCopyBytes {
 public:
  // Default constructor creates empty bytes.
  ZeroCopyBytes() : offset_(0), size_(0) {}

  // Construct from a ParseBuffer reference.
  ZeroCopyBytes(std::shared_ptr<const ParseBuffer> buffer,
                size_t offset, size_t size)
      : buffer_(std::move(buffer)), offset_(offset), size_(size) {}

  // Construct from owned data.
  explicit ZeroCopyBytes(std::string value)
      : materialized_(std::move(value)), offset_(0), size_(0) {}

  // Get a span of the data (zero-copy if buffer is valid).
  absl::Span<const uint8_t> span() const {
    if (!materialized_.empty()) {
      return absl::MakeConstSpan(
          reinterpret_cast<const uint8_t*>(materialized_.data()),
          materialized_.size());
    }
    if (buffer_ && buffer_->IsValid()) {
      return buffer_->BytesFrom(offset_, size_);
    }
    return absl::Span<const uint8_t>();
  }

  // Get the data as an owned string (materializes if necessary).
  std::string ToString() const {
    if (!materialized_.empty()) {
      return materialized_;
    }
    if (buffer_ && buffer_->IsValid()) {
      return std::string(buffer_->Substr(offset_, size_));
    }
    return std::string();
  }

  // Materialize and return reference.
  const std::string& Materialize() {
    if (materialized_.empty() && buffer_ && buffer_->IsValid()) {
      materialized_ = std::string(buffer_->Substr(offset_, size_));
      buffer_.reset();
    }
    return materialized_;
  }

  // Check if the buffer reference is still valid.
  bool IsValid() const {
    return !materialized_.empty() || (buffer_ && buffer_->IsValid());
  }

  // Returns the size.
  size_t size() const {
    if (!materialized_.empty()) {
      return materialized_.size();
    }
    return size_;
  }

  // Returns true if empty.
  bool empty() const {
    return size() == 0;
  }

  // Clear the bytes.
  void Clear() {
    buffer_.reset();
    materialized_.clear();
    offset_ = 0;
    size_ = 0;
  }

 private:
  std::shared_ptr<const ParseBuffer> buffer_;
  std::string materialized_;
  size_t offset_;
  size_t size_;
};

// LazyString combines ZeroCopyString with lazy materialization.
// It keeps the buffer reference and materializes only when str() is called.
class PROTOBUF_EXPORT LazyString {
 public:
  LazyString() : offset_(0), size_(0) {}

  LazyString(std::shared_ptr<const ParseBuffer> buffer,
             size_t offset, size_t size)
      : buffer_(std::move(buffer)), offset_(offset), size_(size) {}

  explicit LazyString(std::string value)
      : materialized_(std::move(value)), offset_(0), size_(0) {}

  // Fast path - return view if buffer valid.
  absl::string_view view() const {
    if (buffer_ && buffer_->IsValid()) {
      return buffer_->Substr(offset_, size_);
    }
    return materialized_;
  }

  // Materialize to owned string.
  const std::string& str() {
    if (materialized_.empty() && buffer_ && buffer_->IsValid()) {
      materialized_ = std::string(buffer_->Substr(offset_, size_));
    }
    return materialized_;
  }

  // Mutable access - forces materialization.
  std::string* mutable_str() {
    if (materialized_.empty() && buffer_ && buffer_->IsValid()) {
      materialized_ = std::string(buffer_->Substr(offset_, size_));
      buffer_.reset();
    }
    return &materialized_;
  }

  size_t size() const {
    if (!materialized_.empty()) {
      return materialized_.size();
    }
    return size_;
  }

  bool empty() const { return size() == 0; }

 private:
  std::shared_ptr<const ParseBuffer> buffer_;
  std::string materialized_;
  size_t offset_;
  size_t size_;
};

}  // namespace zero_copy
}  // namespace protobuf
}  // namespace google

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_ZERO_COPY_ZERO_COPY_STRING_H__
