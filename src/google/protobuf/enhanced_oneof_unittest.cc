// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// RFC-0013: Enhanced Oneof Semantics Tests

#include <string>

#include "google/protobuf/compiler/parser.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/io/tokenizer.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"
#include "google/protobuf/text_format.h"
#include "google/protobuf/unittest.pb.h"
#include <gtest/gtest.h>

namespace google {
namespace protobuf {
namespace {

class EnhancedOneofTest : public testing::Test {
 protected:
  // Helper to parse a proto definition and return the file descriptor
  const FileDescriptor* ParseProto(const std::string& proto_text) {
    io::ArrayInputStream input(proto_text.data(), proto_text.size());
    io::ErrorCollector error_collector;
    io::Tokenizer tokenizer(&input, &error_collector);
    compiler::Parser parser;
    FileDescriptorProto file_proto;
    file_proto.set_name("test.proto");
    EXPECT_TRUE(parser.Parse(&tokenizer, &file_proto));
    return pool_.BuildFile(file_proto);
  }

  DescriptorPool pool_;
};

// Test: Repeated fields in oneof are allowed by parser
TEST_F(EnhancedOneofTest, RepeatedFieldInOneof) {
  const char* proto_text = R"(
    syntax = "proto2";
    message TestMessage {
      oneof selection {
        repeated int64 ids = 1;
        string filter = 2;
      }
    }
  )";

  const FileDescriptor* file = ParseProto(proto_text);
  ASSERT_NE(file, nullptr);

  const Descriptor* message = file->message_type(0);
  ASSERT_EQ(message->oneof_decl_count(), 1);

  const OneofDescriptor* oneof = message->oneof_decl(0);
  ASSERT_EQ(oneof->field_count(), 2);

  // First field should be repeated
  const FieldDescriptor* ids_field = oneof->field(0);
  EXPECT_EQ(ids_field->name(), "ids");
  EXPECT_TRUE(ids_field->is_repeated());
  EXPECT_EQ(ids_field->label(), FieldDescriptor::LABEL_REPEATED);

  // Second field should be optional
  const FieldDescriptor* filter_field = oneof->field(1);
  EXPECT_EQ(filter_field->name(), "filter");
  EXPECT_FALSE(filter_field->is_repeated());
  EXPECT_EQ(filter_field->label(), FieldDescriptor::LABEL_OPTIONAL);
}

// Test: Nested oneofs are allowed by parser
TEST_F(EnhancedOneofTest, NestedOneof) {
  const char* proto_text = R"(
    syntax = "proto2";
    message Shape {
      oneof shape_type {
        int32 circle = 1;
        oneof quadrilateral {
          int32 rectangle = 2;
          int32 square = 3;
        }
        int32 triangle = 4;
      }
    }
  )";

  const FileDescriptor* file = ParseProto(proto_text);
  ASSERT_NE(file, nullptr);

  const Descriptor* message = file->message_type(0);
  // Should have 2 oneofs: shape_type and quadrilateral
  ASSERT_EQ(message->oneof_decl_count(), 2);

  const OneofDescriptor* shape_type = message->oneof_decl(0);
  EXPECT_EQ(shape_type->name(), "shape_type");

  const OneofDescriptor* quadrilateral = message->oneof_decl(1);
  EXPECT_EQ(quadrilateral->name(), "quadrilateral");
  // Nested oneof should have parent_oneof_index set
}

// Test: Multiple repeated fields in same oneof
TEST_F(EnhancedOneofTest, MultipleRepeatedFieldsInOneof) {
  const char* proto_text = R"(
    syntax = "proto2";
    message TestMessage {
      oneof data {
        repeated string names = 1;
        repeated int32 numbers = 2;
        bool flag = 3;
      }
    }
  )";

  const FileDescriptor* file = ParseProto(proto_text);
  ASSERT_NE(file, nullptr);

  const Descriptor* message = file->message_type(0);
  const OneofDescriptor* oneof = message->oneof_decl(0);
  ASSERT_EQ(oneof->field_count(), 3);

  EXPECT_TRUE(oneof->field(0)->is_repeated());  // names
  EXPECT_TRUE(oneof->field(1)->is_repeated());  // numbers
  EXPECT_FALSE(oneof->field(2)->is_repeated()); // flag
}

// Test: track_unset option
TEST_F(EnhancedOneofTest, TrackUnsetOption) {
  const char* proto_text = R"(
    syntax = "proto2";
    message Config {
      oneof mode {
        option (track_unset) = true;
        int32 timeout_ms = 1;
        bool use_default = 2;
      }
    }
  )";

  const FileDescriptor* file = ParseProto(proto_text);
  ASSERT_NE(file, nullptr);

  const Descriptor* message = file->message_type(0);
  const OneofDescriptor* oneof = message->oneof_decl(0);

  // Check that track_unset option is set
  EXPECT_TRUE(oneof->options().track_unset());
}

// Test: required option
TEST_F(EnhancedOneofTest, RequiredOption) {
  const char* proto_text = R"(
    syntax = "proto2";
    message Action {
      oneof action_type {
        option (required) = true;
        int32 create = 1;
        int32 update = 2;
        int32 delete = 3;
      }
    }
  )";

  const FileDescriptor* file = ParseProto(proto_text);
  ASSERT_NE(file, nullptr);

  const Descriptor* message = file->message_type(0);
  const OneofDescriptor* oneof = message->oneof_decl(0);

  // Check that required option is set
  EXPECT_TRUE(oneof->options().required());
}

// Test: Both options together
TEST_F(EnhancedOneofTest, CombinedOptions) {
  const char* proto_text = R"(
    syntax = "proto2";
    message FullFeatured {
      oneof choice {
        option (track_unset) = true;
        option (required) = true;
        string text = 1;
        int32 number = 2;
      }
    }
  )";

  const FileDescriptor* file = ParseProto(proto_text);
  ASSERT_NE(file, nullptr);

  const Descriptor* message = file->message_type(0);
  const OneofDescriptor* oneof = message->oneof_decl(0);

  EXPECT_TRUE(oneof->options().track_unset());
  EXPECT_TRUE(oneof->options().required());
}

// Test: Deeply nested oneofs
TEST_F(EnhancedOneofTest, DeeplyNestedOneof) {
  const char* proto_text = R"(
    syntax = "proto2";
    message Complex {
      oneof outer {
        string name = 1;
        oneof polygon {
          int32 triangle = 2;
          oneof quadrilateral {
            int32 rectangle = 3;
            int32 square = 4;
          }
        }
      }
    }
  )";

  const FileDescriptor* file = ParseProto(proto_text);
  ASSERT_NE(file, nullptr);

  const Descriptor* message = file->message_type(0);
  // Should have 3 oneofs: outer, polygon, quadrilateral
  ASSERT_EQ(message->oneof_decl_count(), 3);

  EXPECT_EQ(message->oneof_decl(0)->name(), "outer");
  EXPECT_EQ(message->oneof_decl(1)->name(), "polygon");
  EXPECT_EQ(message->oneof_decl(2)->name(), "quadrilateral");
}

// Test: Error on required and optional labels (should still fail)
TEST_F(EnhancedOneofTest, RequiredLabelStillErrors) {
  const char* proto_text = R"(
    syntax = "proto2";
    message TestMessage {
      oneof selection {
        required int32 id = 1;
        string filter = 2;
      }
    }
  )";

  io::ArrayInputStream input(proto_text, strlen(proto_text));
  io::ErrorCollector error_collector;
  io::Tokenizer tokenizer(&input, &error_collector);
  compiler::Parser parser;
  FileDescriptorProto file_proto;
  file_proto.set_name("test.proto");

  // This should produce an error
  EXPECT_FALSE(parser.Parse(&tokenizer, &file_proto));
}

// Test: Error on optional label (should still fail)
TEST_F(EnhancedOneofTest, OptionalLabelStillErrors) {
  const char* proto_text = R"(
    syntax = "proto2";
    message TestMessage {
      oneof selection {
        optional int32 id = 1;
        string filter = 2;
      }
    }
  )";

  io::ArrayInputStream input(proto_text, strlen(proto_text));
  io::ErrorCollector error_collector;
  io::Tokenizer tokenizer(&input, &error_collector);
  compiler::Parser parser;
  FileDescriptorProto file_proto;
  file_proto.set_name("test.proto");

  // This should produce an error
  EXPECT_FALSE(parser.Parse(&tokenizer, &file_proto));
}

}  // namespace
}  // namespace protobuf
}  // namespace google
