// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/util/timestamp_tz_util.h"

#include <cstdint>

#include "google/protobuf/timestamp_tz.pb.h"
#include "google/protobuf/duration.pb.h"
#include "google/protobuf/testing/googletest.h"
#include <gtest/gtest.h>

namespace google {
namespace protobuf {
namespace util {

using google::protobuf::TimestampTz;
using google::protobuf::Duration;

namespace {

TEST(TimestampTzUtilTest, StringConversion) {
  TimestampTz ts;

  // Parse with timezone
  EXPECT_TRUE(TimestampTzUtil::FromString(
      "2024-01-15T10:30:00-05:00[America/New_York]", &ts));
  EXPECT_EQ("America/New_York", ts.timezone());
  EXPECT_EQ(-5 * 3600, ts.utc_offset_seconds());

  // Parse without timezone name
  EXPECT_TRUE(TimestampTzUtil::FromString("2024-01-15T10:30:00+00:00", &ts));
  EXPECT_EQ("", ts.timezone());
  EXPECT_EQ(0, ts.utc_offset_seconds());

  // Parse UTC with Z
  EXPECT_TRUE(TimestampTzUtil::FromString("2024-01-15T10:30:00Z", &ts));
  EXPECT_EQ(0, ts.utc_offset_seconds());

  // Parse with fractional seconds
  EXPECT_TRUE(TimestampTzUtil::FromString("2024-01-15T10:30:00.123456789Z", &ts));
  EXPECT_EQ(123456789, ts.nanos());

  // Invalid strings
  EXPECT_FALSE(TimestampTzUtil::FromString("", &ts));
  EXPECT_FALSE(TimestampTzUtil::FromString("not-a-timestamp", &ts));
}

TEST(TimestampTzUtilTest, Validation) {
  TimestampTz ts;
  ts.set_seconds(0);
  ts.set_nanos(0);
  EXPECT_TRUE(TimestampTzUtil::IsValid(ts));

  // Invalid nanos
  ts.set_nanos(1000000000);
  EXPECT_FALSE(TimestampTzUtil::IsValid(ts));

  // Invalid seconds (too large)
  ts.set_seconds(TimestampTzUtil::kTimestampMaxSeconds + 1);
  ts.set_nanos(0);
  EXPECT_FALSE(TimestampTzUtil::IsValid(ts));
}

TEST(TimestampTzUtilTest, LocalTimeString) {
  TimestampTz ts;
  TimestampTzUtil::FromString("2024-01-15T10:30:00-05:00", &ts);

  std::string local = TimestampTzUtil::ToLocalTimeString(ts);
  EXPECT_EQ("2024-01-15T10:30:00", local);
}

TEST(TimestampTzUtilTest, TimestampConversion) {
  TimestampTz ts_tz;
  TimestampTzUtil::FromString("2024-01-15T10:30:00Z", &ts_tz);

  Timestamp ts = TimestampTzUtil::ToTimestamp(ts_tz);
  EXPECT_EQ(ts_tz.seconds(), ts.seconds());
  EXPECT_EQ(ts_tz.nanos(), ts.nanos());

  // Convert back
  TimestampTz ts_tz2;
  TimestampTzUtil::FromTimestamp(ts, "UTC", &ts_tz2);
  EXPECT_EQ(ts_tz.seconds(), ts_tz2.seconds());
  EXPECT_EQ(ts_tz.nanos(), ts_tz2.nanos());
}

TEST(TimestampTzUtilTest, Now) {
  TimestampTz now = TimestampTzUtil::NowUTC();
  EXPECT_TRUE(TimestampTzUtil::IsValid(now));
  EXPECT_GT(now.seconds(), 0);
  EXPECT_EQ("UTC", now.timezone());

  TimestampTz now_ny = TimestampTzUtil::Now("America/New_York");
  EXPECT_EQ("America/New_York", now_ny.timezone());
}

TEST(TimestampTzUtilTest, WithTimezone) {
  TimestampTz ts1, ts2;
  TimestampTzUtil::FromString("2024-01-15T10:30:00Z[UTC]", &ts1);

  TimestampTzUtil::WithTimezone(ts1, "America/Los_Angeles", &ts2);
  EXPECT_EQ("America/Los_Angeles", ts2.timezone());
  // UTC time should be the same
  EXPECT_EQ(ts1.seconds(), ts2.seconds());
  EXPECT_EQ(ts1.nanos(), ts2.nanos());
}

TEST(TimestampTzUtilTest, DurationArithmetic) {
  TimestampTz ts;
  TimestampTzUtil::FromString("2024-01-15T10:30:00Z", &ts);
  int64_t original_seconds = ts.seconds();

  Duration d;
  d.set_seconds(3600);  // 1 hour
  d.set_nanos(0);

  // Add
  TimestampTz result = TimestampTzUtil::Add(ts, d);
  EXPECT_EQ(original_seconds + 3600, result.seconds());

  // Subtract
  result = TimestampTzUtil::Subtract(ts, d);
  EXPECT_EQ(original_seconds - 3600, result.seconds());

  // Difference
  TimestampTz ts2;
  ts2.set_seconds(original_seconds + 7200);
  ts2.set_nanos(0);
  Duration diff = TimestampTzUtil::Difference(ts2, ts);
  EXPECT_EQ(7200, diff.seconds());
}

TEST(TimestampTzUtilTest, Comparison) {
  TimestampTz t1, t2, t3;
  t1.set_seconds(1000);
  t1.set_nanos(0);
  t2.set_seconds(2000);
  t2.set_nanos(0);
  t3.set_seconds(1000);
  t3.set_nanos(0);

  EXPECT_EQ(t1, t3);
  EXPECT_NE(t1, t2);
  EXPECT_LT(t1, t2);
  EXPECT_GT(t2, t1);
  EXPECT_LE(t1, t3);
  EXPECT_GE(t1, t3);
}

TEST(TimestampTzUtilTest, Operators) {
  TimestampTz ts;
  ts.set_seconds(1000);
  ts.set_nanos(500000000);  // 0.5 seconds

  Duration d;
  d.set_seconds(1);
  d.set_nanos(0);

  // +=
  ts += d;
  EXPECT_EQ(1001, ts.seconds());
  EXPECT_EQ(500000000, ts.nanos());

  // -=
  ts -= d;
  EXPECT_EQ(1000, ts.seconds());
  EXPECT_EQ(500000000, ts.nanos());

  // +
  TimestampTz result = ts + d;
  EXPECT_EQ(1001, result.seconds());

  // -
  result = ts - d;
  EXPECT_EQ(999, result.seconds());
}

TEST(TimestampTzUtilTest, DateTimeComponents) {
  TimestampTz ts;
  // 2024-01-15T10:30:45 UTC
  TimestampTzUtil::FromString("2024-01-15T10:30:45Z", &ts);

  EXPECT_EQ(2024, TimestampTzUtil::GetYear(ts));
  EXPECT_EQ(1, TimestampTzUtil::GetMonth(ts));
  EXPECT_EQ(15, TimestampTzUtil::GetDay(ts));
  EXPECT_EQ(10, TimestampTzUtil::GetHour(ts));
  EXPECT_EQ(30, TimestampTzUtil::GetMinute(ts));
  EXPECT_EQ(45, TimestampTzUtil::GetSecond(ts));
}

TEST(TimestampTzUtilTest, DayOfWeek) {
  TimestampTz ts;
  // January 15, 2024 was a Monday
  TimestampTzUtil::FromString("2024-01-15T00:00:00Z", &ts);
  EXPECT_EQ(1, TimestampTzUtil::GetDayOfWeek(ts));  // 1 = Monday
}

TEST(TimestampTzUtilTest, DayOfYear) {
  TimestampTz ts;
  // January 15 is day 15 of the year
  TimestampTzUtil::FromString("2024-01-15T00:00:00Z", &ts);
  EXPECT_EQ(15, TimestampTzUtil::GetDayOfYear(ts));

  // December 31 in leap year (2024) is day 366
  TimestampTzUtil::FromString("2024-12-31T00:00:00Z", &ts);
  EXPECT_EQ(366, TimestampTzUtil::GetDayOfYear(ts));
}

TEST(TimestampTzUtilTest, TimeTConversion) {
  time_t t = 1705312200;  // 2024-01-15T10:30:00Z
  TimestampTz ts = TimestampTzUtil::FromTimeT(t, "UTC");

  EXPECT_EQ(t, ts.seconds());
  EXPECT_EQ("UTC", ts.timezone());

  time_t t2 = TimestampTzUtil::ToTimeT(ts);
  EXPECT_EQ(t, t2);
}

TEST(TimestampTzUtilTest, NanosecondHandling) {
  TimestampTz ts;
  ts.set_seconds(0);
  ts.set_nanos(999999999);

  Duration d;
  d.set_seconds(0);
  d.set_nanos(1);

  // Adding 1 nanosecond should roll over to next second
  ts += d;
  EXPECT_EQ(1, ts.seconds());
  EXPECT_EQ(0, ts.nanos());
}

TEST(TimestampTzUtilTest, StreamOutput) {
  TimestampTz ts;
  TimestampTzUtil::FromString("2024-01-15T10:30:00-05:00[America/New_York]", &ts);

  std::ostringstream oss;
  oss << ts;
  std::string output = oss.str();
  EXPECT_TRUE(output.find("2024-01-15") != std::string::npos);
  EXPECT_TRUE(output.find("America/New_York") != std::string::npos);
}

}  // namespace

}  // namespace util
}  // namespace protobuf
}  // namespace google
