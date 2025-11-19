// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Tests for validation utilities.

#include "google/protobuf/validation_util.h"

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "google/protobuf/testing/googletest.h"
#include <gtest/gtest.h>

namespace google {
namespace protobuf {
namespace validation {
namespace {

// Email validation tests
TEST(ValidationUtilTest, IsValidEmail) {
  // Valid emails
  EXPECT_TRUE(IsValidEmail("test@example.com"));
  EXPECT_TRUE(IsValidEmail("user.name@domain.co.uk"));
  EXPECT_TRUE(IsValidEmail("user+tag@example.org"));

  // Invalid emails
  EXPECT_FALSE(IsValidEmail(""));
  EXPECT_FALSE(IsValidEmail("invalid"));
  EXPECT_FALSE(IsValidEmail("@example.com"));
  EXPECT_FALSE(IsValidEmail("user@"));
  EXPECT_FALSE(IsValidEmail("user@domain"));
  EXPECT_FALSE(IsValidEmail("user@@example.com"));
}

// Hostname validation tests
TEST(ValidationUtilTest, IsValidHostname) {
  // Valid hostnames
  EXPECT_TRUE(IsValidHostname("example.com"));
  EXPECT_TRUE(IsValidHostname("sub.domain.example.com"));
  EXPECT_TRUE(IsValidHostname("localhost"));
  EXPECT_TRUE(IsValidHostname("host123"));

  // Invalid hostnames
  EXPECT_FALSE(IsValidHostname(""));
  EXPECT_FALSE(IsValidHostname("-invalid.com"));
  EXPECT_FALSE(IsValidHostname("invalid-.com"));
  EXPECT_FALSE(IsValidHostname(".invalid.com"));
  EXPECT_FALSE(IsValidHostname("invalid..com"));
}

// IPv4 validation tests
TEST(ValidationUtilTest, IsValidIPv4) {
  // Valid IPv4 addresses
  EXPECT_TRUE(IsValidIPv4("192.168.1.1"));
  EXPECT_TRUE(IsValidIPv4("0.0.0.0"));
  EXPECT_TRUE(IsValidIPv4("255.255.255.255"));
  EXPECT_TRUE(IsValidIPv4("10.0.0.1"));

  // Invalid IPv4 addresses
  EXPECT_FALSE(IsValidIPv4(""));
  EXPECT_FALSE(IsValidIPv4("192.168.1"));
  EXPECT_FALSE(IsValidIPv4("192.168.1.1.1"));
  EXPECT_FALSE(IsValidIPv4("256.1.1.1"));
  EXPECT_FALSE(IsValidIPv4("192.168.1.a"));
  EXPECT_FALSE(IsValidIPv4("192.168.1.-1"));
}

// IPv6 validation tests
TEST(ValidationUtilTest, IsValidIPv6) {
  // Valid IPv6 addresses
  EXPECT_TRUE(IsValidIPv6("2001:0db8:85a3:0000:0000:8a2e:0370:7334"));
  EXPECT_TRUE(IsValidIPv6("2001:db8:85a3::8a2e:370:7334"));
  EXPECT_TRUE(IsValidIPv6("::1"));
  EXPECT_TRUE(IsValidIPv6("::"));

  // Invalid IPv6 addresses
  EXPECT_FALSE(IsValidIPv6(""));
  EXPECT_FALSE(IsValidIPv6("2001:0db8:85a3:0000:0000:8a2e:0370:7334:extra"));
  EXPECT_FALSE(IsValidIPv6("2001:::db8"));
}

// IP validation tests (both v4 and v6)
TEST(ValidationUtilTest, IsValidIP) {
  // Valid IPs
  EXPECT_TRUE(IsValidIP("192.168.1.1"));
  EXPECT_TRUE(IsValidIP("::1"));

  // Invalid IPs
  EXPECT_FALSE(IsValidIP(""));
  EXPECT_FALSE(IsValidIP("invalid"));
}

// URI validation tests
TEST(ValidationUtilTest, IsValidURI) {
  // Valid URIs
  EXPECT_TRUE(IsValidURI("http://example.com"));
  EXPECT_TRUE(IsValidURI("https://example.com/path"));
  EXPECT_TRUE(IsValidURI("ftp://files.example.com"));
  EXPECT_TRUE(IsValidURI("custom-scheme://host"));

  // Invalid URIs
  EXPECT_FALSE(IsValidURI(""));
  EXPECT_FALSE(IsValidURI("example.com"));
  EXPECT_FALSE(IsValidURI("://example.com"));
  EXPECT_FALSE(IsValidURI("1http://example.com"));
}

// UUID validation tests
TEST(ValidationUtilTest, IsValidUUID) {
  // Valid UUIDs
  EXPECT_TRUE(IsValidUUID("550e8400-e29b-41d4-a716-446655440000"));
  EXPECT_TRUE(IsValidUUID("00000000-0000-0000-0000-000000000000"));
  EXPECT_TRUE(IsValidUUID("ffffffff-ffff-ffff-ffff-ffffffffffff"));

  // Invalid UUIDs
  EXPECT_FALSE(IsValidUUID(""));
  EXPECT_FALSE(IsValidUUID("550e8400-e29b-41d4-a716-44665544000"));  // Too short
  EXPECT_FALSE(IsValidUUID("550e8400-e29b-41d4-a716-4466554400000"));  // Too long
  EXPECT_FALSE(IsValidUUID("550e8400e29b41d4a716446655440000"));  // No dashes
  EXPECT_FALSE(IsValidUUID("550e8400-e29b-41d4-a716-44665544000g"));  // Invalid char
}

// Numeric range validation tests
TEST(ValidationUtilTest, ValidateNumericRange) {
  // Valid ranges
  EXPECT_TRUE(ValidateNumericRange(5, "field", true, 0, true, 10, false, 0, false, 0).ok());
  EXPECT_TRUE(ValidateNumericRange(0, "field", true, 0, false, 0, false, 0, false, 0).ok());
  EXPECT_TRUE(ValidateNumericRange(10, "field", false, 0, true, 10, false, 0, false, 0).ok());

  // Invalid ranges
  EXPECT_FALSE(ValidateNumericRange(-1, "field", true, 0, false, 0, false, 0, false, 0).ok());
  EXPECT_FALSE(ValidateNumericRange(11, "field", false, 0, true, 10, false, 0, false, 0).ok());
  EXPECT_FALSE(ValidateNumericRange(5, "field", false, 0, false, 0, true, 5, false, 0).ok());
  EXPECT_FALSE(ValidateNumericRange(5, "field", false, 0, false, 0, false, 0, true, 5).ok());
}

// String length validation tests
TEST(ValidationUtilTest, ValidateStringLength) {
  // Valid lengths
  EXPECT_TRUE(ValidateStringLength("test", "field", true, 1, true, 10).ok());
  EXPECT_TRUE(ValidateStringLength("", "field", true, 0, false, 0).ok());
  EXPECT_TRUE(ValidateStringLength("exactly10!", "field", true, 10, true, 10).ok());

  // Invalid lengths
  EXPECT_FALSE(ValidateStringLength("", "field", true, 1, false, 0).ok());
  EXPECT_FALSE(ValidateStringLength("toolong", "field", false, 0, true, 5).ok());
}

// Repeated count validation tests
TEST(ValidationUtilTest, ValidateRepeatedCount) {
  std::vector<int> items = {1, 2, 3};

  // Valid counts
  EXPECT_TRUE(ValidateRepeatedCount(items, "field", true, 1, true, 5).ok());
  EXPECT_TRUE(ValidateRepeatedCount(items, "field", true, 3, true, 3).ok());

  // Invalid counts
  EXPECT_FALSE(ValidateRepeatedCount(items, "field", true, 5, false, 0).ok());
  EXPECT_FALSE(ValidateRepeatedCount(items, "field", false, 0, true, 2).ok());
}

// Pattern validation tests
TEST(ValidationUtilTest, ValidatePattern) {
  // Valid patterns
  EXPECT_TRUE(ValidatePattern("abc123", "field", "^[a-z0-9]+$").ok());
  EXPECT_TRUE(ValidatePattern("test@example.com", "field", ".*@.*").ok());

  // Invalid patterns
  EXPECT_FALSE(ValidatePattern("ABC", "field", "^[a-z]+$").ok());
  EXPECT_FALSE(ValidatePattern("test", "field", "^[0-9]+$").ok());
}

// Contains/prefix/suffix validation tests
TEST(ValidationUtilTest, ValidateContains) {
  EXPECT_TRUE(ValidateContains("hello world", "field", "world").ok());
  EXPECT_FALSE(ValidateContains("hello", "field", "world").ok());
}

TEST(ValidationUtilTest, ValidatePrefix) {
  EXPECT_TRUE(ValidatePrefix("hello world", "field", "hello").ok());
  EXPECT_FALSE(ValidatePrefix("hello world", "field", "world").ok());
}

TEST(ValidationUtilTest, ValidateSuffix) {
  EXPECT_TRUE(ValidateSuffix("hello world", "field", "world").ok());
  EXPECT_FALSE(ValidateSuffix("hello world", "field", "hello").ok());
}

// In/not_in validation tests
TEST(ValidationUtilTest, ValidateIn) {
  std::vector<int> allowed = {1, 2, 3};
  EXPECT_TRUE(ValidateIn(1, "field", allowed).ok());
  EXPECT_TRUE(ValidateIn(2, "field", allowed).ok());
  EXPECT_FALSE(ValidateIn(4, "field", allowed).ok());
}

TEST(ValidationUtilTest, ValidateNotIn) {
  std::vector<int> disallowed = {0, -1};
  EXPECT_TRUE(ValidateNotIn(1, "field", disallowed).ok());
  EXPECT_FALSE(ValidateNotIn(0, "field", disallowed).ok());
  EXPECT_FALSE(ValidateNotIn(-1, "field", disallowed).ok());
}

}  // namespace
}  // namespace validation
}  // namespace protobuf
}  // namespace google
