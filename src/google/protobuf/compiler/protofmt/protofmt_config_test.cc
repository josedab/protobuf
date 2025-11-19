// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/compiler/protofmt/protofmt_config.h"

#include <string>

#include "gtest/gtest.h"
#include "absl/strings/str_cat.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace protofmt {
namespace {

TEST(ProtofmtConfigTest, DefaultValues) {
  ProtofmtConfig config = ProtofmtConfig::Default();

  EXPECT_EQ(config.indent, 2);
  EXPECT_FALSE(config.align_fields);
  EXPECT_TRUE(config.sort_imports);
  EXPECT_EQ(config.max_line_length, 100);
  EXPECT_TRUE(config.preserve_comments);
  EXPECT_FALSE(config.trailing_commas);
}

TEST(ProtofmtConfigTest, LoadFromStringBasic) {
  std::string yaml = R"yaml(
indent: 4
align_fields: true
sort_imports: false
max_line_length: 80
preserve_comments: false
trailing_commas: true
)yaml";

  auto config_or = ProtofmtConfig::LoadFromString(yaml);
  ASSERT_TRUE(config_or.ok()) << config_or.status().message();

  ProtofmtConfig config = config_or.value();
  EXPECT_EQ(config.indent, 4);
  EXPECT_TRUE(config.align_fields);
  EXPECT_FALSE(config.sort_imports);
  EXPECT_EQ(config.max_line_length, 80);
  EXPECT_FALSE(config.preserve_comments);
  EXPECT_TRUE(config.trailing_commas);
}

TEST(ProtofmtConfigTest, LoadFromStringPartial) {
  std::string yaml = R"yaml(
indent: 4
)yaml";

  auto config_or = ProtofmtConfig::LoadFromString(yaml);
  ASSERT_TRUE(config_or.ok()) << config_or.status().message();

  ProtofmtConfig config = config_or.value();
  EXPECT_EQ(config.indent, 4);
  // Other values should be defaults
  EXPECT_FALSE(config.align_fields);
  EXPECT_TRUE(config.sort_imports);
  EXPECT_EQ(config.max_line_length, 100);
}

TEST(ProtofmtConfigTest, LoadFromStringWithComments) {
  std::string yaml = R"yaml(
# This is a comment
indent: 4
# Another comment
align_fields: true
)yaml";

  auto config_or = ProtofmtConfig::LoadFromString(yaml);
  ASSERT_TRUE(config_or.ok()) << config_or.status().message();

  ProtofmtConfig config = config_or.value();
  EXPECT_EQ(config.indent, 4);
  EXPECT_TRUE(config.align_fields);
}

TEST(ProtofmtConfigTest, LoadFromStringBooleanValues) {
  // Test "yes" as boolean
  std::string yaml1 = "align_fields: yes";
  auto config_or1 = ProtofmtConfig::LoadFromString(yaml1);
  ASSERT_TRUE(config_or1.ok());
  EXPECT_TRUE(config_or1.value().align_fields);

  // Test "true" as boolean
  std::string yaml2 = "align_fields: true";
  auto config_or2 = ProtofmtConfig::LoadFromString(yaml2);
  ASSERT_TRUE(config_or2.ok());
  EXPECT_TRUE(config_or2.value().align_fields);

  // Test "false" as boolean
  std::string yaml3 = "align_fields: false";
  auto config_or3 = ProtofmtConfig::LoadFromString(yaml3);
  ASSERT_TRUE(config_or3.ok());
  EXPECT_FALSE(config_or3.value().align_fields);
}

TEST(ProtofmtConfigTest, LoadFromStringInvalidIndent) {
  std::string yaml = "indent: invalid";
  auto config_or = ProtofmtConfig::LoadFromString(yaml);
  EXPECT_FALSE(config_or.ok());
}

TEST(ProtofmtConfigTest, LoadFromStringIndentOutOfRange) {
  // Too small
  std::string yaml1 = "indent: 0";
  auto config_or1 = ProtofmtConfig::LoadFromString(yaml1);
  EXPECT_FALSE(config_or1.ok());

  // Too large
  std::string yaml2 = "indent: 10";
  auto config_or2 = ProtofmtConfig::LoadFromString(yaml2);
  EXPECT_FALSE(config_or2.ok());
}

TEST(ProtofmtConfigTest, LoadFromStringMaxLineLengthOutOfRange) {
  // Too small
  std::string yaml1 = "max_line_length: 30";
  auto config_or1 = ProtofmtConfig::LoadFromString(yaml1);
  EXPECT_FALSE(config_or1.ok());

  // Too large
  std::string yaml2 = "max_line_length: 600";
  auto config_or2 = ProtofmtConfig::LoadFromString(yaml2);
  EXPECT_FALSE(config_or2.ok());
}

TEST(ProtofmtConfigTest, LoadFromStringUnknownKey) {
  // Unknown keys should be ignored for forward compatibility
  std::string yaml = R"yaml(
indent: 4
unknown_key: some_value
align_fields: true
)yaml";

  auto config_or = ProtofmtConfig::LoadFromString(yaml);
  ASSERT_TRUE(config_or.ok()) << config_or.status().message();

  ProtofmtConfig config = config_or.value();
  EXPECT_EQ(config.indent, 4);
  EXPECT_TRUE(config.align_fields);
}

TEST(ProtofmtConfigTest, LoadFromStringEmptyContent) {
  std::string yaml = "";
  auto config_or = ProtofmtConfig::LoadFromString(yaml);
  ASSERT_TRUE(config_or.ok()) << config_or.status().message();

  // Should return defaults
  ProtofmtConfig config = config_or.value();
  EXPECT_EQ(config.indent, 2);
  EXPECT_FALSE(config.align_fields);
}

TEST(ProtofmtConfigTest, LoadFromStringOnlyComments) {
  std::string yaml = R"yaml(
# Just a comment
# Another comment
)yaml";

  auto config_or = ProtofmtConfig::LoadFromString(yaml);
  ASSERT_TRUE(config_or.ok()) << config_or.status().message();

  // Should return defaults
  ProtofmtConfig config = config_or.value();
  EXPECT_EQ(config.indent, 2);
  EXPECT_FALSE(config.align_fields);
}

TEST(ProtofmtConfigTest, LoadFromStringWhitespaceHandling) {
  std::string yaml = "  indent  :  4  ";
  auto config_or = ProtofmtConfig::LoadFromString(yaml);
  ASSERT_TRUE(config_or.ok()) << config_or.status().message();

  EXPECT_EQ(config_or.value().indent, 4);
}

TEST(ProtofmtConfigTest, LoadFromFileNotFound) {
  auto config_or = ProtofmtConfig::LoadFromFile("/nonexistent/file.yaml");
  EXPECT_FALSE(config_or.ok());
  EXPECT_TRUE(absl::IsNotFound(config_or.status()));
}

}  // namespace
}  // namespace protofmt
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
