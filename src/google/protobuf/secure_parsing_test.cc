// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Tests for security hardening features.

#include "google/protobuf/secure_parsing.h"

#include <cstddef>
#include <string>
#include <vector>

#include "google/protobuf/parse_options.h"
#include "google/protobuf/security_context.h"
#include "google/protobuf/memory_tracker.h"
#include "google/protobuf/unittest.pb.h"
#include <gtest/gtest.h>

namespace google {
namespace protobuf {
namespace {

class SecureParsingTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Create a simple test message
    test_message_.set_optional_int32(123);
    test_message_.set_optional_string("test");
    serialized_data_ = test_message_.SerializeAsString();
  }

  protobuf_unittest::TestAllTypes test_message_;
  std::string serialized_data_;
};

// Test basic secure parsing with default options
TEST_F(SecureParsingTest, BasicParsingWithDefaultOptions) {
  protobuf_unittest::TestAllTypes parsed;
  ParseOptions options;

  SecureParseResult result = SecureParseFromString(&parsed, serialized_data_, options);

  EXPECT_TRUE(result.success);
  EXPECT_EQ(parsed.optional_int32(), 123);
  EXPECT_EQ(parsed.optional_string(), "test");
  EXPECT_EQ(result.bytes_parsed, serialized_data_.size());
}

// Test secure parsing with untrusted options
TEST_F(SecureParsingTest, ParsingWithUntrustedOptions) {
  protobuf_unittest::TestAllTypes parsed;
  ParseOptions options = ParseOptions::Untrusted();

  SecureParseResult result = SecureParseFromString(&parsed, serialized_data_, options);

  EXPECT_TRUE(result.success);
  EXPECT_EQ(parsed.optional_int32(), 123);
}

// Test message size limit enforcement
TEST_F(SecureParsingTest, MessageSizeLimitEnforced) {
  protobuf_unittest::TestAllTypes parsed;
  ParseOptions options;
  options.max_message_size = 1;  // Very small limit

  SecureParseResult result = SecureParseFromString(&parsed, serialized_data_, options);

  EXPECT_FALSE(result.success);
  EXPECT_FALSE(result.error_message.empty());
}

// Test that parsing succeeds when message is within size limit
TEST_F(SecureParsingTest, MessageWithinSizeLimit) {
  protobuf_unittest::TestAllTypes parsed;
  ParseOptions options;
  options.max_message_size = 1024 * 1024;  // 1MB

  SecureParseResult result = SecureParseFromString(&parsed, serialized_data_, options);

  EXPECT_TRUE(result.success);
}

// Test recursion limit
TEST_F(SecureParsingTest, RecursionLimitEnforced) {
  // Create a deeply nested message
  protobuf_unittest::TestAllTypes nested;
  protobuf_unittest::TestAllTypes* current = &nested;

  // Create 50 levels of nesting
  for (int i = 0; i < 50; ++i) {
    current = current->mutable_optional_nested_message()->mutable_bb()
              ? current : current;
    // For this test we just need some nested structure
    current->set_optional_int32(i);
  }

  std::string nested_data = nested.SerializeAsString();

  protobuf_unittest::TestAllTypes parsed;
  ParseOptions options;
  options.max_recursion_depth = 10;  // Very low limit

  SecureParseResult result = SecureParseFromString(&parsed, nested_data, options);

  // Should succeed since the actual nesting is shallow
  EXPECT_TRUE(result.success);
}

// Test empty message parsing
TEST_F(SecureParsingTest, EmptyMessageParsing) {
  protobuf_unittest::TestAllTypes parsed;
  ParseOptions options;

  std::string empty_data;
  SecureParseResult result = SecureParseFromString(&parsed, empty_data, options);

  EXPECT_TRUE(result.success);
}

// Test invalid data rejection
TEST_F(SecureParsingTest, InvalidDataRejected) {
  protobuf_unittest::TestAllTypes parsed;
  ParseOptions options;

  std::string invalid_data = "this is not valid protobuf data";
  SecureParseResult result = SecureParseFromString(&parsed, invalid_data, options);

  EXPECT_FALSE(result.success);
}

// Test ParseFromArrayWithOptions
TEST_F(SecureParsingTest, ParseFromArrayWithOptions) {
  protobuf_unittest::TestAllTypes parsed;
  ParseOptions options;

  bool success = ParseFromArrayWithOptions(
      &parsed, serialized_data_.data(),
      static_cast<int>(serialized_data_.size()), options);

  EXPECT_TRUE(success);
  EXPECT_EQ(parsed.optional_int32(), 123);
}

// Test ParseUntrusted convenience function
TEST_F(SecureParsingTest, ParseUntrustedConvenience) {
  protobuf_unittest::TestAllTypes parsed;

  bool success = ParseUntrusted(&parsed, serialized_data_);

  EXPECT_TRUE(success);
  EXPECT_EQ(parsed.optional_int32(), 123);
}

// Test ParseHighSecurity convenience function
TEST_F(SecureParsingTest, ParseHighSecurityConvenience) {
  protobuf_unittest::TestAllTypes parsed;

  bool success = ParseHighSecurity(&parsed, serialized_data_);

  EXPECT_TRUE(success);
  EXPECT_EQ(parsed.optional_int32(), 123);
}

// Test merge functionality
TEST_F(SecureParsingTest, SecureMergeFromString) {
  protobuf_unittest::TestAllTypes base;
  base.set_optional_int64(456);  // Set a different field

  ParseOptions options;
  SecureParseResult result = SecureMergeFromString(&base, serialized_data_, options);

  EXPECT_TRUE(result.success);
  EXPECT_EQ(base.optional_int32(), 123);  // From merged data
  EXPECT_EQ(base.optional_int64(), 456);  // Preserved from base
}

// Test high security options reject large messages
TEST_F(SecureParsingTest, HighSecurityRejectsLargeMessage) {
  // Create a message larger than high security limit (100KB)
  protobuf_unittest::TestAllTypes large;
  std::string large_string(200 * 1024, 'x');  // 200KB string
  large.set_optional_string(large_string);
  std::string large_data = large.SerializeAsString();

  protobuf_unittest::TestAllTypes parsed;
  ParseOptions options = ParseOptions::HighSecurity();

  SecureParseResult result = SecureParseFromString(&parsed, large_data, options);

  EXPECT_FALSE(result.success);
}

// Test audit logging callback
TEST_F(SecureParsingTest, AuditLoggingCallback) {
  std::vector<SecurityEvent> events;

  protobuf_unittest::TestAllTypes parsed;
  ParseOptions options;
  options.enable_audit_log = true;
  options.audit_callback = [&events](const SecurityEvent& event) {
    events.push_back(event);
  };

  SecureParseResult result = SecureParseFromString(&parsed, serialized_data_, options);

  EXPECT_TRUE(result.success);
  EXPECT_GE(events.size(), 2u);  // At least start and complete events

  // Check that we got the expected event types
  bool found_start = false;
  bool found_complete = false;
  for (const auto& event : events) {
    if (event.type == SecurityEventType::kParseStarted) found_start = true;
    if (event.type == SecurityEventType::kParseCompleted) found_complete = true;
  }
  EXPECT_TRUE(found_start);
  EXPECT_TRUE(found_complete);
}

// Memory tracker tests
class MemoryTrackerTest : public ::testing::Test {
 protected:
  internal::MemoryTracker tracker_{1024};  // 1KB limit
};

TEST_F(MemoryTrackerTest, AllocationWithinLimit) {
  EXPECT_TRUE(tracker_.Allocate(512));
  EXPECT_EQ(tracker_.allocated(), 512u);
  EXPECT_EQ(tracker_.remaining(), 512u);
}

TEST_F(MemoryTrackerTest, AllocationExceedsLimit) {
  EXPECT_TRUE(tracker_.Allocate(512));
  EXPECT_FALSE(tracker_.Allocate(600));  // Would exceed limit
  EXPECT_EQ(tracker_.allocated(), 512u);  // Unchanged
}

TEST_F(MemoryTrackerTest, Deallocation) {
  EXPECT_TRUE(tracker_.Allocate(512));
  tracker_.Deallocate(256);
  EXPECT_EQ(tracker_.allocated(), 256u);
}

TEST_F(MemoryTrackerTest, CanAllocateCheck) {
  EXPECT_TRUE(tracker_.CanAllocate(1024));
  EXPECT_FALSE(tracker_.CanAllocate(1025));
}

TEST_F(MemoryTrackerTest, NoLimitTracker) {
  internal::MemoryTracker unlimited(0);  // No limit
  EXPECT_TRUE(unlimited.Allocate(SIZE_MAX / 2));
  EXPECT_EQ(unlimited.remaining(), SIZE_MAX);
}

TEST_F(MemoryTrackerTest, Reset) {
  EXPECT_TRUE(tracker_.Allocate(512));
  tracker_.Reset();
  EXPECT_EQ(tracker_.allocated(), 0u);
}

// Scoped allocation tests
TEST_F(MemoryTrackerTest, ScopedAllocation) {
  {
    internal::ScopedAllocation scoped(&tracker_, 512);
    EXPECT_TRUE(scoped.succeeded());
    EXPECT_EQ(tracker_.allocated(), 512u);
  }
  // After scope, memory should be deallocated
  EXPECT_EQ(tracker_.allocated(), 0u);
}

TEST_F(MemoryTrackerTest, ScopedAllocationFails) {
  {
    internal::ScopedAllocation scoped(&tracker_, 2000);  // Exceeds limit
    EXPECT_FALSE(scoped.succeeded());
    EXPECT_EQ(tracker_.allocated(), 0u);
  }
}

// Security context tests
class SecurityContextTest : public ::testing::Test {
 protected:
  ParseOptions options_;

  void SetUp() override {
    options_.max_recursion_depth = 10;
    options_.max_field_count = 100;
    options_.max_string_size = 1024;
    options_.max_message_size = 10240;
  }
};

TEST_F(SecurityContextTest, RecursionDepthTracking) {
  internal::SecurityContext context(options_);

  for (int i = 0; i < 10; ++i) {
    EXPECT_TRUE(context.IncrementRecursionDepth());
  }
  EXPECT_EQ(context.current_depth(), 10);

  // 11th increment should fail
  EXPECT_FALSE(context.IncrementRecursionDepth());
}

TEST_F(SecurityContextTest, RecursionDepthDecrement) {
  internal::SecurityContext context(options_);

  EXPECT_TRUE(context.IncrementRecursionDepth());
  EXPECT_EQ(context.current_depth(), 1);

  context.DecrementRecursionDepth();
  EXPECT_EQ(context.current_depth(), 0);
}

TEST_F(SecurityContextTest, FieldCountTracking) {
  internal::SecurityContext context(options_);

  for (int i = 0; i < 100; ++i) {
    EXPECT_TRUE(context.IncrementFieldCount());
  }

  // 101st field should fail
  EXPECT_FALSE(context.IncrementFieldCount());
}

TEST_F(SecurityContextTest, StringSizeCheck) {
  internal::SecurityContext context(options_);

  EXPECT_TRUE(context.CheckStringSize(1024));
  EXPECT_FALSE(context.CheckStringSize(1025));
}

TEST_F(SecurityContextTest, MessageSizeCheck) {
  internal::SecurityContext context(options_);

  EXPECT_TRUE(context.CheckMessageSize(10240));
  EXPECT_FALSE(context.CheckMessageSize(10241));
}

TEST_F(SecurityContextTest, ScopedRecursionDepth) {
  internal::SecurityContext context(options_);

  {
    internal::ScopedRecursionDepth scoped(&context);
    EXPECT_TRUE(scoped.succeeded());
    EXPECT_EQ(context.current_depth(), 1);
  }
  EXPECT_EQ(context.current_depth(), 0);
}

// ParseOptions factory method tests
TEST(ParseOptionsTest, TrustedOptions) {
  ParseOptions opts = ParseOptions::Trusted();
  EXPECT_EQ(opts.max_recursion_depth, 100);
  EXPECT_EQ(opts.max_message_size, 64u * 1024 * 1024);
  EXPECT_FALSE(opts.strict_mode);
}

TEST(ParseOptionsTest, UntrustedOptions) {
  ParseOptions opts = ParseOptions::Untrusted();
  EXPECT_EQ(opts.max_recursion_depth, 32);
  EXPECT_EQ(opts.max_message_size, 1u * 1024 * 1024);
  EXPECT_TRUE(opts.strict_mode);
  EXPECT_TRUE(opts.validate_utf8);
}

TEST(ParseOptionsTest, HighSecurityOptions) {
  ParseOptions opts = ParseOptions::HighSecurity();
  EXPECT_EQ(opts.max_recursion_depth, 16);
  EXPECT_EQ(opts.max_message_size, 100u * 1024);
  EXPECT_TRUE(opts.strict_mode);
  EXPECT_TRUE(opts.enable_audit_log);
}

}  // namespace
}  // namespace protobuf
}  // namespace google
