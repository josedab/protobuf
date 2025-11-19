// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Security context for Protocol Buffers parsing.
// Provides security-focused parsing with configurable limits and validation.

#ifndef GOOGLE_PROTOBUF_SECURITY_CONTEXT_H__
#define GOOGLE_PROTOBUF_SECURITY_CONTEXT_H__

#include <cstddef>
#include <cstdint>
#include <string>

#include "absl/strings/string_view.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "google/protobuf/memory_tracker.h"
#include "google/protobuf/parse_options.h"

// Must be included last.
#include "google/protobuf/port_def.inc"

namespace google {
namespace protobuf {

class MessageLite;

namespace internal {

// Security context for tracking limits and validation during parsing.
// This class maintains state during a parse operation and enforces
// the security limits specified in ParseOptions.
class PROTOBUF_EXPORT SecurityContext {
 public:
  // Create a security context with the specified options.
  explicit SecurityContext(const ParseOptions& options);

  // Check if parsing has exceeded its timeout.
  bool IsTimeoutExceeded() const;

  // Check and track recursion depth.
  // Returns true if the current depth is within limits.
  bool IncrementRecursionDepth();
  void DecrementRecursionDepth();
  int current_depth() const { return current_depth_; }

  // Check and track field count.
  // Returns true if the field count is within limits.
  bool IncrementFieldCount();
  int field_count() const { return field_count_; }

  // Check if a string size is within limits.
  bool CheckStringSize(size_t size) const;

  // Check if a message size is within limits.
  bool CheckMessageSize(size_t size) const;

  // Get the memory tracker.
  MemoryTracker* memory_tracker() { return &memory_tracker_; }

  // Get the options.
  const ParseOptions& options() const { return options_; }

  // Logging security events
  void LogSecurityEvent(SecurityEventType type,
                        const std::string& message_type = "",
                        const std::string& field_name = "",
                        size_t size = 0,
                        const std::string& details = "") const;

  // Validation methods for strict mode

  // Validate UTF-8 encoding of a string.
  // Returns true if valid or if strict mode is disabled.
  bool ValidateUtf8(absl::string_view str) const;

  // Check if unknown fields should be rejected.
  bool ShouldRejectUnknownFields() const { return options_.strict_mode; }

  // Get the last error message.
  const std::string& last_error() const { return last_error_; }

  // Set error message.
  void SetError(const std::string& error) { last_error_ = error; }

 private:
  const ParseOptions& options_;
  MemoryTracker memory_tracker_;
  absl::Time start_time_;
  int current_depth_;
  int field_count_;
  mutable std::string last_error_;
};

// RAII helper for tracking recursion depth in a scope.
class PROTOBUF_EXPORT ScopedRecursionDepth {
 public:
  explicit ScopedRecursionDepth(SecurityContext* context)
      : context_(context), succeeded_(false) {
    if (context_ != nullptr) {
      succeeded_ = context_->IncrementRecursionDepth();
    } else {
      succeeded_ = true;
    }
  }

  ~ScopedRecursionDepth() {
    if (context_ != nullptr && succeeded_) {
      context_->DecrementRecursionDepth();
    }
  }

  bool succeeded() const { return succeeded_; }

  // Prevent copying
  ScopedRecursionDepth(const ScopedRecursionDepth&) = delete;
  ScopedRecursionDepth& operator=(const ScopedRecursionDepth&) = delete;

 private:
  SecurityContext* context_;
  bool succeeded_;
};

}  // namespace internal
}  // namespace protobuf
}  // namespace google

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_SECURITY_CONTEXT_H__
