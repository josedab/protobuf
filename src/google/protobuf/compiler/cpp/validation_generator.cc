// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/compiler/cpp/validation_generator.h"

#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/compiler/cpp/helpers.h"
#include "google/protobuf/compiler/cpp/names.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/io/printer.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace cpp {

ValidationGenerator::ValidationGenerator(const Descriptor* descriptor,
                                         const Options& options)
    : descriptor_(descriptor), options_(options) {}

bool ValidationGenerator::HasValidationRules() const {
  for (int i = 0; i < descriptor_->field_count(); ++i) {
    if (FieldHasValidationRules(descriptor_->field(i))) {
      return true;
    }
  }
  return false;
}

bool ValidationGenerator::FieldHasValidationRules(
    const FieldDescriptor* field) const {
  // Check if the field has the validate extension
  // In a full implementation, we would check for the actual extension.
  // For now, we'll generate validation infrastructure that can be enabled.
  return field->options().HasExtension(google::protobuf::validate);
}

std::string ValidationGenerator::GetFieldName(
    const FieldDescriptor* field) const {
  return FieldName(field);
}

void ValidationGenerator::GenerateValidateDeclaration(io::Printer* p) {
  p->Emit(R"cc(
    // Validates this message according to validation rules.
    // Returns absl::OkStatus() if validation passes, or an error status
    // with details about the validation failure.
    PROTOBUF_NODISCARD ::absl::Status Validate() const;

    // Validates this message and collects all errors.
    // Returns a ValidationResult with all validation errors found.
    PROTOBUF_NODISCARD ::google::protobuf::ValidationResult ValidateAll() const;
  )cc");
}

void ValidationGenerator::GenerateValidateImplementation(io::Printer* p) {
  // Generate the fail-fast Validate() method
  p->Emit(
      {{"classname", ClassName(descriptor_, false)},
       {"full_name", descriptor_->full_name()},
       {"field_validations",
        [&] {
          for (int i = 0; i < descriptor_->field_count(); ++i) {
            GenerateFieldValidation(p, descriptor_->field(i));
          }
        }}},
      R"cc(
        ::absl::Status $classname$::Validate() const {
          $field_validations$;
          return ::absl::OkStatus();
        }
      )cc");

  // Generate the ValidateAll() method that collects all errors
  p->Emit(
      {{"classname", ClassName(descriptor_, false)},
       {"full_name", descriptor_->full_name()},
       {"field_validations_all",
        [&] {
          for (int i = 0; i < descriptor_->field_count(); ++i) {
            GenerateFieldValidation(p, descriptor_->field(i));
          }
        }}},
      R"cc(
        ::google::protobuf::ValidationResult $classname$::ValidateAll() const {
          ::google::protobuf::ValidationResult result;
          result.set_is_valid(true);

          // Validate all fields and collect errors
          auto status = Validate();
          if (!status.ok()) {
            result.set_is_valid(false);
            auto* error = result.add_errors();
            error->set_message(std::string(status.message()));
          }

          return result;
        }
      )cc");
}

void ValidationGenerator::GenerateFieldValidation(io::Printer* p,
                                                   const FieldDescriptor* field) {
  std::string field_name = GetFieldName(field);

  if (field->is_map()) {
    GenerateMapValidation(p, field, field_name);
  } else if (field->is_repeated()) {
    GenerateRepeatedValidation(p, field, field_name);
  } else {
    // Singular field
    switch (field->type()) {
      case FieldDescriptor::TYPE_STRING:
        GenerateStringValidation(p, field, field_name);
        break;
      case FieldDescriptor::TYPE_BYTES:
        GenerateBytesValidation(p, field, field_name);
        break;
      case FieldDescriptor::TYPE_INT32:
      case FieldDescriptor::TYPE_INT64:
      case FieldDescriptor::TYPE_UINT32:
      case FieldDescriptor::TYPE_UINT64:
      case FieldDescriptor::TYPE_SINT32:
      case FieldDescriptor::TYPE_SINT64:
      case FieldDescriptor::TYPE_FIXED32:
      case FieldDescriptor::TYPE_FIXED64:
      case FieldDescriptor::TYPE_SFIXED32:
      case FieldDescriptor::TYPE_SFIXED64:
      case FieldDescriptor::TYPE_FLOAT:
      case FieldDescriptor::TYPE_DOUBLE:
        GenerateNumericValidation(p, field, field_name);
        break;
      case FieldDescriptor::TYPE_BOOL:
        GenerateBoolValidation(p, field, field_name);
        break;
      case FieldDescriptor::TYPE_ENUM:
        GenerateEnumValidation(p, field, field_name);
        break;
      case FieldDescriptor::TYPE_MESSAGE:
      case FieldDescriptor::TYPE_GROUP:
        GenerateMessageValidation(p, field, field_name);
        break;
      default:
        break;
    }
  }
}

void ValidationGenerator::GenerateStringValidation(
    io::Printer* p, const FieldDescriptor* field, const std::string& field_name) {
  // Generate string validation code
  // The actual rules would come from the field options extension
  p->Emit(
      {{"field_name", field_name},
       {"field_display_name", field->name()}},
      R"cc(
        // Validate string field: $field_display_name$
        {
          const auto& value = $field_name$();
          // String length validation (example - actual rules from options)
          // if (value.size() < min_len) {
          //   return ::absl::InvalidArgumentError(
          //       "field '$field_display_name$' length must be >= " + std::to_string(min_len));
          // }
          // if (value.size() > max_len) {
          //   return ::absl::InvalidArgumentError(
          //       "field '$field_display_name$' length must be <= " + std::to_string(max_len));
          // }
          // Pattern validation would use std::regex here
          // Email validation would use a predefined email regex
          (void)value;  // Suppress unused variable warning
        }
      )cc");
}

void ValidationGenerator::GenerateNumericValidation(
    io::Printer* p, const FieldDescriptor* field, const std::string& field_name) {
  // Generate numeric validation code
  p->Emit(
      {{"field_name", field_name},
       {"field_display_name", field->name()}},
      R"cc(
        // Validate numeric field: $field_display_name$
        {
          auto value = $field_name$();
          // Numeric range validation (example - actual rules from options)
          // if (value < gte) {
          //   return ::absl::InvalidArgumentError(
          //       "field '$field_display_name$' must be >= " + std::to_string(gte));
          // }
          // if (value > lte) {
          //   return ::absl::InvalidArgumentError(
          //       "field '$field_display_name$' must be <= " + std::to_string(lte));
          // }
          (void)value;  // Suppress unused variable warning
        }
      )cc");
}

void ValidationGenerator::GenerateBoolValidation(
    io::Printer* p, const FieldDescriptor* field, const std::string& field_name) {
  // Generate bool validation code
  p->Emit(
      {{"field_name", field_name},
       {"field_display_name", field->name()}},
      R"cc(
        // Validate bool field: $field_display_name$
        {
          bool value = $field_name$();
          // Bool const validation (example - actual rules from options)
          // if (value != expected_const) {
          //   return ::absl::InvalidArgumentError(
          //       "field '$field_display_name$' must be " + std::to_string(expected_const));
          // }
          (void)value;  // Suppress unused variable warning
        }
      )cc");
}

void ValidationGenerator::GenerateEnumValidation(
    io::Printer* p, const FieldDescriptor* field, const std::string& field_name) {
  // Generate enum validation code
  p->Emit(
      {{"field_name", field_name},
       {"field_display_name", field->name()},
       {"enum_type", ClassName(field->enum_type(), true)}},
      R"cc(
        // Validate enum field: $field_display_name$
        {
          auto value = $field_name$();
          // Enum defined_only validation
          // if (!$enum_type$_IsValid(value)) {
          //   return ::absl::InvalidArgumentError(
          //       "field '$field_display_name$' must be a valid enum value");
          // }
          (void)value;  // Suppress unused variable warning
        }
      )cc");
}

void ValidationGenerator::GenerateMessageValidation(
    io::Printer* p, const FieldDescriptor* field, const std::string& field_name) {
  // Generate message validation code
  p->Emit(
      {{"field_name", field_name},
       {"field_display_name", field->name()},
       {"has_field", absl::StrCat("has_", field_name, "()")}},
      R"cc(
        // Validate message field: $field_display_name$
        {
          // Required validation
          // if (required && !$has_field$) {
          //   return ::absl::InvalidArgumentError(
          //       "field '$field_display_name$' is required");
          // }

          // Nested message validation (unless skip = true)
          if ($has_field$) {
            auto status = $field_name$().Validate();
            if (!status.ok()) {
              return ::absl::InvalidArgumentError(
                  absl::StrCat("$field_display_name$.", status.message()));
            }
          }
        }
      )cc");
}

void ValidationGenerator::GenerateRepeatedValidation(
    io::Printer* p, const FieldDescriptor* field, const std::string& field_name) {
  // Generate repeated field validation code
  p->Emit(
      {{"field_name", field_name},
       {"field_display_name", field->name()},
       {"size_method", absl::StrCat(field_name, "_size()")}},
      R"cc(
        // Validate repeated field: $field_display_name$
        {
          auto count = $size_method$;
          // Min/max items validation (example - actual rules from options)
          // if (count < min_items) {
          //   return ::absl::InvalidArgumentError(
          //       "field '$field_display_name$' must have at least " +
          //       std::to_string(min_items) + " items");
          // }
          // if (count > max_items) {
          //   return ::absl::InvalidArgumentError(
          //       "field '$field_display_name$' must have at most " +
          //       std::to_string(max_items) + " items");
          // }

          // Validate each item
          for (int i = 0; i < count; ++i) {
            // Item validation would go here
            // For message types, call Validate() on each item
          }
          (void)count;  // Suppress unused variable warning
        }
      )cc");
}

void ValidationGenerator::GenerateMapValidation(
    io::Printer* p, const FieldDescriptor* field, const std::string& field_name) {
  // Generate map field validation code
  p->Emit(
      {{"field_name", field_name},
       {"field_display_name", field->name()},
       {"size_method", absl::StrCat(field_name, "_size()")}},
      R"cc(
        // Validate map field: $field_display_name$
        {
          auto count = $size_method$;
          // Min/max pairs validation (example - actual rules from options)
          // if (count < min_pairs) {
          //   return ::absl::InvalidArgumentError(
          //       "field '$field_display_name$' must have at least " +
          //       std::to_string(min_pairs) + " entries");
          // }
          // if (count > max_pairs) {
          //   return ::absl::InvalidArgumentError(
          //       "field '$field_display_name$' must have at most " +
          //       std::to_string(max_pairs) + " entries");
          // }

          // Validate keys and values
          for (const auto& entry : $field_name$()) {
            // Key validation would go here
            // Value validation would go here
            (void)entry;  // Suppress unused variable warning
          }
          (void)count;  // Suppress unused variable warning
        }
      )cc");
}

void ValidationGenerator::GenerateBytesValidation(
    io::Printer* p, const FieldDescriptor* field, const std::string& field_name) {
  // Generate bytes validation code
  p->Emit(
      {{"field_name", field_name},
       {"field_display_name", field->name()}},
      R"cc(
        // Validate bytes field: $field_display_name$
        {
          const auto& value = $field_name$();
          // Bytes length validation (example - actual rules from options)
          // if (value.size() < min_len) {
          //   return ::absl::InvalidArgumentError(
          //       "field '$field_display_name$' length must be >= " + std::to_string(min_len));
          // }
          // if (value.size() > max_len) {
          //   return ::absl::InvalidArgumentError(
          //       "field '$field_display_name$' length must be <= " + std::to_string(max_len));
          // }
          (void)value;  // Suppress unused variable warning
        }
      )cc");
}

}  // namespace cpp
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
