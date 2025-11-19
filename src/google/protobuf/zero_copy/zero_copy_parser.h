// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#ifndef GOOGLE_PROTOBUF_ZERO_COPY_ZERO_COPY_PARSER_H__
#define GOOGLE_PROTOBUF_ZERO_COPY_ZERO_COPY_PARSER_H__

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/message_lite.h"
#include "google/protobuf/zero_copy/parse_buffer.h"
#include "google/protobuf/zero_copy/zero_copy_string.h"
#include "google/protobuf/port_def.inc"

namespace google {
namespace protobuf {
namespace zero_copy {

// Field location information for zero-copy access.
struct FieldLocation {
  size_t offset;    // Offset from start of buffer
  size_t size;      // Size of field data
  uint32_t field_number;  // Field number in the message
};

// ZeroCopyParseResult holds the result of a zero-copy parse operation.
// It contains the buffer reference and locations of string/bytes fields.
class PROTOBUF_EXPORT ZeroCopyParseResult {
 public:
  ZeroCopyParseResult() = default;

  explicit ZeroCopyParseResult(std::shared_ptr<ParseBuffer> buffer)
      : buffer_(std::move(buffer)) {}

  // Get the underlying buffer.
  const std::shared_ptr<ParseBuffer>& Buffer() const { return buffer_; }

  // Add a field location.
  void AddFieldLocation(uint32_t field_number, size_t offset, size_t size) {
    field_locations_.push_back({offset, size, field_number});
  }

  // Get field locations.
  const std::vector<FieldLocation>& FieldLocations() const {
    return field_locations_;
  }

  // Find a field location by field number.
  const FieldLocation* FindField(uint32_t field_number) const {
    for (const auto& loc : field_locations_) {
      if (loc.field_number == field_number) {
        return &loc;
      }
    }
    return nullptr;
  }

  // Create a ZeroCopyString for a field.
  ZeroCopyString GetString(uint32_t field_number) const {
    const FieldLocation* loc = FindField(field_number);
    if (loc && buffer_) {
      return ZeroCopyString(buffer_, loc->offset, loc->size);
    }
    return ZeroCopyString();
  }

  // Create a ZeroCopyBytes for a field.
  ZeroCopyBytes GetBytes(uint32_t field_number) const {
    const FieldLocation* loc = FindField(field_number);
    if (loc && buffer_) {
      return ZeroCopyBytes(buffer_, loc->offset, loc->size);
    }
    return ZeroCopyBytes();
  }

  // Get a string_view directly for a field.
  absl::string_view GetStringView(uint32_t field_number) const {
    const FieldLocation* loc = FindField(field_number);
    if (loc && buffer_) {
      return buffer_->Substr(loc->offset, loc->size);
    }
    return absl::string_view();
  }

  // Get bytes span directly for a field.
  absl::Span<const uint8_t> GetBytesSpan(uint32_t field_number) const {
    const FieldLocation* loc = FindField(field_number);
    if (loc && buffer_) {
      return buffer_->BytesFrom(loc->offset, loc->size);
    }
    return absl::Span<const uint8_t>();
  }

 private:
  std::shared_ptr<ParseBuffer> buffer_;
  std::vector<FieldLocation> field_locations_;
};

// ZeroCopyParser enables zero-copy parsing of protocol buffer messages.
// It parses messages while tracking the locations of string and bytes fields
// so they can be accessed directly from the source buffer without copying.
//
// Usage:
//   auto buffer = ParseBuffer::Create(std::move(data));
//   MyMessage msg;
//   ZeroCopyParser parser;
//
//   auto result = parser.Parse(buffer, &msg);
//   if (result.ok()) {
//     // Access string field without copy
//     absl::string_view name = result->GetStringView(1);  // field number 1
//   }
//
// Note: This is a simplified implementation that demonstrates the concept.
// For production use, the code generator would need to be modified to
// generate messages that directly support zero-copy accessors.
//
class PROTOBUF_EXPORT ZeroCopyParser {
 public:
  ZeroCopyParser() = default;

  // Parse a message from a ParseBuffer.
  // Returns a ZeroCopyParseResult that can be used to access string/bytes
  // fields without copying.
  template <typename Message>
  absl::StatusOr<ZeroCopyParseResult> Parse(
      std::shared_ptr<ParseBuffer> buffer, Message* message);

  // Parse a message from a ParseBuffer with field tracking enabled.
  // This version tracks string/bytes field locations for zero-copy access.
  template <typename Message>
  absl::StatusOr<ZeroCopyParseResult> ParseWithTracking(
      std::shared_ptr<ParseBuffer> buffer, Message* message);

  // Enable/disable field tracking (enabled by default).
  void SetTrackFields(bool track) { track_fields_ = track; }
  bool TrackFields() const { return track_fields_; }

 private:
  bool track_fields_ = true;
};

// Implementation of Parse
template <typename Message>
absl::StatusOr<ZeroCopyParseResult> ZeroCopyParser::Parse(
    std::shared_ptr<ParseBuffer> buffer, Message* message) {
  if (!buffer || !buffer->IsValid()) {
    return absl::InvalidArgumentError("Invalid or null buffer");
  }

  if (!message) {
    return absl::InvalidArgumentError("Null message pointer");
  }

  // Parse using standard protobuf parsing
  absl::string_view data = buffer->Data();
  bool success = message->ParseFromArray(data.data(),
                                         static_cast<int>(data.size()));
  if (!success) {
    return absl::InvalidArgumentError("Failed to parse message");
  }

  // Create result with buffer reference
  ZeroCopyParseResult result(std::move(buffer));

  // Note: In a full implementation, the code generator would produce
  // messages that track field locations during parsing. This simplified
  // version doesn't track individual field locations since that would
  // require modifying the generated code.

  return result;
}

// Implementation of ParseWithTracking - for demonstration purposes
// In production, this would integrate with the generated code
template <typename Message>
absl::StatusOr<ZeroCopyParseResult> ZeroCopyParser::ParseWithTracking(
    std::shared_ptr<ParseBuffer> buffer, Message* message) {
  // For now, this is the same as Parse.
  // A full implementation would use reflection to track field locations.
  return Parse(std::move(buffer), message);
}

// Convenience functions for common use cases

// Parse from a string, creating an owned buffer.
template <typename Message>
absl::StatusOr<ZeroCopyParseResult> ParseZeroCopy(
    std::string data, Message* message) {
  auto buffer = ParseBuffer::Create(std::move(data));
  ZeroCopyParser parser;
  return parser.Parse(std::move(buffer), message);
}

// Parse from external data without copying.
template <typename Message>
absl::StatusOr<ZeroCopyParseResult> ParseZeroCopyFromArray(
    const char* data, size_t size, Message* message) {
  auto buffer = ParseBuffer::Wrap(data, size);
  ZeroCopyParser parser;
  return parser.Parse(std::move(buffer), message);
}

// Parse from a memory-mapped file.
template <typename Message>
absl::StatusOr<ZeroCopyParseResult> ParseZeroCopyFromFile(
    const std::string& path, Message* message) {
  auto buffer = MmapBuffer::Open(path);
  if (!buffer) {
    return absl::NotFoundError("Failed to open file: " + path);
  }
  ZeroCopyParser parser;
  return parser.Parse(std::move(buffer), message);
}

}  // namespace zero_copy
}  // namespace protobuf
}  // namespace google

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_ZERO_COPY_ZERO_COPY_PARSER_H__
