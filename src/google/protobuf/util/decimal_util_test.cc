// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/util/decimal_util.h"

#include <cstdint>

#include "google/protobuf/decimal.pb.h"
#include "google/protobuf/testing/googletest.h"
#include <gtest/gtest.h>

namespace google {
namespace protobuf {
namespace util {

using google::protobuf::Decimal;

namespace {

TEST(DecimalUtilTest, StringConversion) {
  Decimal decimal;

  // Test basic decimal
  EXPECT_TRUE(DecimalUtil::FromString("123.45", &decimal));
  EXPECT_EQ("123.45", DecimalUtil::ToString(decimal));

  // Test integer
  EXPECT_TRUE(DecimalUtil::FromString("12345", &decimal));
  EXPECT_EQ("12345", DecimalUtil::ToString(decimal));

  // Test with leading zeros after decimal
  EXPECT_TRUE(DecimalUtil::FromString("0.001", &decimal));
  EXPECT_EQ("0.001", DecimalUtil::ToString(decimal));

  // Test negative
  EXPECT_TRUE(DecimalUtil::FromString("-123.45", &decimal));
  EXPECT_EQ("-123.45", DecimalUtil::ToString(decimal));

  // Test zero
  EXPECT_TRUE(DecimalUtil::FromString("0", &decimal));
  EXPECT_TRUE(DecimalUtil::IsZero(decimal));

  // Test invalid strings
  EXPECT_FALSE(DecimalUtil::FromString("", &decimal));
  EXPECT_FALSE(DecimalUtil::FromString("abc", &decimal));
  EXPECT_FALSE(DecimalUtil::FromString("12.34.56", &decimal));
}

TEST(DecimalUtilTest, ZeroAndSign) {
  Decimal zero = DecimalUtil::Zero();
  EXPECT_TRUE(DecimalUtil::IsZero(zero));
  EXPECT_FALSE(DecimalUtil::IsNegative(zero));
  EXPECT_EQ(0, DecimalUtil::Sign(zero));

  Decimal positive;
  DecimalUtil::FromString("42", &positive);
  EXPECT_FALSE(DecimalUtil::IsZero(positive));
  EXPECT_FALSE(DecimalUtil::IsNegative(positive));
  EXPECT_EQ(1, DecimalUtil::Sign(positive));

  Decimal negative;
  DecimalUtil::FromString("-42", &negative);
  EXPECT_FALSE(DecimalUtil::IsZero(negative));
  EXPECT_TRUE(DecimalUtil::IsNegative(negative));
  EXPECT_EQ(-1, DecimalUtil::Sign(negative));
}

TEST(DecimalUtilTest, DoubleConversion) {
  Decimal decimal;
  DecimalUtil::FromString("123.45", &decimal);
  EXPECT_DOUBLE_EQ(123.45, DecimalUtil::ToDouble(decimal));

  DecimalUtil::FromDouble(99.99, 2, &decimal);
  EXPECT_EQ("99.99", DecimalUtil::ToString(decimal));
}

TEST(DecimalUtilTest, Int64Conversion) {
  Decimal decimal;
  DecimalUtil::FromInt64(12345, &decimal);
  EXPECT_EQ(12345, DecimalUtil::ToInt64(decimal));

  DecimalUtil::FromInt64(-12345, &decimal);
  EXPECT_EQ(-12345, DecimalUtil::ToInt64(decimal));

  DecimalUtil::FromInt64(0, &decimal);
  EXPECT_EQ(0, DecimalUtil::ToInt64(decimal));
  EXPECT_TRUE(DecimalUtil::IsZero(decimal));
}

TEST(DecimalUtilTest, Arithmetic) {
  Decimal a, b;
  DecimalUtil::FromString("10.5", &a);
  DecimalUtil::FromString("3.2", &b);

  // Addition
  Decimal sum = DecimalUtil::Add(a, b);
  EXPECT_DOUBLE_EQ(13.7, DecimalUtil::ToDouble(sum));

  // Subtraction
  Decimal diff = DecimalUtil::Subtract(a, b);
  EXPECT_DOUBLE_EQ(7.3, DecimalUtil::ToDouble(diff));

  // Multiplication
  Decimal product = DecimalUtil::Multiply(a, b);
  EXPECT_DOUBLE_EQ(33.6, DecimalUtil::ToDouble(product));

  // Division
  Decimal quotient = DecimalUtil::Divide(a, b, 4);
  EXPECT_NEAR(3.28125, DecimalUtil::ToDouble(quotient), 0.0001);
}

TEST(DecimalUtilTest, UnaryOperations) {
  Decimal decimal;
  DecimalUtil::FromString("42.5", &decimal);

  // Negate
  Decimal neg = DecimalUtil::Negate(decimal);
  EXPECT_EQ("-42.5", DecimalUtil::ToString(neg));

  // Abs
  Decimal abs = DecimalUtil::Abs(neg);
  EXPECT_EQ("42.5", DecimalUtil::ToString(abs));
}

TEST(DecimalUtilTest, Comparison) {
  Decimal a, b, c;
  DecimalUtil::FromString("10.5", &a);
  DecimalUtil::FromString("20.5", &b);
  DecimalUtil::FromString("10.5", &c);

  EXPECT_EQ(0, DecimalUtil::Compare(a, c));
  EXPECT_LT(DecimalUtil::Compare(a, b), 0);
  EXPECT_GT(DecimalUtil::Compare(b, a), 0);

  // Operator overloads
  EXPECT_EQ(a, c);
  EXPECT_NE(a, b);
  EXPECT_LT(a, b);
  EXPECT_GT(b, a);
  EXPECT_LE(a, c);
  EXPECT_GE(a, c);
}

TEST(DecimalUtilTest, OperatorOverloads) {
  Decimal a, b;
  DecimalUtil::FromString("10", &a);
  DecimalUtil::FromString("3", &b);

  Decimal sum = a + b;
  EXPECT_DOUBLE_EQ(13.0, DecimalUtil::ToDouble(sum));

  Decimal diff = a - b;
  EXPECT_DOUBLE_EQ(7.0, DecimalUtil::ToDouble(diff));

  Decimal product = a * b;
  EXPECT_DOUBLE_EQ(30.0, DecimalUtil::ToDouble(product));

  Decimal neg = -a;
  EXPECT_DOUBLE_EQ(-10.0, DecimalUtil::ToDouble(neg));
}

TEST(DecimalUtilTest, Rounding) {
  Decimal decimal;
  DecimalUtil::FromString("123.456", &decimal);

  Decimal rounded = DecimalUtil::Round(decimal, 2);
  EXPECT_DOUBLE_EQ(123.46, DecimalUtil::ToDouble(rounded));

  Decimal truncated = DecimalUtil::Truncate(decimal, 2);
  EXPECT_DOUBLE_EQ(123.45, DecimalUtil::ToDouble(truncated));
}

TEST(DecimalUtilTest, StreamOutput) {
  Decimal decimal;
  DecimalUtil::FromString("123.45", &decimal);

  std::ostringstream oss;
  oss << decimal;
  EXPECT_EQ("123.45", oss.str());
}

TEST(DecimalUtilTest, CurrencyCalculation) {
  // Example from RFC: currency calculations without precision loss
  Decimal price, quantity, total;
  DecimalUtil::FromString("19.99", &price);
  DecimalUtil::FromString("3", &quantity);

  total = DecimalUtil::Multiply(price, quantity);
  EXPECT_DOUBLE_EQ(59.97, DecimalUtil::ToDouble(total));
}

}  // namespace

}  // namespace util
}  // namespace protobuf
}  // namespace google
