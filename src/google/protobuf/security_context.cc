// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/security_context.h"

#include <cstddef>
#include <string>

#include "absl/log/absl_log.h"
#include "absl/strings/string_view.h"
#include "absl/time/clock.h"
#include "absl/time/time.h"
#include "google/protobuf/parse_options.h"
#include "utf8_validity.h"

namespace google {
namespace protobuf {
namespace internal {

SecurityContext::SecurityContext(const ParseOptions& options)
    : options_(options),
      memory_tracker_(options.max_message_size),
      start_time_(absl::Now()),
      current_depth_(0),
      field_count_(0) {}

bool SecurityContext::IsTimeoutExceeded() const {
  if (options_.parse_timeout == absl::InfiniteDuration()) {
    return false;  // No timeout
  }
  return (absl::Now() - start_time_) > options_.parse_timeout;
}

bool SecurityContext::IncrementRecursionDepth() {
  ++current_depth_;
  if (current_depth_ > options_.max_recursion_depth) {
    last_error_ = "Maximum recursion depth exceeded";
    LogSecurityEvent(SecurityEventType::kRecursionLimitExceeded,
                     "", "", current_depth_, last_error_);
    return false;
  }
  return true;
}

void SecurityContext::DecrementRecursionDepth() {
  if (current_depth_ > 0) {
    --current_depth_;
  }
}

bool SecurityContext::IncrementFieldCount() {
  ++field_count_;
  if (field_count_ > options_.max_field_count) {
    last_error_ = "Maximum field count exceeded";
    LogSecurityEvent(SecurityEventType::kFieldCountLimitExceeded,
                     "", "", field_count_, last_error_);
    return false;
  }
  return true;
}

bool SecurityContext::CheckStringSize(size_t size) const {
  if (size > options_.max_string_size) {
    last_error_ = "String size exceeds maximum allowed";
    LogSecurityEvent(SecurityEventType::kStringSizeLimitExceeded,
                     "", "", size, last_error_);
    return false;
  }
  return true;
}

bool SecurityContext::CheckMessageSize(size_t size) const {
  if (size > options_.max_message_size) {
    last_error_ = "Message size exceeds maximum allowed";
    LogSecurityEvent(SecurityEventType::kMessageSizeLimitExceeded,
                     "", "", size, last_error_);
    return false;
  }
  return true;
}

void SecurityContext::LogSecurityEvent(SecurityEventType type,
                                        const std::string& message_type,
                                        const std::string& field_name,
                                        size_t size,
                                        const std::string& details) const {
  if (!options_.enable_audit_log) {
    return;
  }

  if (options_.audit_callback != nullptr) {
    SecurityEvent event;
    event.type = type;
    event.message_type = message_type;
    event.field_name = field_name;
    event.size = size;
    event.depth = current_depth_;
    event.details = details;
    options_.audit_callback(event);
  }

  // Also log to ABSL_LOG for debugging
  const char* event_name = "";
  switch (type) {
    case SecurityEventType::kParseStarted:
      event_name = "ParseStarted";
      break;
    case SecurityEventType::kParseCompleted:
      event_name = "ParseCompleted";
      break;
    case SecurityEventType::kParseFailed:
      event_name = "ParseFailed";
      break;
    case SecurityEventType::kRecursionLimitExceeded:
      event_name = "RecursionLimitExceeded";
      break;
    case SecurityEventType::kMessageSizeLimitExceeded:
      event_name = "MessageSizeLimitExceeded";
      break;
    case SecurityEventType::kFieldCountLimitExceeded:
      event_name = "FieldCountLimitExceeded";
      break;
    case SecurityEventType::kStringSizeLimitExceeded:
      event_name = "StringSizeLimitExceeded";
      break;
    case SecurityEventType::kTimeoutExceeded:
      event_name = "TimeoutExceeded";
      break;
    case SecurityEventType::kUnknownFieldRejected:
      event_name = "UnknownFieldRejected";
      break;
    case SecurityEventType::kInvalidUtf8Rejected:
      event_name = "InvalidUtf8Rejected";
      break;
    case SecurityEventType::kInvalidEnumRejected:
      event_name = "InvalidEnumRejected";
      break;
  }

  ABSL_LOG(INFO) << "Security event: " << event_name
                 << " message_type=" << message_type
                 << " field=" << field_name
                 << " size=" << size
                 << " depth=" << current_depth_
                 << " details=" << details;
}

bool SecurityContext::ValidateUtf8(absl::string_view str) const {
  if (!options_.strict_mode || !options_.validate_utf8) {
    return true;  // Validation disabled
  }

  if (!utf8_range::IsStructurallyValid(str)) {
    last_error_ = "Invalid UTF-8 encoding in string field";
    LogSecurityEvent(SecurityEventType::kInvalidUtf8Rejected,
                     "", "", str.size(), last_error_);
    return false;
  }
  return true;
}

}  // namespace internal
}  // namespace protobuf
}  // namespace google
