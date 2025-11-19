// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Defines utilities for the Decimal well known type.

#ifndef GOOGLE_PROTOBUF_UTIL_DECIMAL_UTIL_H__
#define GOOGLE_PROTOBUF_UTIL_DECIMAL_UTIL_H__

#include <cstdint>
#include <ostream>
#include <string>

#include "google/protobuf/decimal.pb.h"
#include "absl/strings/string_view.h"

// Must be included last.
#include "google/protobuf/port_def.inc"

namespace google {
namespace protobuf {
namespace util {

// Utility functions for Decimal.
class PROTOBUF_EXPORT DecimalUtil {
  typedef google::protobuf::Decimal Decimal;

 public:
  // Maximum recommended precision (38 digits like SQL Server DECIMAL).
  static constexpr int32_t kMaxPrecision = 38;
  static constexpr int32_t kMaxScale = 38;

  // Validates the decimal value.
  static bool IsValid(const Decimal& decimal) {
    return decimal.scale() >= 0 && decimal.scale() <= kMaxScale;
  }

  // Checks if the decimal is zero.
  static bool IsZero(const Decimal& decimal);

  // Checks if the decimal is negative.
  static bool IsNegative(const Decimal& decimal);

  // Converts Decimal to/from string format.
  // Format: [-]digits[.digits]
  // Examples: "123.45", "-0.001", "1000000"
  static std::string ToString(const Decimal& decimal);
  static bool FromString(absl::string_view value, Decimal* decimal);

  // Converts to/from double (lossy conversion).
  static double ToDouble(const Decimal& decimal);
  static void FromDouble(double value, int32_t scale, Decimal* decimal);

  // Converts to/from int64 (truncates fractional part).
  static int64_t ToInt64(const Decimal& decimal);
  static void FromInt64(int64_t value, Decimal* decimal);

  // Arithmetic operations.
  // These return a new Decimal with the result.
  static Decimal Add(const Decimal& d1, const Decimal& d2);
  static Decimal Subtract(const Decimal& d1, const Decimal& d2);
  static Decimal Multiply(const Decimal& d1, const Decimal& d2);
  static Decimal Divide(const Decimal& d1, const Decimal& d2, int32_t scale);

  // Unary operations.
  static Decimal Negate(const Decimal& decimal);
  static Decimal Abs(const Decimal& decimal);

  // Rounding operations.
  static Decimal Round(const Decimal& decimal, int32_t scale);
  static Decimal Truncate(const Decimal& decimal, int32_t scale);

  // Comparison (-1, 0, or 1).
  static int Compare(const Decimal& d1, const Decimal& d2);

  // Creates a zero Decimal.
  static Decimal Zero();

  // Creates a Decimal from coefficient and scale.
  static void FromCoefficient(const std::string& coefficient, int32_t scale,
                              Decimal* decimal);

  // Gets the sign (-1, 0, or 1).
  static int Sign(const Decimal& decimal);
};

}  // namespace util
}  // namespace protobuf
}  // namespace google

namespace google {
namespace protobuf {

// Relational operators for Decimal.
inline bool operator==(const Decimal& d1, const Decimal& d2) {
  return google::protobuf::util::DecimalUtil::Compare(d1, d2) == 0;
}

inline bool operator!=(const Decimal& d1, const Decimal& d2) {
  return google::protobuf::util::DecimalUtil::Compare(d1, d2) != 0;
}

inline bool operator<(const Decimal& d1, const Decimal& d2) {
  return google::protobuf::util::DecimalUtil::Compare(d1, d2) < 0;
}

inline bool operator>(const Decimal& d1, const Decimal& d2) {
  return google::protobuf::util::DecimalUtil::Compare(d1, d2) > 0;
}

inline bool operator<=(const Decimal& d1, const Decimal& d2) {
  return google::protobuf::util::DecimalUtil::Compare(d1, d2) <= 0;
}

inline bool operator>=(const Decimal& d1, const Decimal& d2) {
  return google::protobuf::util::DecimalUtil::Compare(d1, d2) >= 0;
}

// Arithmetic operators.
inline Decimal operator+(const Decimal& d1, const Decimal& d2) {
  return google::protobuf::util::DecimalUtil::Add(d1, d2);
}

inline Decimal operator-(const Decimal& d1, const Decimal& d2) {
  return google::protobuf::util::DecimalUtil::Subtract(d1, d2);
}

inline Decimal operator*(const Decimal& d1, const Decimal& d2) {
  return google::protobuf::util::DecimalUtil::Multiply(d1, d2);
}

inline Decimal operator-(const Decimal& d) {
  return google::protobuf::util::DecimalUtil::Negate(d);
}

inline std::ostream& operator<<(std::ostream& out, const Decimal& decimal) {
  out << google::protobuf::util::DecimalUtil::ToString(decimal);
  return out;
}

}  // namespace protobuf
}  // namespace google

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_UTIL_DECIMAL_UTIL_H__
