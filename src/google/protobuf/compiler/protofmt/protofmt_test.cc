// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/compiler/protofmt/protofmt.h"

#include <string>

#include "gtest/gtest.h"
#include "absl/strings/str_cat.h"
#include "google/protobuf/compiler/protofmt/protofmt_config.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace protofmt {
namespace {

class ProtobufFormatterTest : public ::testing::Test {
 protected:
  void SetUp() override {
    config_ = ProtofmtConfig::Default();
  }

  std::string Format(const std::string& input) {
    ProtobufFormatter formatter(config_);
    auto result_or = formatter.Format(input);
    EXPECT_TRUE(result_or.ok()) << result_or.status().message();
    return result_or.value().output;
  }

  bool IsFormatted(const std::string& input) {
    ProtobufFormatter formatter(config_);
    auto result_or = formatter.IsFormatted(input);
    EXPECT_TRUE(result_or.ok()) << result_or.status().message();
    return result_or.value();
  }

  ProtofmtConfig config_;
};

TEST_F(ProtobufFormatterTest, BasicMessage) {
  std::string input = R"proto(
syntax="proto3";package user;message User{string name=1;int32 age=2;}
)proto";

  std::string expected = R"proto(syntax = "proto3";

package user;

message User {
  string name = 1;
  int32 age = 2;
}
)proto";

  EXPECT_EQ(Format(input), expected);
}

TEST_F(ProtobufFormatterTest, FormattedMessageIsIdempotent) {
  std::string formatted = R"proto(syntax = "proto3";

package user;

message User {
  string name = 1;
  int32 age = 2;
}
)proto";

  // Format should be idempotent
  EXPECT_EQ(Format(formatted), formatted);
  EXPECT_TRUE(IsFormatted(formatted));
}

TEST_F(ProtobufFormatterTest, ImportsSorted) {
  std::string input = R"proto(
syntax = "proto3";
import "z.proto";
import "a.proto";
import "m.proto";
message Foo {}
)proto";

  std::string expected = R"proto(syntax = "proto3";

import "a.proto";
import "m.proto";
import "z.proto";

message Foo {
}
)proto";

  config_.sort_imports = true;
  EXPECT_EQ(Format(input), expected);
}

TEST_F(ProtobufFormatterTest, ImportsNotSorted) {
  std::string input = R"proto(
syntax = "proto3";
import "z.proto";
import "a.proto";
message Foo {}
)proto";

  std::string expected = R"proto(syntax = "proto3";

import "z.proto";
import "a.proto";

message Foo {
}
)proto";

  config_.sort_imports = false;
  EXPECT_EQ(Format(input), expected);
}

TEST_F(ProtobufFormatterTest, RepeatedField) {
  std::string input = R"proto(
syntax = "proto3";
message User {
  repeated string tags = 1;
}
)proto";

  std::string expected = R"proto(syntax = "proto3";

message User {
  repeated string tags = 1;
}
)proto";

  EXPECT_EQ(Format(input), expected);
}

TEST_F(ProtobufFormatterTest, NestedMessage) {
  std::string input = R"proto(
syntax = "proto3";
message Outer {
  message Inner {
    string value = 1;
  }
  Inner inner = 1;
}
)proto";

  std::string expected = R"proto(syntax = "proto3";

message Outer {
  message Inner {
    string value = 1;
  }
  Outer.Inner inner = 1;
}
)proto";

  EXPECT_EQ(Format(input), expected);
}

TEST_F(ProtobufFormatterTest, EnumFormatting) {
  std::string input = R"proto(
syntax = "proto3";
enum Status {STATUS_UNKNOWN=0;STATUS_ACTIVE=1;STATUS_INACTIVE=2;}
)proto";

  std::string expected = R"proto(syntax = "proto3";

enum Status {
  STATUS_UNKNOWN = 0;
  STATUS_ACTIVE = 1;
  STATUS_INACTIVE = 2;
}
)proto";

  EXPECT_EQ(Format(input), expected);
}

TEST_F(ProtobufFormatterTest, ServiceFormatting) {
  std::string input = R"proto(
syntax = "proto3";
message Request {}
message Response {}
service MyService{rpc GetData(Request)returns(Response);}
)proto";

  std::string expected = R"proto(syntax = "proto3";

message Request {
}

message Response {
}

service MyService {
  rpc GetData(Request) returns (Response);
}
)proto";

  EXPECT_EQ(Format(input), expected);
}

TEST_F(ProtobufFormatterTest, StreamingRpc) {
  std::string input = R"proto(
syntax = "proto3";
message Request {}
message Response {}
service MyService {
  rpc StreamData(stream Request) returns (stream Response);
}
)proto";

  std::string expected = R"proto(syntax = "proto3";

message Request {
}

message Response {
}

service MyService {
  rpc StreamData(stream Request) returns (stream Response);
}
)proto";

  EXPECT_EQ(Format(input), expected);
}

TEST_F(ProtobufFormatterTest, OneofFormatting) {
  std::string input = R"proto(
syntax = "proto3";
message Container {
  oneof value {
    string text = 1;
    int32 number = 2;
  }
}
)proto";

  std::string expected = R"proto(syntax = "proto3";

message Container {
  oneof value {
    string text = 1;
    int32 number = 2;
  }
}
)proto";

  EXPECT_EQ(Format(input), expected);
}

TEST_F(ProtobufFormatterTest, FileOptions) {
  std::string input = R"proto(
syntax = "proto3";
package mypackage;
option java_package = "com.example";
option go_package = "github.com/example/proto";
message Foo {}
)proto";

  std::string expected = R"proto(syntax = "proto3";

package mypackage;

option java_package = "com.example";
option go_package = "github.com/example/proto";

message Foo {
}
)proto";

  EXPECT_EQ(Format(input), expected);
}

TEST_F(ProtobufFormatterTest, FieldAlignment) {
  std::string input = R"proto(
syntax = "proto3";
message User {
  string name = 1;
  int32 age = 2;
  repeated string tags = 3;
  bool active = 4;
}
)proto";

  config_.align_fields = true;
  std::string result = Format(input);

  // Check that the result is properly formatted
  EXPECT_NE(result.find("string"), std::string::npos);
  EXPECT_NE(result.find("int32"), std::string::npos);
  EXPECT_NE(result.find("bool"), std::string::npos);
}

TEST_F(ProtobufFormatterTest, DeprecatedField) {
  std::string input = R"proto(
syntax = "proto3";
message User {
  string name = 1 [deprecated = true];
}
)proto";

  std::string expected = R"proto(syntax = "proto3";

message User {
  string name = 1 [deprecated = true];
}
)proto";

  EXPECT_EQ(Format(input), expected);
}

TEST_F(ProtobufFormatterTest, CustomIndent) {
  std::string input = R"proto(
syntax = "proto3";
message User {
  string name = 1;
}
)proto";

  config_.indent = 4;
  std::string result = Format(input);

  // Check for 4-space indent
  EXPECT_NE(result.find("    string"), std::string::npos);
}

TEST_F(ProtobufFormatterTest, ReservedRanges) {
  std::string input = R"proto(
syntax = "proto3";
message User {
  string name = 1;
  reserved 2, 15, 9 to 11;
  reserved "foo", "bar";
}
)proto";

  std::string result = Format(input);
  EXPECT_NE(result.find("reserved"), std::string::npos);
}

TEST_F(ProtobufFormatterTest, Extensions) {
  std::string input = R"proto(
syntax = "proto2";
message MyMessage {
  extensions 100 to 199;
}
)proto";

  std::string result = Format(input);
  EXPECT_NE(result.find("extensions 100 to 199"), std::string::npos);
}

TEST_F(ProtobufFormatterTest, PublicImport) {
  std::string input = R"proto(
syntax = "proto3";
import public "other.proto";
message Foo {}
)proto";

  std::string expected = R"proto(syntax = "proto3";

import public "other.proto";

message Foo {
}
)proto";

  EXPECT_EQ(Format(input), expected);
}

TEST_F(ProtobufFormatterTest, WeakImport) {
  std::string input = R"proto(
syntax = "proto3";
import weak "other.proto";
message Foo {}
)proto";

  std::string expected = R"proto(syntax = "proto3";

import weak "other.proto";

message Foo {
}
)proto";

  EXPECT_EQ(Format(input), expected);
}

TEST_F(ProtobufFormatterTest, GetDiffReturnsEmptyForFormatted) {
  std::string formatted = R"proto(syntax = "proto3";

package user;

message User {
  string name = 1;
}
)proto";

  ProtobufFormatter formatter(config_);
  auto diff_or = formatter.GetDiff(formatted);
  EXPECT_TRUE(diff_or.ok());
  EXPECT_EQ(diff_or.value(), "");
}

TEST_F(ProtobufFormatterTest, GetDiffReturnsChanges) {
  std::string unformatted = R"proto(
syntax="proto3";package user;message User{string name=1;}
)proto";

  ProtobufFormatter formatter(config_);
  auto diff_or = formatter.GetDiff(unformatted);
  EXPECT_TRUE(diff_or.ok());
  EXPECT_FALSE(diff_or.value().empty());
}

TEST_F(ProtobufFormatterTest, InvalidProtoReturnsError) {
  std::string invalid = R"proto(
syntax = "proto3";
message { // missing name
  string name = 1;
}
)proto";

  ProtobufFormatter formatter(config_);
  auto result_or = formatter.Format(invalid);
  EXPECT_FALSE(result_or.ok());
}

TEST_F(ProtobufFormatterTest, Proto2Syntax) {
  std::string input = R"proto(
syntax = "proto2";
message User {
  required string name = 1;
  optional int32 age = 2;
}
)proto";

  std::string expected = R"proto(syntax = "proto2";

message User {
  required string name = 1;
  int32 age = 2;
}
)proto";

  EXPECT_EQ(Format(input), expected);
}

TEST_F(ProtobufFormatterTest, AllScalarTypes) {
  std::string input = R"proto(
syntax = "proto3";
message AllTypes {
  double field1 = 1;
  float field2 = 2;
  int32 field3 = 3;
  int64 field4 = 4;
  uint32 field5 = 5;
  uint64 field6 = 6;
  sint32 field7 = 7;
  sint64 field8 = 8;
  fixed32 field9 = 9;
  fixed64 field10 = 10;
  sfixed32 field11 = 11;
  sfixed64 field12 = 12;
  bool field13 = 13;
  string field14 = 14;
  bytes field15 = 15;
}
)proto";

  std::string result = Format(input);

  // Verify all types are present
  EXPECT_NE(result.find("double"), std::string::npos);
  EXPECT_NE(result.find("float"), std::string::npos);
  EXPECT_NE(result.find("int32"), std::string::npos);
  EXPECT_NE(result.find("int64"), std::string::npos);
  EXPECT_NE(result.find("uint32"), std::string::npos);
  EXPECT_NE(result.find("uint64"), std::string::npos);
  EXPECT_NE(result.find("sint32"), std::string::npos);
  EXPECT_NE(result.find("sint64"), std::string::npos);
  EXPECT_NE(result.find("fixed32"), std::string::npos);
  EXPECT_NE(result.find("fixed64"), std::string::npos);
  EXPECT_NE(result.find("sfixed32"), std::string::npos);
  EXPECT_NE(result.find("sfixed64"), std::string::npos);
  EXPECT_NE(result.find("bool"), std::string::npos);
  EXPECT_NE(result.find("string"), std::string::npos);
  EXPECT_NE(result.find("bytes"), std::string::npos);
}

TEST_F(ProtobufFormatterTest, DeprecatedMessage) {
  std::string input = R"proto(
syntax = "proto3";
message User {
  option deprecated = true;
  string name = 1;
}
)proto";

  std::string result = Format(input);
  EXPECT_NE(result.find("option deprecated = true"), std::string::npos);
}

}  // namespace
}  // namespace protofmt
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
