// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Defines utilities for the TimestampTz well known type.

#ifndef GOOGLE_PROTOBUF_UTIL_TIMESTAMP_TZ_UTIL_H__
#define GOOGLE_PROTOBUF_UTIL_TIMESTAMP_TZ_UTIL_H__

#include <cstdint>
#include <ctime>
#include <ostream>
#include <string>

#include "google/protobuf/timestamp_tz.pb.h"
#include "google/protobuf/timestamp.pb.h"
#include "google/protobuf/duration.pb.h"
#include "absl/strings/string_view.h"

// Must be included last.
#include "google/protobuf/port_def.inc"

namespace google {
namespace protobuf {
namespace util {

// Utility functions for TimestampTz.
class PROTOBUF_EXPORT TimestampTzUtil {
  typedef google::protobuf::TimestampTz TimestampTz;
  typedef google::protobuf::Timestamp Timestamp;
  typedef google::protobuf::Duration Duration;

 public:
  // The min/max TimestampTz values (same as Timestamp).
  static constexpr int64_t kTimestampMinSeconds = -62135596800LL;
  static constexpr int64_t kTimestampMaxSeconds = 253402300799LL;
  static constexpr int32_t kTimestampMinNanoseconds = 0;
  static constexpr int32_t kTimestampMaxNanoseconds = 999999999;

  // Common timezone names.
  static constexpr absl::string_view kTimezoneUTC = "UTC";
  static constexpr absl::string_view kTimezoneNewYork = "America/New_York";
  static constexpr absl::string_view kTimezoneLosAngeles = "America/Los_Angeles";
  static constexpr absl::string_view kTimezoneLondon = "Europe/London";
  static constexpr absl::string_view kTimezoneTokyo = "Asia/Tokyo";

  // Validates the timestamp.
  static bool IsValid(const TimestampTz& timestamp) {
    return timestamp.seconds() <= kTimestampMaxSeconds &&
           timestamp.seconds() >= kTimestampMinSeconds &&
           timestamp.nanos() <= kTimestampMaxNanoseconds &&
           timestamp.nanos() >= kTimestampMinNanoseconds;
  }

  // Converts TimestampTz to/from string format.
  // Format: "2024-01-15T10:30:00-05:00[America/New_York]"
  // or without timezone name: "2024-01-15T10:30:00-05:00"
  static std::string ToString(const TimestampTz& timestamp);
  static bool FromString(absl::string_view value, TimestampTz* timestamp);

  // Converts to local time string in the stored timezone.
  // Format: "2024-01-15T10:30:00"
  static std::string ToLocalTimeString(const TimestampTz& timestamp);

  // Converts to/from UTC Timestamp.
  static Timestamp ToTimestamp(const TimestampTz& timestamp_tz);
  static void FromTimestamp(const Timestamp& timestamp,
                            absl::string_view timezone,
                            TimestampTz* timestamp_tz);

  // Gets current time in the specified timezone.
  static TimestampTz Now(absl::string_view timezone);

  // Gets current time in UTC.
  static TimestampTz NowUTC();

  // Creates TimestampTz in a specific timezone.
  static void WithTimezone(const TimestampTz& source,
                           absl::string_view timezone,
                           TimestampTz* result);

  // Time zone operations.
  // Gets the UTC offset for a timezone at a specific time.
  static int32_t GetUtcOffset(absl::string_view timezone, int64_t utc_seconds);

  // Checks if a timezone name is valid.
  static bool IsValidTimezone(absl::string_view timezone);

  // Gets the timezone abbreviation (e.g., "EST", "PDT") for a timezone at
  // a specific time.
  static std::string GetTimezoneAbbreviation(absl::string_view timezone,
                                              int64_t utc_seconds);

  // Checks if daylight saving time is in effect.
  static bool IsDaylightSavingTime(absl::string_view timezone,
                                    int64_t utc_seconds);

  // Arithmetic with Duration.
  static TimestampTz Add(const TimestampTz& ts, const Duration& d);
  static TimestampTz Subtract(const TimestampTz& ts, const Duration& d);
  static Duration Difference(const TimestampTz& t1, const TimestampTz& t2);

  // Conversion to/from time_t (loses timezone info).
  static TimestampTz FromTimeT(time_t value, absl::string_view timezone);
  static time_t ToTimeT(const TimestampTz& timestamp);

  // Date/time component extraction in local time.
  static int GetYear(const TimestampTz& timestamp);
  static int GetMonth(const TimestampTz& timestamp);  // 1-12
  static int GetDay(const TimestampTz& timestamp);    // 1-31
  static int GetHour(const TimestampTz& timestamp);   // 0-23
  static int GetMinute(const TimestampTz& timestamp); // 0-59
  static int GetSecond(const TimestampTz& timestamp); // 0-59
  static int GetDayOfWeek(const TimestampTz& timestamp); // 0=Sunday
  static int GetDayOfYear(const TimestampTz& timestamp); // 1-366
};

}  // namespace util
}  // namespace protobuf
}  // namespace google

namespace google {
namespace protobuf {

// Assignment operators.
PROTOBUF_EXPORT TimestampTz& operator+=(TimestampTz& t,
                                        const Duration& d);  // NOLINT
PROTOBUF_EXPORT TimestampTz& operator-=(TimestampTz& t,
                                        const Duration& d);  // NOLINT

// Relational operators (compares UTC time, ignores timezone).
inline bool operator<(const TimestampTz& t1, const TimestampTz& t2) {
  if (t1.seconds() == t2.seconds()) {
    return t1.nanos() < t2.nanos();
  }
  return t1.seconds() < t2.seconds();
}

inline bool operator>(const TimestampTz& t1, const TimestampTz& t2) {
  return t2 < t1;
}

inline bool operator>=(const TimestampTz& t1, const TimestampTz& t2) {
  return !(t1 < t2);
}

inline bool operator<=(const TimestampTz& t1, const TimestampTz& t2) {
  return !(t2 < t1);
}

inline bool operator==(const TimestampTz& t1, const TimestampTz& t2) {
  return t1.seconds() == t2.seconds() && t1.nanos() == t2.nanos();
}

inline bool operator!=(const TimestampTz& t1, const TimestampTz& t2) {
  return !(t1 == t2);
}

// Additive operators.
inline TimestampTz operator+(const TimestampTz& t, const Duration& d) {
  TimestampTz result = t;
  return result += d;
}

inline TimestampTz operator+(const Duration& d, const TimestampTz& t) {
  TimestampTz result = t;
  return result += d;
}

inline TimestampTz operator-(const TimestampTz& t, const Duration& d) {
  TimestampTz result = t;
  return result -= d;
}

PROTOBUF_EXPORT Duration operator-(const TimestampTz& t1,
                                    const TimestampTz& t2);

inline std::ostream& operator<<(std::ostream& out, const TimestampTz& t) {
  out << google::protobuf::util::TimestampTzUtil::ToString(t);
  return out;
}

}  // namespace protobuf
}  // namespace google

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_UTIL_TIMESTAMP_TZ_UTIL_H__
