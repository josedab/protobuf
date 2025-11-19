// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/util/timestamp_tz_util.h"

#include <ctime>

#include "absl/strings/str_format.h"
#include "absl/strings/numbers.h"
#include "google/protobuf/util/time_util.h"

namespace google {
namespace protobuf {
namespace util {

namespace {

// Parse an integer from a string view, advancing the position.
bool ParseInt(absl::string_view& str, int count, int* result) {
  if (str.size() < static_cast<size_t>(count)) {
    return false;
  }
  *result = 0;
  for (int i = 0; i < count; ++i) {
    char c = str[i];
    if (c < '0' || c > '9') {
      return false;
    }
    *result = *result * 10 + (c - '0');
  }
  str = str.substr(count);
  return true;
}

// Convert broken-down time to Unix timestamp.
int64_t TimeToSeconds(int year, int month, int day, int hour, int minute,
                      int second) {
  // Days from year 1 to year
  int64_t days = (year - 1) * 365 + (year - 1) / 4 - (year - 1) / 100 +
                 (year - 1) / 400;

  // Days in months of this year
  static const int days_before_month[] = {0,   31,  59,  90,  120, 151,
                                          181, 212, 243, 273, 304, 334};
  days += days_before_month[month - 1];

  // Add leap day if applicable
  if (month > 2) {
    bool is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (is_leap) {
      days += 1;
    }
  }

  days += day - 1;

  // Days from epoch (1970-01-01)
  static const int64_t epoch_days = 719162;  // Days from year 1 to 1970
  days -= epoch_days;

  return days * 86400 + hour * 3600 + minute * 60 + second;
}

// Convert Unix timestamp to broken-down time.
void SecondsToTime(int64_t seconds, int* year, int* month, int* day, int* hour,
                   int* minute, int* second) {
  // Handle time of day
  int64_t remaining = seconds % 86400;
  if (remaining < 0) {
    remaining += 86400;
    seconds -= 86400;
  }

  *hour = remaining / 3600;
  remaining %= 3600;
  *minute = remaining / 60;
  *second = remaining % 60;

  // Handle date
  int64_t days = seconds / 86400 + 719162;  // Days from year 1

  // Approximate year
  *year = static_cast<int>(days / 365);
  while (true) {
    int64_t year_days =
        (*year - 1) * 365 + (*year - 1) / 4 - (*year - 1) / 100 + (*year - 1) / 400;
    if (year_days <= days) {
      int64_t next_year_days = *year * 365 + *year / 4 - *year / 100 + *year / 400;
      if (next_year_days > days) {
        days -= year_days;
        break;
      }
      (*year)++;
    } else {
      (*year)--;
    }
  }

  // Month and day
  bool is_leap = (*year % 4 == 0 && *year % 100 != 0) || (*year % 400 == 0);
  static const int days_in_month[] = {31, 28, 31, 30, 31, 30,
                                      31, 31, 30, 31, 30, 31};

  *month = 1;
  while (days >= days_in_month[*month - 1] +
                     ((*month == 2 && is_leap) ? 1 : 0)) {
    days -= days_in_month[*month - 1] + ((*month == 2 && is_leap) ? 1 : 0);
    (*month)++;
  }
  *day = days + 1;
}

}  // namespace

std::string TimestampTzUtil::ToString(const TimestampTz& timestamp) {
  int year, month, day, hour, minute, second;

  // Apply UTC offset to get local time
  int64_t local_seconds = timestamp.seconds() + timestamp.utc_offset_seconds();
  SecondsToTime(local_seconds, &year, &month, &day, &hour, &minute, &second);

  // Format the offset
  int offset_hours = std::abs(timestamp.utc_offset_seconds()) / 3600;
  int offset_minutes = (std::abs(timestamp.utc_offset_seconds()) % 3600) / 60;
  char offset_sign = timestamp.utc_offset_seconds() >= 0 ? '+' : '-';

  std::string result = absl::StrFormat(
      "%04d-%02d-%02dT%02d:%02d:%02d", year, month, day, hour, minute, second);

  // Add nanoseconds if present
  if (timestamp.nanos() > 0) {
    if (timestamp.nanos() % 1000000 == 0) {
      result += absl::StrFormat(".%03d", timestamp.nanos() / 1000000);
    } else if (timestamp.nanos() % 1000 == 0) {
      result += absl::StrFormat(".%06d", timestamp.nanos() / 1000);
    } else {
      result += absl::StrFormat(".%09d", timestamp.nanos());
    }
  }

  // Add offset
  result += absl::StrFormat("%c%02d:%02d", offset_sign, offset_hours, offset_minutes);

  // Add timezone name if present
  if (!timestamp.timezone().empty()) {
    result += "[";
    result += timestamp.timezone();
    result += "]";
  }

  return result;
}

bool TimestampTzUtil::FromString(absl::string_view value, TimestampTz* timestamp) {
  // Parse date: YYYY-MM-DD
  int year, month, day, hour = 0, minute = 0, second = 0;
  int32_t nanos = 0;

  if (!ParseInt(value, 4, &year)) return false;
  if (value.empty() || value[0] != '-') return false;
  value = value.substr(1);
  if (!ParseInt(value, 2, &month)) return false;
  if (value.empty() || value[0] != '-') return false;
  value = value.substr(1);
  if (!ParseInt(value, 2, &day)) return false;

  // Parse time: THH:MM:SS
  if (!value.empty() && (value[0] == 'T' || value[0] == ' ')) {
    value = value.substr(1);
    if (!ParseInt(value, 2, &hour)) return false;
    if (value.empty() || value[0] != ':') return false;
    value = value.substr(1);
    if (!ParseInt(value, 2, &minute)) return false;
    if (value.empty() || value[0] != ':') return false;
    value = value.substr(1);
    if (!ParseInt(value, 2, &second)) return false;

    // Parse fractional seconds
    if (!value.empty() && value[0] == '.') {
      value = value.substr(1);
      int frac_digits = 0;
      int frac_value = 0;
      while (!value.empty() && value[0] >= '0' && value[0] <= '9' &&
             frac_digits < 9) {
        frac_value = frac_value * 10 + (value[0] - '0');
        frac_digits++;
        value = value.substr(1);
      }
      // Pad to nanoseconds
      while (frac_digits < 9) {
        frac_value *= 10;
        frac_digits++;
      }
      nanos = frac_value;
    }
  }

  // Parse offset
  int32_t offset_seconds = 0;
  if (!value.empty() && value[0] == 'Z') {
    value = value.substr(1);
  } else if (!value.empty() && (value[0] == '+' || value[0] == '-')) {
    bool negative = value[0] == '-';
    value = value.substr(1);
    int offset_hours, offset_minutes = 0;
    if (!ParseInt(value, 2, &offset_hours)) return false;
    if (!value.empty() && value[0] == ':') {
      value = value.substr(1);
      if (!ParseInt(value, 2, &offset_minutes)) return false;
    }
    offset_seconds = offset_hours * 3600 + offset_minutes * 60;
    if (negative) {
      offset_seconds = -offset_seconds;
    }
  }

  // Parse timezone name
  std::string timezone;
  if (!value.empty() && value[0] == '[') {
    value = value.substr(1);
    size_t end = value.find(']');
    if (end == absl::string_view::npos) return false;
    timezone = std::string(value.substr(0, end));
    value = value.substr(end + 1);
  }

  // Convert to UTC seconds
  int64_t utc_seconds = TimeToSeconds(year, month, day, hour, minute, second);
  utc_seconds -= offset_seconds;  // Convert local time to UTC

  timestamp->set_seconds(utc_seconds);
  timestamp->set_nanos(nanos);
  timestamp->set_timezone(timezone);
  timestamp->set_utc_offset_seconds(offset_seconds);

  return true;
}

std::string TimestampTzUtil::ToLocalTimeString(const TimestampTz& timestamp) {
  int year, month, day, hour, minute, second;

  int64_t local_seconds = timestamp.seconds() + timestamp.utc_offset_seconds();
  SecondsToTime(local_seconds, &year, &month, &day, &hour, &minute, &second);

  std::string result = absl::StrFormat(
      "%04d-%02d-%02dT%02d:%02d:%02d", year, month, day, hour, minute, second);

  if (timestamp.nanos() > 0) {
    if (timestamp.nanos() % 1000000 == 0) {
      result += absl::StrFormat(".%03d", timestamp.nanos() / 1000000);
    } else if (timestamp.nanos() % 1000 == 0) {
      result += absl::StrFormat(".%06d", timestamp.nanos() / 1000);
    } else {
      result += absl::StrFormat(".%09d", timestamp.nanos());
    }
  }

  return result;
}

Timestamp TimestampTzUtil::ToTimestamp(const TimestampTz& timestamp_tz) {
  Timestamp timestamp;
  timestamp.set_seconds(timestamp_tz.seconds());
  timestamp.set_nanos(timestamp_tz.nanos());
  return timestamp;
}

void TimestampTzUtil::FromTimestamp(const Timestamp& timestamp,
                                     absl::string_view timezone,
                                     TimestampTz* timestamp_tz) {
  timestamp_tz->set_seconds(timestamp.seconds());
  timestamp_tz->set_nanos(timestamp.nanos());
  timestamp_tz->set_timezone(std::string(timezone));

  // For simplicity, assume UTC offset is 0 for UTC timezone
  // A full implementation would use a timezone database
  if (timezone == "UTC" || timezone.empty()) {
    timestamp_tz->set_utc_offset_seconds(0);
  } else {
    // Placeholder: proper implementation would look up timezone database
    timestamp_tz->set_utc_offset_seconds(0);
  }
}

TimestampTz TimestampTzUtil::Now(absl::string_view timezone) {
  time_t now = std::time(nullptr);
  TimestampTz result;
  result.set_seconds(now);
  result.set_nanos(0);
  result.set_timezone(std::string(timezone));

  // For simplicity, use UTC offset 0
  // A full implementation would query the timezone database
  result.set_utc_offset_seconds(0);

  return result;
}

TimestampTz TimestampTzUtil::NowUTC() {
  return Now("UTC");
}

void TimestampTzUtil::WithTimezone(const TimestampTz& source,
                                    absl::string_view timezone,
                                    TimestampTz* result) {
  result->set_seconds(source.seconds());
  result->set_nanos(source.nanos());
  result->set_timezone(std::string(timezone));

  // A full implementation would look up the new timezone's offset
  result->set_utc_offset_seconds(source.utc_offset_seconds());
}

int32_t TimestampTzUtil::GetUtcOffset(absl::string_view timezone,
                                       int64_t utc_seconds) {
  // Placeholder implementation
  // A full implementation would use a timezone database like IANA tzdata
  if (timezone == "UTC" || timezone.empty()) {
    return 0;
  }
  return 0;
}

bool TimestampTzUtil::IsValidTimezone(absl::string_view timezone) {
  // Placeholder: accept common timezone names
  return !timezone.empty();
}

std::string TimestampTzUtil::GetTimezoneAbbreviation(absl::string_view timezone,
                                                      int64_t utc_seconds) {
  // Placeholder implementation
  if (timezone == "UTC" || timezone.empty()) {
    return "UTC";
  }
  return std::string(timezone);
}

bool TimestampTzUtil::IsDaylightSavingTime(absl::string_view timezone,
                                            int64_t utc_seconds) {
  // Placeholder implementation
  return false;
}

TimestampTz TimestampTzUtil::Add(const TimestampTz& ts, const Duration& d) {
  TimestampTz result = ts;
  result += d;
  return result;
}

TimestampTz TimestampTzUtil::Subtract(const TimestampTz& ts, const Duration& d) {
  TimestampTz result = ts;
  result -= d;
  return result;
}

Duration TimestampTzUtil::Difference(const TimestampTz& t1,
                                      const TimestampTz& t2) {
  return t1 - t2;
}

TimestampTz TimestampTzUtil::FromTimeT(time_t value,
                                        absl::string_view timezone) {
  TimestampTz result;
  result.set_seconds(value);
  result.set_nanos(0);
  result.set_timezone(std::string(timezone));
  result.set_utc_offset_seconds(0);
  return result;
}

time_t TimestampTzUtil::ToTimeT(const TimestampTz& timestamp) {
  return static_cast<time_t>(timestamp.seconds());
}

int TimestampTzUtil::GetYear(const TimestampTz& timestamp) {
  int year, month, day, hour, minute, second;
  int64_t local = timestamp.seconds() + timestamp.utc_offset_seconds();
  SecondsToTime(local, &year, &month, &day, &hour, &minute, &second);
  return year;
}

int TimestampTzUtil::GetMonth(const TimestampTz& timestamp) {
  int year, month, day, hour, minute, second;
  int64_t local = timestamp.seconds() + timestamp.utc_offset_seconds();
  SecondsToTime(local, &year, &month, &day, &hour, &minute, &second);
  return month;
}

int TimestampTzUtil::GetDay(const TimestampTz& timestamp) {
  int year, month, day, hour, minute, second;
  int64_t local = timestamp.seconds() + timestamp.utc_offset_seconds();
  SecondsToTime(local, &year, &month, &day, &hour, &minute, &second);
  return day;
}

int TimestampTzUtil::GetHour(const TimestampTz& timestamp) {
  int year, month, day, hour, minute, second;
  int64_t local = timestamp.seconds() + timestamp.utc_offset_seconds();
  SecondsToTime(local, &year, &month, &day, &hour, &minute, &second);
  return hour;
}

int TimestampTzUtil::GetMinute(const TimestampTz& timestamp) {
  int year, month, day, hour, minute, second;
  int64_t local = timestamp.seconds() + timestamp.utc_offset_seconds();
  SecondsToTime(local, &year, &month, &day, &hour, &minute, &second);
  return minute;
}

int TimestampTzUtil::GetSecond(const TimestampTz& timestamp) {
  int year, month, day, hour, minute, second;
  int64_t local = timestamp.seconds() + timestamp.utc_offset_seconds();
  SecondsToTime(local, &year, &month, &day, &hour, &minute, &second);
  return second;
}

int TimestampTzUtil::GetDayOfWeek(const TimestampTz& timestamp) {
  int64_t local = timestamp.seconds() + timestamp.utc_offset_seconds();
  int64_t days = local / 86400;
  // January 1, 1970 was Thursday (4)
  int dow = (days + 4) % 7;
  if (dow < 0) dow += 7;
  return dow;
}

int TimestampTzUtil::GetDayOfYear(const TimestampTz& timestamp) {
  int year, month, day, hour, minute, second;
  int64_t local = timestamp.seconds() + timestamp.utc_offset_seconds();
  SecondsToTime(local, &year, &month, &day, &hour, &minute, &second);

  bool is_leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
  static const int days_before_month[] = {0,   31,  59,  90,  120, 151,
                                          181, 212, 243, 273, 304, 334};
  int doy = days_before_month[month - 1] + day;
  if (month > 2 && is_leap) {
    doy++;
  }
  return doy;
}

}  // namespace util
}  // namespace protobuf
}  // namespace google

namespace google {
namespace protobuf {

TimestampTz& operator+=(TimestampTz& t, const Duration& d) {
  int64_t seconds = t.seconds() + d.seconds();
  int32_t nanos = t.nanos() + d.nanos();

  if (nanos >= 1000000000) {
    seconds++;
    nanos -= 1000000000;
  } else if (nanos < 0) {
    seconds--;
    nanos += 1000000000;
  }

  t.set_seconds(seconds);
  t.set_nanos(nanos);
  return t;
}

TimestampTz& operator-=(TimestampTz& t, const Duration& d) {
  int64_t seconds = t.seconds() - d.seconds();
  int32_t nanos = t.nanos() - d.nanos();

  if (nanos < 0) {
    seconds--;
    nanos += 1000000000;
  } else if (nanos >= 1000000000) {
    seconds++;
    nanos -= 1000000000;
  }

  t.set_seconds(seconds);
  t.set_nanos(nanos);
  return t;
}

Duration operator-(const TimestampTz& t1, const TimestampTz& t2) {
  Duration d;
  int64_t seconds = t1.seconds() - t2.seconds();
  int32_t nanos = t1.nanos() - t2.nanos();

  if (nanos < 0) {
    seconds--;
    nanos += 1000000000;
  }

  d.set_seconds(seconds);
  d.set_nanos(nanos);
  return d;
}

}  // namespace protobuf
}  // namespace google
