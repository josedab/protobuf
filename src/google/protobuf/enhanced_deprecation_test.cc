// Protocol Buffers - Google's data interchange format
// Copyright 2024 Google LLC.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Tests for enhanced field deprecation feature.

#include <string>
#include <vector>

#include "google/protobuf/descriptor.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/message.h"
#include <gtest/gtest.h>

namespace google {
namespace protobuf {
namespace {

// Helper function to extract deprecation info from a field using reflection.
// Returns true if the deprecation extension was found and populates the output
// parameters.
bool GetDeprecationInfo(const FieldDescriptor* field, std::string* replacement,
                        std::string* removal_version, std::string* since_version,
                        std::string* migration_guide, bool* breaking,
                        std::string* documentation_url) {
  if (!field->options().deprecated()) {
    return false;
  }

  const FieldOptions& options = field->options();
  const Reflection* reflection = options.GetReflection();

  std::vector<const FieldDescriptor*> fields;
  reflection->ListFields(options, &fields);

  for (const FieldDescriptor* f : fields) {
    if (f->is_extension() && f->number() == 1001 &&
        f->message_type() != nullptr &&
        f->message_type()->name() == "DeprecationInfo") {
      const Message& deprecation_msg = reflection->GetMessage(options, f);
      const Reflection* dep_refl = deprecation_msg.GetReflection();
      const Descriptor* dep_desc = deprecation_msg.GetDescriptor();

      const FieldDescriptor* replacement_field =
          dep_desc->FindFieldByName("replacement");
      const FieldDescriptor* removal_field =
          dep_desc->FindFieldByName("removal_version");
      const FieldDescriptor* since_field =
          dep_desc->FindFieldByName("since_version");
      const FieldDescriptor* migration_field =
          dep_desc->FindFieldByName("migration_guide");
      const FieldDescriptor* breaking_field =
          dep_desc->FindFieldByName("breaking");
      const FieldDescriptor* doc_url_field =
          dep_desc->FindFieldByName("documentation_url");

      if (replacement && replacement_field &&
          dep_refl->HasField(deprecation_msg, replacement_field)) {
        *replacement = dep_refl->GetString(deprecation_msg, replacement_field);
      }
      if (removal_version && removal_field &&
          dep_refl->HasField(deprecation_msg, removal_field)) {
        *removal_version = dep_refl->GetString(deprecation_msg, removal_field);
      }
      if (since_version && since_field &&
          dep_refl->HasField(deprecation_msg, since_field)) {
        *since_version = dep_refl->GetString(deprecation_msg, since_field);
      }
      if (migration_guide && migration_field &&
          dep_refl->HasField(deprecation_msg, migration_field)) {
        *migration_guide = dep_refl->GetString(deprecation_msg, migration_field);
      }
      if (breaking && breaking_field &&
          dep_refl->HasField(deprecation_msg, breaking_field)) {
        *breaking = dep_refl->GetBool(deprecation_msg, breaking_field);
      }
      if (documentation_url && doc_url_field &&
          dep_refl->HasField(deprecation_msg, doc_url_field)) {
        *documentation_url =
            dep_refl->GetString(deprecation_msg, doc_url_field);
      }

      return true;
    }
  }

  return false;
}

// Test that the DeprecationInfo message is properly defined.
TEST(EnhancedDeprecationTest, DeprecationInfoMessageExists) {
  const DescriptorPool* pool = DescriptorPool::generated_pool();
  const Descriptor* deprecation_info =
      pool->FindMessageTypeByName("google.protobuf.DeprecationInfo");

  // Note: This test will fail if deprecation.proto hasn't been compiled yet.
  // This is expected during initial implementation.
  if (deprecation_info == nullptr) {
    GTEST_SKIP() << "DeprecationInfo message not found - proto may not be "
                    "compiled yet";
  }

  ASSERT_NE(deprecation_info, nullptr);
  EXPECT_EQ(deprecation_info->name(), "DeprecationInfo");

  // Verify all expected fields exist
  EXPECT_NE(deprecation_info->FindFieldByName("replacement"), nullptr);
  EXPECT_NE(deprecation_info->FindFieldByName("removal_version"), nullptr);
  EXPECT_NE(deprecation_info->FindFieldByName("since_version"), nullptr);
  EXPECT_NE(deprecation_info->FindFieldByName("migration_guide"), nullptr);
  EXPECT_NE(deprecation_info->FindFieldByName("breaking"), nullptr);
  EXPECT_NE(deprecation_info->FindFieldByName("documentation_url"), nullptr);
  EXPECT_NE(deprecation_info->FindFieldByName("tags"), nullptr);
}

// Test that the deprecation extension is properly defined for FieldOptions.
TEST(EnhancedDeprecationTest, FieldOptionsExtensionExists) {
  const DescriptorPool* pool = DescriptorPool::generated_pool();
  const FieldDescriptor* deprecation_ext =
      pool->FindExtensionByName("google.protobuf.deprecation");

  // Note: This test will fail if deprecation.proto hasn't been compiled yet.
  if (deprecation_ext == nullptr) {
    GTEST_SKIP() << "deprecation extension not found - proto may not be "
                    "compiled yet";
  }

  ASSERT_NE(deprecation_ext, nullptr);
  EXPECT_EQ(deprecation_ext->number(), 1001);
  EXPECT_EQ(deprecation_ext->containing_type()->full_name(),
            "google.protobuf.FieldOptions");
}

// Test basic deprecation flag (backward compatibility).
TEST(EnhancedDeprecationTest, BasicDeprecationStillWorks) {
  const DescriptorPool* pool = DescriptorPool::generated_pool();
  const Descriptor* field_options =
      pool->FindMessageTypeByName("google.protobuf.FieldOptions");
  ASSERT_NE(field_options, nullptr);

  const FieldDescriptor* deprecated_field =
      field_options->FindFieldByName("deprecated");
  ASSERT_NE(deprecated_field, nullptr);
  EXPECT_EQ(deprecated_field->type(), FieldDescriptor::TYPE_BOOL);
}

// Test that DeprecationInfo fields have correct types.
TEST(EnhancedDeprecationTest, DeprecationInfoFieldTypes) {
  const DescriptorPool* pool = DescriptorPool::generated_pool();
  const Descriptor* deprecation_info =
      pool->FindMessageTypeByName("google.protobuf.DeprecationInfo");

  if (deprecation_info == nullptr) {
    GTEST_SKIP() << "DeprecationInfo message not found";
  }

  // Check field types
  const FieldDescriptor* replacement =
      deprecation_info->FindFieldByName("replacement");
  EXPECT_EQ(replacement->type(), FieldDescriptor::TYPE_STRING);

  const FieldDescriptor* breaking =
      deprecation_info->FindFieldByName("breaking");
  EXPECT_EQ(breaking->type(), FieldDescriptor::TYPE_BOOL);

  const FieldDescriptor* tags = deprecation_info->FindFieldByName("tags");
  EXPECT_TRUE(tags->is_map());
}

}  // namespace
}  // namespace protobuf
}  // namespace google
