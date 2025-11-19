// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Fuzzing target for secure parsing.
// Tests parsing with restrictive security limits.

#include <cstddef>
#include <cstdint>
#include <string>

#include "google/protobuf/parse_options.h"
#include "google/protobuf/secure_parsing.h"
#include "google/protobuf/unittest.pb.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  // Configure restrictive limits for fuzzing
  google::protobuf::ParseOptions options;
  options.max_recursion_depth = 32;
  options.max_message_size = 1024 * 1024;  // 1MB
  options.max_field_count = 1000;
  options.max_string_size = 64 * 1024;  // 64KB
  options.strict_mode = false;  // Don't reject unknown fields during fuzzing

  // Try to parse the fuzzed input with security limits
  google::protobuf::protobuf_unittest::TestAllTypes message;
  google::protobuf::SecureParseFromArray(&message, data, size, options);

  // Also test with high security options
  google::protobuf::ParseOptions high_security =
      google::protobuf::ParseOptions::HighSecurity();
  high_security.enable_audit_log = false;  // Disable logging during fuzzing

  google::protobuf::protobuf_unittest::TestAllTypes message2;
  google::protobuf::SecureParseFromArray(&message2, data, size, high_security);

  // Also test with untrusted options
  google::protobuf::ParseOptions untrusted =
      google::protobuf::ParseOptions::Untrusted();

  google::protobuf::protobuf_unittest::TestAllTypes message3;
  google::protobuf::SecureParseFromArray(&message3, data, size, untrusted);

  return 0;
}
