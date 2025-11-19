// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#ifndef GOOGLE_PROTOBUF_VALIDATION_UTIL_H__
#define GOOGLE_PROTOBUF_VALIDATION_UTIL_H__

#include <regex>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/string_view.h"

namespace google {
namespace protobuf {

// Utility functions for validating Protocol Buffer field values.
// These functions are used by generated validation code.

namespace validation {

// String format validation functions

// Validates that a string is a valid email address.
// Uses a simplified pattern that covers most common email formats.
inline bool IsValidEmail(absl::string_view value) {
  if (value.empty()) return false;

  // Find @ symbol
  size_t at_pos = value.find('@');
  if (at_pos == absl::string_view::npos || at_pos == 0) return false;

  // Check for domain part
  absl::string_view domain = value.substr(at_pos + 1);
  if (domain.empty() || domain.find('.') == absl::string_view::npos) return false;

  return true;
}

// Validates that a string is a valid hostname (RFC 1123).
inline bool IsValidHostname(absl::string_view value) {
  if (value.empty() || value.size() > 253) return false;

  // Check for valid hostname characters
  for (char c : value) {
    if (!std::isalnum(static_cast<unsigned char>(c)) && c != '-' && c != '.') {
      return false;
    }
  }

  // Cannot start or end with hyphen or dot
  if (value.front() == '-' || value.front() == '.' ||
      value.back() == '-' || value.back() == '.') {
    return false;
  }

  return true;
}

// Validates that a string is a valid IPv4 address.
inline bool IsValidIPv4(absl::string_view value) {
  if (value.empty()) return false;

  int parts = 0;
  int current = 0;
  bool has_digit = false;

  for (size_t i = 0; i <= value.size(); ++i) {
    char c = (i < value.size()) ? value[i] : '.';

    if (c == '.') {
      if (!has_digit) return false;
      if (current > 255) return false;
      parts++;
      current = 0;
      has_digit = false;
    } else if (std::isdigit(static_cast<unsigned char>(c))) {
      current = current * 10 + (c - '0');
      has_digit = true;
    } else {
      return false;
    }
  }

  return parts == 4;
}

// Validates that a string is a valid IPv6 address.
inline bool IsValidIPv6(absl::string_view value) {
  if (value.empty()) return false;

  // Simplified IPv6 validation
  int colons = 0;
  int double_colon = 0;

  for (size_t i = 0; i < value.size(); ++i) {
    char c = value[i];
    if (c == ':') {
      colons++;
      if (i + 1 < value.size() && value[i + 1] == ':') {
        double_colon++;
        if (double_colon > 1) return false;
        i++;  // Skip next colon
        colons++;
      }
    } else if (!std::isxdigit(static_cast<unsigned char>(c))) {
      return false;
    }
  }

  // Valid IPv6 has 7 colons, or fewer with ::
  if (double_colon) {
    return colons <= 7;
  }
  return colons == 7;
}

// Validates that a string is a valid IP address (v4 or v6).
inline bool IsValidIP(absl::string_view value) {
  return IsValidIPv4(value) || IsValidIPv6(value);
}

// Validates that a string is a valid URI (RFC 3986).
inline bool IsValidURI(absl::string_view value) {
  if (value.empty()) return false;

  // Check for scheme (must start with letter)
  size_t scheme_end = value.find("://");
  if (scheme_end == absl::string_view::npos || scheme_end == 0) return false;

  // Check scheme characters
  for (size_t i = 0; i < scheme_end; ++i) {
    char c = value[i];
    if (i == 0) {
      if (!std::isalpha(static_cast<unsigned char>(c))) return false;
    } else {
      if (!std::isalnum(static_cast<unsigned char>(c)) && c != '+' && c != '-' && c != '.') {
        return false;
      }
    }
  }

  // Must have something after ://
  return value.size() > scheme_end + 3;
}

// Validates that a string is a valid UUID (RFC 4122).
inline bool IsValidUUID(absl::string_view value) {
  // UUID format: 8-4-4-4-12 (36 characters total)
  if (value.size() != 36) return false;

  for (size_t i = 0; i < value.size(); ++i) {
    char c = value[i];
    if (i == 8 || i == 13 || i == 18 || i == 23) {
      if (c != '-') return false;
    } else {
      if (!std::isxdigit(static_cast<unsigned char>(c))) return false;
    }
  }

  return true;
}

// Numeric range validation

template <typename T>
inline absl::Status ValidateNumericRange(T value, const char* field_name,
                                         bool has_gte, T gte,
                                         bool has_lte, T lte,
                                         bool has_gt, T gt,
                                         bool has_lt, T lt) {
  if (has_gte && value < gte) {
    return absl::InvalidArgumentError(
        absl::StrCat("field '", field_name, "' must be >= ", gte));
  }
  if (has_lte && value > lte) {
    return absl::InvalidArgumentError(
        absl::StrCat("field '", field_name, "' must be <= ", lte));
  }
  if (has_gt && value <= gt) {
    return absl::InvalidArgumentError(
        absl::StrCat("field '", field_name, "' must be > ", gt));
  }
  if (has_lt && value >= lt) {
    return absl::InvalidArgumentError(
        absl::StrCat("field '", field_name, "' must be < ", lt));
  }
  return absl::OkStatus();
}

// String length validation

inline absl::Status ValidateStringLength(absl::string_view value,
                                         const char* field_name,
                                         bool has_min_len, size_t min_len,
                                         bool has_max_len, size_t max_len) {
  if (has_min_len && value.size() < min_len) {
    return absl::InvalidArgumentError(
        absl::StrCat("field '", field_name, "' length must be >= ", min_len));
  }
  if (has_max_len && value.size() > max_len) {
    return absl::InvalidArgumentError(
        absl::StrCat("field '", field_name, "' length must be <= ", max_len));
  }
  return absl::OkStatus();
}

// Repeated field validation

template <typename Container>
inline absl::Status ValidateRepeatedCount(const Container& items,
                                          const char* field_name,
                                          bool has_min_items, size_t min_items,
                                          bool has_max_items, size_t max_items) {
  size_t count = items.size();
  if (has_min_items && count < min_items) {
    return absl::InvalidArgumentError(
        absl::StrCat("field '", field_name, "' must have at least ", min_items,
                     " items"));
  }
  if (has_max_items && count > max_items) {
    return absl::InvalidArgumentError(
        absl::StrCat("field '", field_name, "' must have at most ", max_items,
                     " items"));
  }
  return absl::OkStatus();
}

// Pattern matching validation

inline absl::Status ValidatePattern(absl::string_view value,
                                    const char* field_name,
                                    const std::string& pattern) {
  try {
    std::regex re(pattern);
    if (!std::regex_match(std::string(value), re)) {
      return absl::InvalidArgumentError(
          absl::StrCat("field '", field_name, "' must match pattern '",
                       pattern, "'"));
    }
  } catch (const std::regex_error& e) {
    return absl::InternalError(
        absl::StrCat("invalid regex pattern for field '", field_name, "': ",
                     e.what()));
  }
  return absl::OkStatus();
}

// Contains/prefix/suffix validation

inline absl::Status ValidateContains(absl::string_view value,
                                     const char* field_name,
                                     absl::string_view substring) {
  if (value.find(substring) == absl::string_view::npos) {
    return absl::InvalidArgumentError(
        absl::StrCat("field '", field_name, "' must contain '", substring, "'"));
  }
  return absl::OkStatus();
}

inline absl::Status ValidatePrefix(absl::string_view value,
                                   const char* field_name,
                                   absl::string_view prefix) {
  if (value.size() < prefix.size() ||
      value.substr(0, prefix.size()) != prefix) {
    return absl::InvalidArgumentError(
        absl::StrCat("field '", field_name, "' must have prefix '", prefix, "'"));
  }
  return absl::OkStatus();
}

inline absl::Status ValidateSuffix(absl::string_view value,
                                   const char* field_name,
                                   absl::string_view suffix) {
  if (value.size() < suffix.size() ||
      value.substr(value.size() - suffix.size()) != suffix) {
    return absl::InvalidArgumentError(
        absl::StrCat("field '", field_name, "' must have suffix '", suffix, "'"));
  }
  return absl::OkStatus();
}

// In/not-in validation for values

template <typename T>
inline absl::Status ValidateIn(T value, const char* field_name,
                               const std::vector<T>& allowed) {
  for (const auto& v : allowed) {
    if (value == v) return absl::OkStatus();
  }
  return absl::InvalidArgumentError(
      absl::StrCat("field '", field_name, "' must be one of the allowed values"));
}

template <typename T>
inline absl::Status ValidateNotIn(T value, const char* field_name,
                                  const std::vector<T>& disallowed) {
  for (const auto& v : disallowed) {
    if (value == v) {
      return absl::InvalidArgumentError(
          absl::StrCat("field '", field_name,
                       "' must not be one of the disallowed values"));
    }
  }
  return absl::OkStatus();
}

}  // namespace validation
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_VALIDATION_UTIL_H__
