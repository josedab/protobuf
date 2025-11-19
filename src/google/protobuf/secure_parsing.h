// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Secure parsing utilities for Protocol Buffers.
// Provides security-hardened parsing with configurable limits.

#ifndef GOOGLE_PROTOBUF_SECURE_PARSING_H__
#define GOOGLE_PROTOBUF_SECURE_PARSING_H__

#include <cstddef>
#include <string>

#include "absl/strings/string_view.h"
#include "google/protobuf/message_lite.h"
#include "google/protobuf/parse_options.h"
#include "google/protobuf/security_context.h"

// Must be included last.
#include "google/protobuf/port_def.inc"

namespace google {
namespace protobuf {

// Parse result with detailed error information.
struct PROTOBUF_EXPORT SecureParseResult {
  bool success;
  std::string error_message;

  // Security metrics
  int max_depth_reached;
  int field_count;
  size_t bytes_parsed;

  // Implicit conversion to bool for convenience
  operator bool() const { return success; }
};

// Secure parsing functions with configurable security limits.
// These functions provide additional security guarantees beyond
// the standard parsing methods.

// Parse a protocol buffer from a string with security options.
// Returns detailed result including success/failure and metrics.
PROTOBUF_EXPORT SecureParseResult SecureParseFromString(
    MessageLite* message,
    absl::string_view data,
    const ParseOptions& options);

// Parse a protocol buffer from an array with security options.
PROTOBUF_EXPORT SecureParseResult SecureParseFromArray(
    MessageLite* message,
    const void* data,
    int size,
    const ParseOptions& options);

// Convenience overloads that return bool for compatibility.

// Parse from string with security options (simple bool return).
inline bool ParseFromStringWithOptions(
    MessageLite* message,
    absl::string_view data,
    const ParseOptions& options) {
  return SecureParseFromString(message, data, options).success;
}

// Parse from array with security options (simple bool return).
inline bool ParseFromArrayWithOptions(
    MessageLite* message,
    const void* data,
    int size,
    const ParseOptions& options) {
  return SecureParseFromArray(message, data, size, options).success;
}

// Parse with default untrusted options.
// Convenience function for parsing untrusted input with safe defaults.
inline bool ParseUntrusted(MessageLite* message, absl::string_view data) {
  return ParseFromStringWithOptions(message, data, ParseOptions::Untrusted());
}

// Parse with high security options.
// Uses most restrictive settings for maximum security.
inline bool ParseHighSecurity(MessageLite* message, absl::string_view data) {
  return ParseFromStringWithOptions(message, data, ParseOptions::HighSecurity());
}

// Merge functions with security options.

// Merge from string with security options.
PROTOBUF_EXPORT SecureParseResult SecureMergeFromString(
    MessageLite* message,
    absl::string_view data,
    const ParseOptions& options);

// Merge from array with security options.
PROTOBUF_EXPORT SecureParseResult SecureMergeFromArray(
    MessageLite* message,
    const void* data,
    int size,
    const ParseOptions& options);

}  // namespace protobuf
}  // namespace google

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_SECURE_PARSING_H__
