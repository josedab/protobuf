// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Unit tests for zero-copy parsing functionality.

#include "google/protobuf/zero_copy/zero_copy.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>

#include <gtest/gtest.h>
#include "absl/status/status.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"

namespace google {
namespace protobuf {
namespace zero_copy {
namespace {

// =============================================================================
// ParseBuffer Tests
// =============================================================================

TEST(ParseBufferTest, CreateOwned) {
  std::string data = "Hello, World!";
  auto buffer = ParseBuffer::Create(data);

  EXPECT_NE(buffer, nullptr);
  EXPECT_TRUE(buffer->IsValid());
  EXPECT_EQ(buffer->Data(), "Hello, World!");
  EXPECT_EQ(buffer->Size(), 13);
}

TEST(ParseBufferTest, CreateOwnedMoved) {
  std::string data = "Hello, World!";
  auto buffer = ParseBuffer::Create(std::move(data));

  EXPECT_NE(buffer, nullptr);
  EXPECT_TRUE(buffer->IsValid());
  EXPECT_EQ(buffer->Data(), "Hello, World!");
  EXPECT_EQ(buffer->Size(), 13);
}

TEST(ParseBufferTest, Wrap) {
  std::string data = "Hello, World!";
  auto buffer = ParseBuffer::Wrap(data.data(), data.size());

  EXPECT_NE(buffer, nullptr);
  EXPECT_TRUE(buffer->IsValid());
  EXPECT_EQ(buffer->Data(), "Hello, World!");
  EXPECT_EQ(buffer->Size(), 13);
}

TEST(ParseBufferTest, WrapEmpty) {
  auto buffer = ParseBuffer::Wrap("", 0);

  EXPECT_NE(buffer, nullptr);
  EXPECT_TRUE(buffer->IsValid());
  EXPECT_TRUE(buffer->Data().empty());
  EXPECT_EQ(buffer->Size(), 0);
}

TEST(ParseBufferTest, Substr) {
  std::string data = "Hello, World!";
  auto buffer = ParseBuffer::Create(data);

  EXPECT_EQ(buffer->Substr(0, 5), "Hello");
  EXPECT_EQ(buffer->Substr(7, 5), "World");
  EXPECT_EQ(buffer->Substr(0, 100), "Hello, World!");  // Beyond end
  EXPECT_EQ(buffer->Substr(100, 5), "");  // Beyond buffer
}

TEST(ParseBufferTest, Bytes) {
  std::string data = "ABC";
  auto buffer = ParseBuffer::Create(data);

  auto bytes = buffer->Bytes();
  EXPECT_EQ(bytes.size(), 3);
  EXPECT_EQ(bytes[0], 'A');
  EXPECT_EQ(bytes[1], 'B');
  EXPECT_EQ(bytes[2], 'C');
}

TEST(ParseBufferTest, BytesFrom) {
  std::string data = "Hello";
  auto buffer = ParseBuffer::Create(data);

  auto bytes = buffer->BytesFrom(1, 3);
  EXPECT_EQ(bytes.size(), 3);
  EXPECT_EQ(bytes[0], 'e');
  EXPECT_EQ(bytes[1], 'l');
  EXPECT_EQ(bytes[2], 'l');
}

TEST(ParseBufferTest, Invalidate) {
  auto buffer = ParseBuffer::Create("Test");
  EXPECT_TRUE(buffer->IsValid());

  buffer->Invalidate();
  EXPECT_FALSE(buffer->IsValid());
}

TEST(ParseBufferTest, SharedOwnership) {
  auto buffer = ParseBuffer::Create("Test data");
  auto buffer2 = buffer;  // Shared copy

  EXPECT_EQ(buffer.use_count(), 2);
  EXPECT_EQ(buffer->Data(), buffer2->Data());
  EXPECT_EQ(buffer->DataPtr(), buffer2->DataPtr());  // Same underlying data
}

// =============================================================================
// ZeroCopyString Tests
// =============================================================================

TEST(ZeroCopyStringTest, DefaultConstructor) {
  ZeroCopyString str;
  EXPECT_TRUE(str.empty());
  EXPECT_EQ(str.size(), 0);
  EXPECT_EQ(str.view(), "");
}

TEST(ZeroCopyStringTest, FromOwnedString) {
  ZeroCopyString str(std::string("Hello"));
  EXPECT_FALSE(str.empty());
  EXPECT_EQ(str.size(), 5);
  EXPECT_EQ(str.view(), "Hello");
  EXPECT_TRUE(str.IsMaterialized());
  EXPECT_FALSE(str.IsZeroCopy());
}

TEST(ZeroCopyStringTest, FromStringView) {
  ZeroCopyString str(absl::string_view("World"));
  EXPECT_EQ(str.view(), "World");
  EXPECT_TRUE(str.IsMaterialized());
}

TEST(ZeroCopyStringTest, FromBuffer) {
  auto buffer = ParseBuffer::Create("Hello, World!");
  ZeroCopyString str(buffer, 0, 5);  // "Hello"

  EXPECT_EQ(str.view(), "Hello");
  EXPECT_TRUE(str.IsZeroCopy());
  EXPECT_FALSE(str.IsMaterialized());
  EXPECT_TRUE(str.IsValid());
}

TEST(ZeroCopyStringTest, FromBufferMiddle) {
  auto buffer = ParseBuffer::Create("Hello, World!");
  ZeroCopyString str(buffer, 7, 5);  // "World"

  EXPECT_EQ(str.view(), "World");
  EXPECT_EQ(str.ToString(), "World");
}

TEST(ZeroCopyStringTest, Materialize) {
  auto buffer = ParseBuffer::Create("Test string");
  ZeroCopyString str(buffer, 0, 4);

  EXPECT_TRUE(str.IsZeroCopy());
  const std::string& mat = str.Materialize();
  EXPECT_EQ(mat, "Test");
  EXPECT_TRUE(str.IsMaterialized());
  EXPECT_FALSE(str.IsZeroCopy());
}

TEST(ZeroCopyStringTest, MaterializeAfterBufferInvalidation) {
  auto buffer = ParseBuffer::Create("Data");
  ZeroCopyString str(buffer, 0, 4);

  // Materialize before invalidation
  std::string copy = str.ToString();
  EXPECT_EQ(copy, "Data");

  // Invalidate buffer
  buffer->Invalidate();

  // After buffer invalidation, view returns empty if not materialized
  // But ToString should still return the materialized copy
  ZeroCopyString str2(buffer, 0, 4);
  EXPECT_FALSE(str2.IsValid());
}

TEST(ZeroCopyStringTest, Comparison) {
  auto buffer = ParseBuffer::Create("Hello");
  ZeroCopyString str(buffer, 0, 5);

  EXPECT_TRUE(str == "Hello");
  EXPECT_FALSE(str == "World");
  EXPECT_FALSE(str != "Hello");
  EXPECT_TRUE(str != "World");
  EXPECT_FALSE(str < "Hello");
  EXPECT_TRUE(str < "World");
}

TEST(ZeroCopyStringTest, Clear) {
  auto buffer = ParseBuffer::Create("Test");
  ZeroCopyString str(buffer, 0, 4);

  str.Clear();
  EXPECT_TRUE(str.empty());
  EXPECT_EQ(str.size(), 0);
}

TEST(ZeroCopyStringTest, CopyAndMove) {
  auto buffer = ParseBuffer::Create("Test data");
  ZeroCopyString str1(buffer, 0, 4);

  // Copy
  ZeroCopyString str2 = str1;
  EXPECT_EQ(str2.view(), "Test");

  // Move
  ZeroCopyString str3 = std::move(str1);
  EXPECT_EQ(str3.view(), "Test");
}

// =============================================================================
// ZeroCopyBytes Tests
// =============================================================================

TEST(ZeroCopyBytesTest, DefaultConstructor) {
  ZeroCopyBytes bytes;
  EXPECT_TRUE(bytes.empty());
  EXPECT_EQ(bytes.size(), 0);
}

TEST(ZeroCopyBytesTest, FromBuffer) {
  auto buffer = ParseBuffer::Create("\x01\x02\x03\x04");
  ZeroCopyBytes bytes(buffer, 1, 2);

  auto span = bytes.span();
  EXPECT_EQ(span.size(), 2);
  EXPECT_EQ(span[0], 0x02);
  EXPECT_EQ(span[1], 0x03);
}

TEST(ZeroCopyBytesTest, Materialize) {
  auto buffer = ParseBuffer::Create("Binary");
  ZeroCopyBytes bytes(buffer, 0, 6);

  const std::string& mat = bytes.Materialize();
  EXPECT_EQ(mat, "Binary");
}

// =============================================================================
// LazyString Tests
// =============================================================================

TEST(LazyStringTest, FromBuffer) {
  auto buffer = ParseBuffer::Create("Lazy test");
  LazyString str(buffer, 0, 4);

  EXPECT_EQ(str.view(), "Lazy");
  EXPECT_EQ(str.size(), 4);
}

TEST(LazyStringTest, StrMaterialize) {
  auto buffer = ParseBuffer::Create("Lazy");
  LazyString str(buffer, 0, 4);

  const std::string& s = str.str();
  EXPECT_EQ(s, "Lazy");
}

TEST(LazyStringTest, MutableStr) {
  auto buffer = ParseBuffer::Create("Edit");
  LazyString str(buffer, 0, 4);

  std::string* mutable_str = str.mutable_str();
  *mutable_str = "Modified";
  EXPECT_EQ(str.str(), "Modified");
}

// =============================================================================
// ZeroCopyParseResult Tests
// =============================================================================

TEST(ZeroCopyParseResultTest, AddAndFindField) {
  auto buffer = ParseBuffer::Create("field1field2");
  ZeroCopyParseResult result(buffer);

  result.AddFieldLocation(1, 0, 6);   // "field1"
  result.AddFieldLocation(2, 6, 6);   // "field2"

  const FieldLocation* loc1 = result.FindField(1);
  ASSERT_NE(loc1, nullptr);
  EXPECT_EQ(loc1->offset, 0);
  EXPECT_EQ(loc1->size, 6);

  const FieldLocation* loc2 = result.FindField(2);
  ASSERT_NE(loc2, nullptr);
  EXPECT_EQ(loc2->offset, 6);
  EXPECT_EQ(loc2->size, 6);

  const FieldLocation* loc3 = result.FindField(3);
  EXPECT_EQ(loc3, nullptr);
}

TEST(ZeroCopyParseResultTest, GetString) {
  auto buffer = ParseBuffer::Create("HelloWorld");
  ZeroCopyParseResult result(buffer);

  result.AddFieldLocation(1, 0, 5);   // "Hello"
  result.AddFieldLocation(2, 5, 5);   // "World"

  ZeroCopyString str1 = result.GetString(1);
  EXPECT_EQ(str1.view(), "Hello");

  ZeroCopyString str2 = result.GetString(2);
  EXPECT_EQ(str2.view(), "World");
}

TEST(ZeroCopyParseResultTest, GetStringView) {
  auto buffer = ParseBuffer::Create("TestData");
  ZeroCopyParseResult result(buffer);

  result.AddFieldLocation(1, 0, 4);   // "Test"
  result.AddFieldLocation(2, 4, 4);   // "Data"

  EXPECT_EQ(result.GetStringView(1), "Test");
  EXPECT_EQ(result.GetStringView(2), "Data");
  EXPECT_EQ(result.GetStringView(3), "");  // Non-existent
}

TEST(ZeroCopyParseResultTest, GetBytes) {
  auto buffer = ParseBuffer::Create("\x01\x02\x03\x04");
  ZeroCopyParseResult result(buffer);

  result.AddFieldLocation(1, 0, 2);
  result.AddFieldLocation(2, 2, 2);

  ZeroCopyBytes bytes1 = result.GetBytes(1);
  auto span1 = bytes1.span();
  EXPECT_EQ(span1.size(), 2);
  EXPECT_EQ(span1[0], 0x01);
  EXPECT_EQ(span1[1], 0x02);
}

// =============================================================================
// MmapBuffer Tests (requires file system access)
// =============================================================================

TEST(MmapBufferTest, OpenNonExistent) {
  auto buffer = MmapBuffer::Open("/nonexistent/path/file.pb");
  EXPECT_EQ(buffer, nullptr);
}

// =============================================================================
// Integration Tests
// =============================================================================

TEST(ZeroCopyIntegrationTest, BufferSharing) {
  // Multiple strings sharing the same buffer
  auto buffer = ParseBuffer::Create("Hello World Test");

  ZeroCopyString str1(buffer, 0, 5);    // "Hello"
  ZeroCopyString str2(buffer, 6, 5);    // "World"
  ZeroCopyString str3(buffer, 12, 4);   // "Test"

  EXPECT_EQ(str1.view(), "Hello");
  EXPECT_EQ(str2.view(), "World");
  EXPECT_EQ(str3.view(), "Test");

  // All share the same buffer
  EXPECT_TRUE(str1.IsZeroCopy());
  EXPECT_TRUE(str2.IsZeroCopy());
  EXPECT_TRUE(str3.IsZeroCopy());
}

TEST(ZeroCopyIntegrationTest, PartialMaterialization) {
  auto buffer = ParseBuffer::Create("Data1Data2");

  ZeroCopyString str1(buffer, 0, 5);
  ZeroCopyString str2(buffer, 5, 5);

  // Materialize only str1
  str1.Materialize();

  EXPECT_TRUE(str1.IsMaterialized());
  EXPECT_TRUE(str2.IsZeroCopy());  // str2 still zero-copy
}

TEST(ZeroCopyIntegrationTest, LargeBinaryData) {
  // Simulate large binary data
  std::string large_data(1024 * 1024, 'X');  // 1MB
  auto buffer = ParseBuffer::Create(std::move(large_data));

  ZeroCopyBytes bytes(buffer, 0, 1024 * 1024);

  auto span = bytes.span();
  EXPECT_EQ(span.size(), 1024 * 1024);
  EXPECT_EQ(span[0], 'X');
  EXPECT_EQ(span[span.size() - 1], 'X');
}

}  // namespace
}  // namespace zero_copy
}  // namespace protobuf
}  // namespace google
