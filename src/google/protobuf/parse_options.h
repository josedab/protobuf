// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Security-focused parse options for Protocol Buffers.
// These options allow configuring resource limits and validation
// when parsing untrusted input.

#ifndef GOOGLE_PROTOBUF_PARSE_OPTIONS_H__
#define GOOGLE_PROTOBUF_PARSE_OPTIONS_H__

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

#include "absl/time/time.h"

// Must be included last.
#include "google/protobuf/port_def.inc"

namespace google {
namespace protobuf {

// Security event types for audit logging
enum class SecurityEventType {
  kParseStarted,
  kParseCompleted,
  kParseFailed,
  kRecursionLimitExceeded,
  kMessageSizeLimitExceeded,
  kFieldCountLimitExceeded,
  kStringSizeLimitExceeded,
  kTimeoutExceeded,
  kUnknownFieldRejected,
  kInvalidUtf8Rejected,
  kInvalidEnumRejected,
};

// Information about a security event for audit logging
struct SecurityEvent {
  SecurityEventType type;
  std::string message_type;
  std::string field_name;
  size_t size;
  int depth;
  std::string details;
};

// Callback type for security audit logging
using SecurityAuditCallback = std::function<void(const SecurityEvent&)>;

// Options for controlling parse behavior with security limits.
// These options allow configuring resource limits and validation
// when parsing untrusted input.
struct PROTOBUF_EXPORT ParseOptions {
  // Maximum recursion depth for nested messages (default: 100)
  // Set lower values when parsing untrusted input to prevent stack overflow.
  int max_recursion_depth = 100;

  // Maximum message size in bytes (default: 64MB)
  // Limits total memory allocation during parsing.
  size_t max_message_size = 64 * 1024 * 1024;

  // Maximum number of fields in a message (default: 10000)
  // Prevents DoS attacks using messages with excessive fields.
  int max_field_count = 10000;

  // Maximum string/bytes field size (default: 2GB)
  // Limits individual string or bytes field allocations.
  size_t max_string_size = 2ULL * 1024 * 1024 * 1024;

  // Timeout for parsing (default: infinite)
  // Prevents slow parsing attacks. Set to finite duration for untrusted input.
  absl::Duration parse_timeout = absl::InfiniteDuration();

  // Strict mode - reject unknown fields (default: false)
  // When true, parsing fails if unknown fields are encountered.
  bool strict_mode = false;

  // Validate UTF-8 encoding in string fields (default: false)
  // When true with strict_mode, invalid UTF-8 causes parse failure.
  bool validate_utf8 = false;

  // Enable security audit logging (default: false)
  // When true, security events are logged via the audit callback.
  bool enable_audit_log = false;

  // Callback for security audit events (default: nullptr)
  // Set this along with enable_audit_log to receive security events.
  SecurityAuditCallback audit_callback = nullptr;

  // Factory methods for common security configurations

  // Returns ParseOptions suitable for parsing trusted input.
  // Uses default limits - same as current protobuf behavior.
  static ParseOptions Trusted() {
    return ParseOptions();
  }

  // Returns ParseOptions suitable for parsing untrusted input.
  // Uses restrictive limits to prevent resource exhaustion.
  static ParseOptions Untrusted() {
    ParseOptions opts;
    opts.max_recursion_depth = 32;
    opts.max_message_size = 1 * 1024 * 1024;  // 1MB
    opts.max_field_count = 1000;
    opts.max_string_size = 100 * 1024;  // 100KB
    opts.strict_mode = true;
    opts.validate_utf8 = true;
    return opts;
  }

  // Returns ParseOptions with maximum security.
  // Uses very restrictive limits and enables all validations.
  static ParseOptions HighSecurity() {
    ParseOptions opts;
    opts.max_recursion_depth = 16;
    opts.max_message_size = 100 * 1024;  // 100KB
    opts.max_field_count = 100;
    opts.max_string_size = 10 * 1024;  // 10KB
    opts.strict_mode = true;
    opts.validate_utf8 = true;
    opts.enable_audit_log = true;
    opts.parse_timeout = absl::Seconds(1);
    return opts;
  }
};

}  // namespace protobuf
}  // namespace google

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_PARSE_OPTIONS_H__
