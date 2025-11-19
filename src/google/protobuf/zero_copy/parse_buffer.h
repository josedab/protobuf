// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#ifndef GOOGLE_PROTOBUF_ZERO_COPY_PARSE_BUFFER_H__
#define GOOGLE_PROTOBUF_ZERO_COPY_PARSE_BUFFER_H__

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include "absl/strings/string_view.h"
#include "google/protobuf/port_def.inc"

namespace google {
namespace protobuf {
namespace zero_copy {

// ParseBuffer is a shared buffer that maintains the lifetime of data for
// zero-copy parsing. It enables messages to reference data directly from
// the source buffer without copying.
//
// Usage:
//   auto buffer = ParseBuffer::Create(std::move(raw_data));
//   Message msg;
//   ZeroCopyParser parser;
//   parser.Parse(buffer, &msg);
//   // msg.field_view() returns views into buffer's data
//
class PROTOBUF_EXPORT ParseBuffer {
 public:
  virtual ~ParseBuffer() = default;

  // Create a ParseBuffer that owns the given data.
  static std::shared_ptr<ParseBuffer> Create(std::string data);

  // Create a ParseBuffer that wraps external data without taking ownership.
  // The caller must ensure the data remains valid for the lifetime of the
  // buffer and any messages parsed from it.
  static std::shared_ptr<ParseBuffer> Wrap(const char* data, size_t size);

  // Returns the data as a string_view.
  absl::string_view Data() const { return data_; }

  // Returns the size of the buffer.
  size_t Size() const { return data_.size(); }

  // Returns a pointer to the underlying data.
  const char* DataPtr() const { return data_.data(); }

  // Check if the buffer is still valid (not invalidated).
  bool IsValid() const { return valid_; }

  // Invalidate the buffer. After this, zero-copy accessors should
  // materialize their data. This is useful when the underlying data
  // is about to be freed or modified.
  void Invalidate() { valid_ = false; }

  // Returns a substring view of the buffer.
  absl::string_view Substr(size_t offset, size_t length) const {
    if (offset >= data_.size()) {
      return absl::string_view();
    }
    return data_.substr(offset, length);
  }

  // Returns the data as a span of bytes.
  absl::Span<const uint8_t> Bytes() const {
    return absl::MakeConstSpan(
        reinterpret_cast<const uint8_t*>(data_.data()), data_.size());
  }

  // Returns a span of bytes from offset.
  absl::Span<const uint8_t> BytesFrom(size_t offset, size_t length) const {
    if (offset >= data_.size()) {
      return absl::Span<const uint8_t>();
    }
    size_t actual_length = std::min(length, data_.size() - offset);
    return absl::MakeConstSpan(
        reinterpret_cast<const uint8_t*>(data_.data() + offset), actual_length);
  }

 protected:
  ParseBuffer() = default;

  // Disable copy/move to ensure proper sharing through shared_ptr.
  ParseBuffer(const ParseBuffer&) = delete;
  ParseBuffer& operator=(const ParseBuffer&) = delete;

  absl::string_view data_;
  bool valid_ = true;
};

// OwnedParseBuffer owns the underlying string data.
class PROTOBUF_EXPORT OwnedParseBuffer : public ParseBuffer {
 public:
  explicit OwnedParseBuffer(std::string data)
      : owned_data_(std::move(data)) {
    data_ = owned_data_;
  }

 private:
  std::string owned_data_;
};

// WrappedParseBuffer wraps external data without ownership.
class PROTOBUF_EXPORT WrappedParseBuffer : public ParseBuffer {
 public:
  WrappedParseBuffer(const char* data, size_t size) {
    data_ = absl::string_view(data, size);
  }
};

// MmapBuffer provides zero-copy access to memory-mapped file data.
// This allows parsing very large files without loading them entirely
// into memory.
//
// Usage:
//   auto buffer = MmapBuffer::Open("large_data.pb");
//   if (buffer) {
//     Message msg;
//     ZeroCopyParser parser;
//     parser.Parse(buffer, &msg);
//   }
//
class PROTOBUF_EXPORT MmapBuffer : public ParseBuffer {
 public:
  ~MmapBuffer() override;

  // Open a file and memory-map it.
  // Returns nullptr if the file cannot be opened or mapped.
  static std::shared_ptr<MmapBuffer> Open(const std::string& path);

  // Returns the file descriptor (for debugging/advanced use).
  int FileDescriptor() const { return fd_; }

  // Returns the total size of the mapped file.
  size_t MappedSize() const { return mapped_size_; }

 private:
  MmapBuffer() = default;

  int fd_ = -1;
  void* mapped_data_ = nullptr;
  size_t mapped_size_ = 0;
};

}  // namespace zero_copy
}  // namespace protobuf
}  // namespace google

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_ZERO_COPY_PARSE_BUFFER_H__
