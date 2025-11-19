// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/util/decimal_util.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"

namespace google {
namespace protobuf {
namespace util {

namespace {

// Helper to convert coefficient bytes to a vector of digits (little-endian).
std::vector<uint8_t> BytesToDigits(const std::string& bytes, bool* is_negative) {
  std::vector<uint8_t> result;
  *is_negative = false;

  if (bytes.empty()) {
    result.push_back(0);
    return result;
  }

  // Check sign (two's complement)
  *is_negative = (static_cast<uint8_t>(bytes[0]) & 0x80) != 0;

  // Convert from big-endian bytes to value
  std::vector<uint8_t> value;
  for (size_t i = 0; i < bytes.size(); ++i) {
    value.push_back(static_cast<uint8_t>(bytes[i]));
  }

  // If negative, compute two's complement
  if (*is_negative) {
    bool carry = true;
    for (int i = value.size() - 1; i >= 0; --i) {
      value[i] = ~value[i];
      if (carry) {
        if (value[i] == 255) {
          value[i] = 0;
        } else {
          value[i]++;
          carry = false;
        }
      }
    }
  }

  // Convert bytes to base-10 digits
  while (!value.empty()) {
    uint32_t remainder = 0;
    std::vector<uint8_t> quotient;

    for (size_t i = 0; i < value.size(); ++i) {
      uint32_t dividend = (remainder << 8) | value[i];
      uint8_t q = dividend / 10;
      remainder = dividend % 10;
      if (!quotient.empty() || q != 0) {
        quotient.push_back(q);
      }
    }

    result.push_back(remainder);
    value = quotient;
  }

  if (result.empty()) {
    result.push_back(0);
  }

  return result;
}

// Helper to convert decimal digits to coefficient bytes.
std::string DigitsToBytes(const std::vector<uint8_t>& digits, bool is_negative) {
  if (digits.empty() || (digits.size() == 1 && digits[0] == 0)) {
    return "";
  }

  // Convert base-10 digits to base-256 (big-endian)
  std::vector<uint8_t> value;
  std::vector<uint8_t> temp_digits = digits;  // Little-endian

  while (!temp_digits.empty()) {
    uint32_t remainder = 0;
    std::vector<uint8_t> quotient;

    for (int i = temp_digits.size() - 1; i >= 0; --i) {
      uint32_t dividend = remainder * 10 + temp_digits[i];
      uint8_t q = dividend / 256;
      remainder = dividend % 256;
      if (!quotient.empty() || q != 0) {
        quotient.insert(quotient.begin(), q);
      }
    }

    value.insert(value.begin(), static_cast<uint8_t>(remainder));
    temp_digits = quotient;
  }

  // Add leading zero if high bit is set (to keep positive)
  if (!is_negative && !value.empty() && (value[0] & 0x80)) {
    value.insert(value.begin(), 0);
  }

  // If negative, compute two's complement
  if (is_negative && !value.empty()) {
    // Ensure we have room for sign bit
    if (!(value[0] & 0x80)) {
      value.insert(value.begin(), 0);
    }

    bool carry = true;
    for (int i = value.size() - 1; i >= 0; --i) {
      value[i] = ~value[i];
      if (carry) {
        if (value[i] == 255) {
          value[i] = 0;
        } else {
          value[i]++;
          carry = false;
        }
      }
    }
  }

  return std::string(reinterpret_cast<char*>(value.data()), value.size());
}

}  // namespace

std::string DecimalUtil::ToString(const Decimal& decimal) {
  bool is_negative;
  std::vector<uint8_t> digits = BytesToDigits(decimal.coefficient(), &is_negative);

  int32_t scale = decimal.scale();

  // Build the string representation
  std::string result;
  if (is_negative) {
    result = "-";
  }

  if (scale <= 0) {
    // No decimal point, might need trailing zeros
    for (int i = digits.size() - 1; i >= 0; --i) {
      result += ('0' + digits[i]);
    }
    for (int i = 0; i < -scale; ++i) {
      result += '0';
    }
  } else if (scale >= static_cast<int32_t>(digits.size())) {
    // Need leading zeros after decimal point
    result += "0.";
    for (int i = 0; i < scale - static_cast<int32_t>(digits.size()); ++i) {
      result += '0';
    }
    for (int i = digits.size() - 1; i >= 0; --i) {
      result += ('0' + digits[i]);
    }
  } else {
    // Decimal point in the middle
    int integer_digits = digits.size() - scale;
    for (int i = digits.size() - 1; i >= scale; --i) {
      result += ('0' + digits[i]);
    }
    result += '.';
    for (int i = scale - 1; i >= 0; --i) {
      result += ('0' + digits[i]);
    }
  }

  return result;
}

bool DecimalUtil::FromString(absl::string_view value, Decimal* decimal) {
  if (value.empty()) {
    return false;
  }

  bool is_negative = false;
  size_t pos = 0;

  // Handle sign
  if (value[0] == '-') {
    is_negative = true;
    pos = 1;
  } else if (value[0] == '+') {
    pos = 1;
  }

  if (pos >= value.size()) {
    return false;
  }

  // Parse digits and find decimal point
  std::vector<uint8_t> digits;  // Little-endian
  int32_t scale = 0;
  bool found_decimal = false;
  bool found_digit = false;

  for (; pos < value.size(); ++pos) {
    char c = value[pos];
    if (c == '.') {
      if (found_decimal) {
        return false;  // Multiple decimal points
      }
      found_decimal = true;
    } else if (c >= '0' && c <= '9') {
      found_digit = true;
      if (found_decimal) {
        scale++;
      }
      digits.insert(digits.begin(), c - '0');
    } else {
      return false;  // Invalid character
    }
  }

  if (!found_digit) {
    return false;
  }

  // Remove leading zeros (which are trailing in little-endian)
  while (digits.size() > 1 && digits.back() == 0) {
    digits.pop_back();
  }

  // Convert to bytes
  std::string coefficient = DigitsToBytes(digits, is_negative);

  decimal->set_coefficient(coefficient);
  decimal->set_scale(scale);
  return true;
}

bool DecimalUtil::IsZero(const Decimal& decimal) {
  return decimal.coefficient().empty() ||
         (decimal.coefficient().size() == 1 && decimal.coefficient()[0] == 0);
}

bool DecimalUtil::IsNegative(const Decimal& decimal) {
  if (decimal.coefficient().empty()) {
    return false;
  }
  return (static_cast<uint8_t>(decimal.coefficient()[0]) & 0x80) != 0;
}

double DecimalUtil::ToDouble(const Decimal& decimal) {
  bool is_negative;
  std::vector<uint8_t> digits = BytesToDigits(decimal.coefficient(), &is_negative);

  double result = 0.0;
  double multiplier = 1.0;

  for (size_t i = 0; i < digits.size(); ++i) {
    result += digits[i] * multiplier;
    multiplier *= 10.0;
  }

  // Apply scale
  for (int32_t i = 0; i < decimal.scale(); ++i) {
    result /= 10.0;
  }

  return is_negative ? -result : result;
}

void DecimalUtil::FromDouble(double value, int32_t scale, Decimal* decimal) {
  // Scale the value
  for (int32_t i = 0; i < scale; ++i) {
    value *= 10.0;
  }

  int64_t int_value = static_cast<int64_t>(std::round(value));
  FromInt64(int_value, decimal);
  decimal->set_scale(scale);
}

int64_t DecimalUtil::ToInt64(const Decimal& decimal) {
  return static_cast<int64_t>(ToDouble(decimal));
}

void DecimalUtil::FromInt64(int64_t value, Decimal* decimal) {
  if (value == 0) {
    decimal->set_coefficient("");
    decimal->set_scale(0);
    return;
  }

  bool is_negative = value < 0;
  uint64_t abs_value = is_negative ? -static_cast<uint64_t>(value)
                                   : static_cast<uint64_t>(value);

  std::vector<uint8_t> digits;
  while (abs_value > 0) {
    digits.push_back(abs_value % 10);
    abs_value /= 10;
  }

  decimal->set_coefficient(DigitsToBytes(digits, is_negative));
  decimal->set_scale(0);
}

Decimal DecimalUtil::Add(const Decimal& d1, const Decimal& d2) {
  // Simplified implementation using double conversion
  // A production implementation would use arbitrary precision arithmetic
  double v1 = ToDouble(d1);
  double v2 = ToDouble(d2);
  int32_t scale = std::max(d1.scale(), d2.scale());

  Decimal result;
  FromDouble(v1 + v2, scale, &result);
  return result;
}

Decimal DecimalUtil::Subtract(const Decimal& d1, const Decimal& d2) {
  double v1 = ToDouble(d1);
  double v2 = ToDouble(d2);
  int32_t scale = std::max(d1.scale(), d2.scale());

  Decimal result;
  FromDouble(v1 - v2, scale, &result);
  return result;
}

Decimal DecimalUtil::Multiply(const Decimal& d1, const Decimal& d2) {
  double v1 = ToDouble(d1);
  double v2 = ToDouble(d2);
  int32_t scale = d1.scale() + d2.scale();

  Decimal result;
  FromDouble(v1 * v2, scale, &result);
  return result;
}

Decimal DecimalUtil::Divide(const Decimal& d1, const Decimal& d2, int32_t scale) {
  double v1 = ToDouble(d1);
  double v2 = ToDouble(d2);

  Decimal result;
  FromDouble(v1 / v2, scale, &result);
  return result;
}

Decimal DecimalUtil::Negate(const Decimal& decimal) {
  Decimal result;
  result.set_scale(decimal.scale());

  if (decimal.coefficient().empty()) {
    return result;
  }

  // Negate by computing two's complement
  std::string coeff = decimal.coefficient();
  bool carry = true;

  for (int i = coeff.size() - 1; i >= 0; --i) {
    coeff[i] = ~coeff[i];
    if (carry) {
      if (static_cast<uint8_t>(coeff[i]) == 255) {
        coeff[i] = 0;
      } else {
        coeff[i]++;
        carry = false;
      }
    }
  }

  result.set_coefficient(coeff);
  return result;
}

Decimal DecimalUtil::Abs(const Decimal& decimal) {
  if (IsNegative(decimal)) {
    return Negate(decimal);
  }
  return decimal;
}

int DecimalUtil::Compare(const Decimal& d1, const Decimal& d2) {
  double v1 = ToDouble(d1);
  double v2 = ToDouble(d2);

  if (v1 < v2) return -1;
  if (v1 > v2) return 1;
  return 0;
}

Decimal DecimalUtil::Zero() {
  Decimal result;
  result.set_coefficient("");
  result.set_scale(0);
  return result;
}

int DecimalUtil::Sign(const Decimal& decimal) {
  if (IsZero(decimal)) return 0;
  return IsNegative(decimal) ? -1 : 1;
}

Decimal DecimalUtil::Round(const Decimal& decimal, int32_t scale) {
  double value = ToDouble(decimal);
  Decimal result;
  FromDouble(value, scale, &result);
  return result;
}

Decimal DecimalUtil::Truncate(const Decimal& decimal, int32_t scale) {
  double value = ToDouble(decimal);
  double multiplier = std::pow(10.0, scale);
  value = std::trunc(value * multiplier) / multiplier;

  Decimal result;
  FromDouble(value, scale, &result);
  return result;
}

void DecimalUtil::FromCoefficient(const std::string& coefficient, int32_t scale,
                                   Decimal* decimal) {
  decimal->set_coefficient(coefficient);
  decimal->set_scale(scale);
}

}  // namespace util
}  // namespace protobuf
}  // namespace google
