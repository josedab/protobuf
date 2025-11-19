// Protocol Buffers - Google's data interchange format
// Copyright 2024 Google LLC.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "doc_generator/doc_comment_parser.h"

#include <gtest/gtest.h>

namespace google {
namespace protobuf {
namespace doc_generator {
namespace {

TEST(DocCommentParserTest, ParseSimpleComment) {
  DocCommentParser parser;
  auto result = parser.Parse("This is a simple comment");

  EXPECT_EQ(result.description, "This is a simple comment");
  EXPECT_TRUE(result.since_version.empty());
  EXPECT_TRUE(result.deprecated_version.empty());
}

TEST(DocCommentParserTest, ParseDocComment) {
  DocCommentParser parser;
  auto result = parser.Parse(R"(/**
 * This is a doc comment.
 *
 * It has multiple lines.
 */
)");

  EXPECT_TRUE(result.is_doc_comment);
  EXPECT_FALSE(result.description.empty());
  EXPECT_NE(result.description.find("doc comment"), std::string::npos);
}

TEST(DocCommentParserTest, ParseSinceAnnotation) {
  DocCommentParser parser;
  auto result = parser.Parse(R"(
Description of the element.

@since v1.0.0
)");

  EXPECT_EQ(result.since_version, "v1.0.0");
  EXPECT_NE(result.description.find("Description"), std::string::npos);
}

TEST(DocCommentParserTest, ParseDeprecatedAnnotation) {
  DocCommentParser parser;
  auto result = parser.Parse(R"(
Old element.

@deprecated v3.0.0 Use NewElement instead
)");

  EXPECT_EQ(result.deprecated_version, "v3.0.0");
  EXPECT_EQ(result.deprecated_reason, "Use NewElement instead");
}

TEST(DocCommentParserTest, ParseAuthorAnnotation) {
  DocCommentParser parser;
  auto result = parser.Parse(R"(
Test element.

@author team-platform
)");

  EXPECT_EQ(result.author, "team-platform");
}

TEST(DocCommentParserTest, ParseSeeAnnotation) {
  DocCommentParser parser;
  auto result = parser.Parse(R"(
Test element.

@see OtherMessage
@see AnotherService
)");

  ASSERT_EQ(result.see_also.size(), 2);
  EXPECT_EQ(result.see_also[0], "OtherMessage");
  EXPECT_EQ(result.see_also[1], "AnotherService");
}

TEST(DocCommentParserTest, ParseFieldAnnotations) {
  DocCommentParser parser;
  auto result = parser.Parse(R"(
Email field.

@format email
@required
@example "user@example.com"
@minLength 5
@maxLength 100
)");

  EXPECT_EQ(result.format, "email");
  EXPECT_TRUE(result.required);
  EXPECT_EQ(result.example, "\"user@example.com\"");
  EXPECT_EQ(result.min_length, 5);
  EXPECT_EQ(result.max_length, 100);
}

TEST(DocCommentParserTest, ParseNumericConstraints) {
  DocCommentParser parser;
  auto result = parser.Parse(R"(
Age field.

@min 0
@max 150
)");

  EXPECT_EQ(result.min_value, 0);
  EXPECT_EQ(result.max_value, 150);
}

TEST(DocCommentParserTest, ParsePatternAnnotation) {
  DocCommentParser parser;
  auto result = parser.Parse(R"(
Phone number.

@pattern "^\\+[1-9]\\d{1,14}$"
)");

  EXPECT_EQ(result.pattern, "\"^\\+[1-9]\\d{1,14}$\"");
}

TEST(DocCommentParserTest, ParseCodeBlock) {
  DocCommentParser parser;
  auto result = parser.Parse(R"(
Description.

```protobuf
message Example {
  string name = 1;
}
```
)");

  ASSERT_EQ(result.examples.size(), 1);
  EXPECT_EQ(result.examples[0].language, "protobuf");
  EXPECT_NE(result.examples[0].code.find("message Example"),
            std::string::npos);
}

TEST(DocCommentParserTest, ParseMultipleCodeBlocks) {
  DocCommentParser parser;
  auto result = parser.Parse(R"(
Examples in different languages.

```python
user = User(name="Alice")
```

```java
User user = User.newBuilder().setName("Alice").build();
```
)");

  ASSERT_EQ(result.examples.size(), 2);
  EXPECT_EQ(result.examples[0].language, "python");
  EXPECT_EQ(result.examples[1].language, "java");
}

TEST(DocCommentParserTest, ParseCrossReferences) {
  DocCommentParser parser;
  auto result = parser.Parse(R"(
See {@link OtherMessage} for details.
Use {@link UserService.GetUser} to retrieve.
Related to {@link User#email}.
)");

  ASSERT_EQ(result.cross_references.size(), 3);

  EXPECT_EQ(result.cross_references[0].target, "OtherMessage");
  EXPECT_EQ(result.cross_references[0].type, CrossReference::MESSAGE);

  EXPECT_EQ(result.cross_references[1].target, "UserService.GetUser");
  EXPECT_EQ(result.cross_references[1].type, CrossReference::METHOD);

  EXPECT_EQ(result.cross_references[2].target, "User#email");
  EXPECT_EQ(result.cross_references[2].type, CrossReference::FIELD);
}

TEST(DocCommentParserTest, ParseLocalFieldReference) {
  DocCommentParser parser;
  auto result = parser.Parse("Related to {@link #local_field}.");

  ASSERT_EQ(result.cross_references.size(), 1);
  EXPECT_EQ(result.cross_references[0].target, "#local_field");
  EXPECT_EQ(result.cross_references[0].type, CrossReference::LOCAL_FIELD);
}

TEST(DocCommentParserTest, ParseThrowsAnnotation) {
  DocCommentParser parser;
  auto result = parser.Parse(R"(
Get user by ID.

@throws NOT_FOUND if user doesn't exist
@throws PERMISSION_DENIED if not authorized
)");

  ASSERT_EQ(result.throws.size(), 2);
  EXPECT_NE(result.throws[0].find("NOT_FOUND"), std::string::npos);
  EXPECT_NE(result.throws[1].find("PERMISSION_DENIED"), std::string::npos);
}

TEST(DocCommentParserTest, ParseTripleSlashComment) {
  DocCommentParser parser;
  auto result = parser.Parse(R"(/// This is a triple slash comment.
/// It continues here.
)");

  EXPECT_TRUE(result.is_doc_comment);
  EXPECT_NE(result.description.find("triple slash"), std::string::npos);
}

TEST(DocCommentParserTest, MarkdownToHtmlHeaders) {
  std::string markdown = "# Header 1\n## Header 2\n### Header 3";
  std::string html = DocCommentParser::MarkdownToHtml(markdown);

  EXPECT_NE(html.find("<h1>Header 1</h1>"), std::string::npos);
  EXPECT_NE(html.find("<h2>Header 2</h2>"), std::string::npos);
  EXPECT_NE(html.find("<h3>Header 3</h3>"), std::string::npos);
}

TEST(DocCommentParserTest, MarkdownToHtmlBoldItalic) {
  std::string markdown = "This is **bold** and *italic*.";
  std::string html = DocCommentParser::MarkdownToHtml(markdown);

  EXPECT_NE(html.find("<strong>bold</strong>"), std::string::npos);
  EXPECT_NE(html.find("<em>italic</em>"), std::string::npos);
}

TEST(DocCommentParserTest, MarkdownToHtmlInlineCode) {
  std::string markdown = "Use `code` here.";
  std::string html = DocCommentParser::MarkdownToHtml(markdown);

  EXPECT_NE(html.find("<code>code</code>"), std::string::npos);
}

TEST(DocCommentParserTest, EscapeHtml) {
  std::string text = "<script>alert('xss')</script>";
  std::string escaped = DocCommentParser::EscapeHtml(text);

  EXPECT_NE(escaped.find("&lt;"), std::string::npos);
  EXPECT_NE(escaped.find("&gt;"), std::string::npos);
  EXPECT_EQ(escaped.find("<script>"), std::string::npos);
}

TEST(DocCommentParserTest, Dedent) {
  std::string text = "    line 1\n    line 2\n    line 3";
  std::string dedented = DocCommentParser::Dedent(text);

  EXPECT_EQ(dedented[0], 'l');  // Should start with 'l' not space
}

TEST(DocCommentParserTest, ParseComplexDocComment) {
  DocCommentParser parser;
  auto result = parser.Parse(R"(/**
 * User represents a person in the system.
 *
 * Users are created via the {@link UserService.CreateUser} RPC and can be
 * retrieved using their {@link #uuid_id}.
 *
 * ## Lifecycle
 *
 * 1. Created with `CreateUser`
 * 2. Updated with `UpdateUser`
 * 3. Deleted with `DeleteUser`
 *
 * @example
 * ```protobuf
 * User user = {
 *   uuid_id: "550e8400-e29b-41d4-a716-446655440000",
 *   email: "user@example.com",
 *   name: "Alice Smith"
 * };
 * ```
 *
 * @see UserService
 * @since v1.0.0
 * @deprecated v3.0.0 Use UserV2 instead
 */
)");

  EXPECT_TRUE(result.is_doc_comment);
  EXPECT_NE(result.description.find("represents a person"), std::string::npos);
  EXPECT_EQ(result.since_version, "v1.0.0");
  EXPECT_EQ(result.deprecated_version, "v3.0.0");
  EXPECT_EQ(result.deprecated_reason, "Use UserV2 instead");
  ASSERT_GE(result.see_also.size(), 1);
  EXPECT_EQ(result.see_also[0], "UserService");
  EXPECT_GE(result.cross_references.size(), 2);
}

TEST(DocCommentParserTest, ParseDefaultAnnotation) {
  DocCommentParser parser;
  auto result = parser.Parse(R"(
Optional timeout.

@default "30s"
)");

  EXPECT_EQ(result.default_value, "\"30s\"");
}

TEST(DocCommentParserTest, EmptyComment) {
  DocCommentParser parser;
  auto result = parser.Parse("");

  EXPECT_TRUE(result.description.empty());
  EXPECT_FALSE(result.is_doc_comment);
}

TEST(DocCommentParserTest, CommentWithOnlyWhitespace) {
  DocCommentParser parser;
  auto result = parser.Parse("   \n\t\n   ");

  EXPECT_TRUE(result.description.empty());
}

}  // namespace
}  // namespace doc_generator
}  // namespace protobuf
}  // namespace google
