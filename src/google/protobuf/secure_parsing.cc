// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/secure_parsing.h"

#include <cstddef>
#include <string>

#include "absl/strings/string_view.h"
#include "google/protobuf/io/coded_stream.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "google/protobuf/message_lite.h"
#include "google/protobuf/parse_options.h"
#include "google/protobuf/security_context.h"

namespace google {
namespace protobuf {

SecureParseResult SecureParseFromString(
    MessageLite* message,
    absl::string_view data,
    const ParseOptions& options) {
  SecureParseResult result;
  result.success = false;
  result.max_depth_reached = 0;
  result.field_count = 0;
  result.bytes_parsed = 0;

  // Create security context
  internal::SecurityContext context(options);

  // Log parse started event
  context.LogSecurityEvent(SecurityEventType::kParseStarted,
                           message->GetTypeName().data(), "",
                           data.size(), "");

  // Check message size limit before parsing
  if (!context.CheckMessageSize(data.size())) {
    result.error_message = context.last_error();
    context.LogSecurityEvent(SecurityEventType::kParseFailed,
                             message->GetTypeName().data(), "",
                             data.size(), result.error_message);
    return result;
  }

  // Check timeout before starting
  if (context.IsTimeoutExceeded()) {
    result.error_message = "Parse timeout exceeded before starting";
    context.LogSecurityEvent(SecurityEventType::kTimeoutExceeded,
                             message->GetTypeName().data(), "",
                             data.size(), result.error_message);
    return result;
  }

  // Track memory allocation
  if (!context.memory_tracker()->Allocate(data.size())) {
    result.error_message = "Message size exceeds memory limit";
    context.LogSecurityEvent(SecurityEventType::kMessageSizeLimitExceeded,
                             message->GetTypeName().data(), "",
                             data.size(), result.error_message);
    return result;
  }

  // Create input stream with limits
  io::ArrayInputStream array_stream(data.data(), static_cast<int>(data.size()));
  io::CodedInputStream input(&array_stream);

  // Apply recursion limit from options
  input.SetRecursionLimit(options.max_recursion_depth);

  // Apply total bytes limit
  if (options.max_message_size < static_cast<size_t>(INT_MAX)) {
    input.SetTotalBytesLimit(static_cast<int>(options.max_message_size));
  }

  // Clear the message before parsing
  message->Clear();

  // Perform the parse
  bool parse_success = message->MergePartialFromCodedStream(&input);

  // Check for various failure conditions
  if (!parse_success) {
    result.error_message = "Parse failed";
    context.LogSecurityEvent(SecurityEventType::kParseFailed,
                             message->GetTypeName().data(), "",
                             data.size(), result.error_message);
    return result;
  }

  // Verify entire message was consumed
  if (!input.ConsumedEntireMessage()) {
    result.error_message = "Failed to consume entire message";
    context.LogSecurityEvent(SecurityEventType::kParseFailed,
                             message->GetTypeName().data(), "",
                             data.size(), result.error_message);
    return result;
  }

  // Check timeout after parsing
  if (context.IsTimeoutExceeded()) {
    result.error_message = "Parse timeout exceeded";
    context.LogSecurityEvent(SecurityEventType::kTimeoutExceeded,
                             message->GetTypeName().data(), "",
                             data.size(), result.error_message);
    return result;
  }

  // Check if message is initialized (has all required fields)
  if (!message->IsInitialized()) {
    result.error_message = "Message is missing required fields";
    context.LogSecurityEvent(SecurityEventType::kParseFailed,
                             message->GetTypeName().data(), "",
                             data.size(), result.error_message);
    return result;
  }

  // Success
  result.success = true;
  result.bytes_parsed = data.size();
  result.max_depth_reached = context.current_depth();
  result.field_count = context.field_count();

  context.LogSecurityEvent(SecurityEventType::kParseCompleted,
                           message->GetTypeName().data(), "",
                           data.size(), "");

  return result;
}

SecureParseResult SecureParseFromArray(
    MessageLite* message,
    const void* data,
    int size,
    const ParseOptions& options) {
  return SecureParseFromString(
      message,
      absl::string_view(static_cast<const char*>(data), size),
      options);
}

SecureParseResult SecureMergeFromString(
    MessageLite* message,
    absl::string_view data,
    const ParseOptions& options) {
  SecureParseResult result;
  result.success = false;
  result.max_depth_reached = 0;
  result.field_count = 0;
  result.bytes_parsed = 0;

  // Create security context
  internal::SecurityContext context(options);

  // Log parse started event
  context.LogSecurityEvent(SecurityEventType::kParseStarted,
                           message->GetTypeName().data(), "",
                           data.size(), "Merge operation");

  // Check message size limit before parsing
  if (!context.CheckMessageSize(data.size())) {
    result.error_message = context.last_error();
    return result;
  }

  // Check timeout before starting
  if (context.IsTimeoutExceeded()) {
    result.error_message = "Parse timeout exceeded before starting";
    return result;
  }

  // Track memory allocation
  if (!context.memory_tracker()->Allocate(data.size())) {
    result.error_message = "Message size exceeds memory limit";
    return result;
  }

  // Create input stream with limits
  io::ArrayInputStream array_stream(data.data(), static_cast<int>(data.size()));
  io::CodedInputStream input(&array_stream);

  // Apply recursion limit from options
  input.SetRecursionLimit(options.max_recursion_depth);

  // Apply total bytes limit
  if (options.max_message_size < static_cast<size_t>(INT_MAX)) {
    input.SetTotalBytesLimit(static_cast<int>(options.max_message_size));
  }

  // Perform the merge (don't clear first)
  bool parse_success = message->MergePartialFromCodedStream(&input);

  if (!parse_success) {
    result.error_message = "Merge failed";
    return result;
  }

  // Verify entire message was consumed
  if (!input.ConsumedEntireMessage()) {
    result.error_message = "Failed to consume entire message during merge";
    return result;
  }

  // Check timeout after parsing
  if (context.IsTimeoutExceeded()) {
    result.error_message = "Parse timeout exceeded during merge";
    return result;
  }

  // Success
  result.success = true;
  result.bytes_parsed = data.size();
  result.max_depth_reached = context.current_depth();
  result.field_count = context.field_count();

  context.LogSecurityEvent(SecurityEventType::kParseCompleted,
                           message->GetTypeName().data(), "",
                           data.size(), "Merge completed");

  return result;
}

SecureParseResult SecureMergeFromArray(
    MessageLite* message,
    const void* data,
    int size,
    const ParseOptions& options) {
  return SecureMergeFromString(
      message,
      absl::string_view(static_cast<const char*>(data), size),
      options);
}

}  // namespace protobuf
}  // namespace google
