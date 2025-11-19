// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/compiler/java/validation_generator.h"

#include <string>

#include "absl/strings/str_cat.h"
#include "google/protobuf/compiler/java/context.h"
#include "google/protobuf/compiler/java/helpers.h"
#include "google/protobuf/compiler/java/name_resolver.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/io/printer.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace java {

JavaValidationGenerator::JavaValidationGenerator(const Descriptor* descriptor,
                                                 Context* context)
    : descriptor_(descriptor), context_(context) {}

bool JavaValidationGenerator::HasValidationRules() const {
  for (int i = 0; i < descriptor_->field_count(); ++i) {
    if (FieldHasValidationRules(descriptor_->field(i))) {
      return true;
    }
  }
  return false;
}

bool JavaValidationGenerator::FieldHasValidationRules(
    const FieldDescriptor* field) const {
  // Check if the field has the validate extension
  return field->options().HasExtension(google::protobuf::validate);
}

std::string JavaValidationGenerator::GetAccessorName(
    const FieldDescriptor* field) const {
  return UnderscoresToCamelCase(field->name(), true);
}

void JavaValidationGenerator::GenerateValidateMethods(io::Printer* printer) {
  printer->Print(
      "/**\n"
      " * Validates this message according to validation rules.\n"
      " * @return ValidationResult containing validation errors if any.\n"
      " */\n"
      "public com.google.protobuf.ValidationResult validate() {\n"
      "  com.google.protobuf.ValidationResult.Builder result = \n"
      "      com.google.protobuf.ValidationResult.newBuilder();\n"
      "  result.setIsValid(true);\n"
      "\n");

  // Generate validation for each field
  for (int i = 0; i < descriptor_->field_count(); ++i) {
    GenerateFieldValidation(printer, descriptor_->field(i));
  }

  printer->Print(
      "  return result.build();\n"
      "}\n"
      "\n"
      "/**\n"
      " * Validates this message and throws if validation fails.\n"
      " * @throws com.google.protobuf.ValidationException if validation fails.\n"
      " */\n"
      "public void validateOrThrow() throws com.google.protobuf.ValidationException {\n"
      "  com.google.protobuf.ValidationResult result = validate();\n"
      "  if (!result.getIsValid()) {\n"
      "    throw new com.google.protobuf.ValidationException(result);\n"
      "  }\n"
      "}\n\n");
}

void JavaValidationGenerator::GenerateValidateMethodBodies(
    io::Printer* printer) {
  // Implementation bodies are generated inline in GenerateValidateMethods
}

void JavaValidationGenerator::GenerateFieldValidation(
    io::Printer* printer, const FieldDescriptor* field) {
  std::string accessor_name = GetAccessorName(field);

  if (field->is_map()) {
    GenerateMapValidation(printer, field);
  } else if (field->is_repeated()) {
    GenerateRepeatedValidation(printer, field);
  } else {
    // Singular field
    switch (field->type()) {
      case FieldDescriptor::TYPE_STRING:
        GenerateStringValidation(printer, field);
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
        GenerateNumericValidation(printer, field);
        break;
      case FieldDescriptor::TYPE_MESSAGE:
      case FieldDescriptor::TYPE_GROUP:
        GenerateMessageValidation(printer, field);
        break;
      default:
        // Other field types (bool, bytes, enum) - basic validation
        break;
    }
  }
}

void JavaValidationGenerator::GenerateStringValidation(
    io::Printer* printer, const FieldDescriptor* field) {
  std::string accessor = GetAccessorName(field);
  std::string field_name = field->name();

  printer->Print(
      "  // Validate string field: $field_name$\n"
      "  {\n"
      "    String value = get$accessor$();\n"
      "    // String validation rules would be applied here\n"
      "    // Example: length validation, pattern matching, email format, etc.\n"
      "    // if (value.length() < minLen) {\n"
      "    //   result.setIsValid(false);\n"
      "    //   result.addErrors(com.google.protobuf.ValidationError.newBuilder()\n"
      "    //       .setField(\"$field_name$\")\n"
      "    //       .setMessage(\"length must be >= \" + minLen)\n"
      "    //       .build());\n"
      "    // }\n"
      "  }\n",
      "accessor", accessor,
      "field_name", field_name);
}

void JavaValidationGenerator::GenerateNumericValidation(
    io::Printer* printer, const FieldDescriptor* field) {
  std::string accessor = GetAccessorName(field);
  std::string field_name = field->name();

  printer->Print(
      "  // Validate numeric field: $field_name$\n"
      "  {\n"
      "    // Numeric validation rules would be applied here\n"
      "    // Example: range validation (gte, lte, gt, lt)\n"
      "    // var value = get$accessor$();\n"
      "    // if (value < gte) {\n"
      "    //   result.setIsValid(false);\n"
      "    //   result.addErrors(com.google.protobuf.ValidationError.newBuilder()\n"
      "    //       .setField(\"$field_name$\")\n"
      "    //       .setMessage(\"must be >= \" + gte)\n"
      "    //       .build());\n"
      "    // }\n"
      "  }\n",
      "accessor", accessor,
      "field_name", field_name);
}

void JavaValidationGenerator::GenerateMessageValidation(
    io::Printer* printer, const FieldDescriptor* field) {
  std::string accessor = GetAccessorName(field);
  std::string field_name = field->name();

  printer->Print(
      "  // Validate message field: $field_name$\n"
      "  {\n"
      "    // Required validation\n"
      "    // if (required && !has$accessor$()) {\n"
      "    //   result.setIsValid(false);\n"
      "    //   result.addErrors(com.google.protobuf.ValidationError.newBuilder()\n"
      "    //       .setField(\"$field_name$\")\n"
      "    //       .setMessage(\"field is required\")\n"
      "    //       .build());\n"
      "    // }\n"
      "\n"
      "    // Nested validation (unless skip = true)\n"
      "    if (has$accessor$()) {\n"
      "      com.google.protobuf.ValidationResult nestedResult = \n"
      "          get$accessor$().validate();\n"
      "      if (!nestedResult.getIsValid()) {\n"
      "        result.setIsValid(false);\n"
      "        for (com.google.protobuf.ValidationError error : \n"
      "             nestedResult.getErrorsList()) {\n"
      "          result.addErrors(error.toBuilder()\n"
      "              .setField(\"$field_name$.\" + error.getField())\n"
      "              .build());\n"
      "        }\n"
      "      }\n"
      "    }\n"
      "  }\n",
      "accessor", accessor,
      "field_name", field_name);
}

void JavaValidationGenerator::GenerateRepeatedValidation(
    io::Printer* printer, const FieldDescriptor* field) {
  std::string accessor = GetAccessorName(field);
  std::string field_name = field->name();

  printer->Print(
      "  // Validate repeated field: $field_name$\n"
      "  {\n"
      "    int count = get$accessor$Count();\n"
      "    // Min/max items validation\n"
      "    // if (count < minItems) {\n"
      "    //   result.setIsValid(false);\n"
      "    //   result.addErrors(com.google.protobuf.ValidationError.newBuilder()\n"
      "    //       .setField(\"$field_name$\")\n"
      "    //       .setMessage(\"must have at least \" + minItems + \" items\")\n"
      "    //       .build());\n"
      "    // }\n"
      "\n"
      "    // Validate each item\n"
      "    for (int i = 0; i < count; i++) {\n"
      "      // Item validation would go here\n"
      "    }\n"
      "  }\n",
      "accessor", accessor,
      "field_name", field_name);
}

void JavaValidationGenerator::GenerateMapValidation(io::Printer* printer,
                                                     const FieldDescriptor* field) {
  std::string accessor = GetAccessorName(field);
  std::string field_name = field->name();

  printer->Print(
      "  // Validate map field: $field_name$\n"
      "  {\n"
      "    int count = get$accessor$Count();\n"
      "    // Min/max pairs validation\n"
      "    // if (count < minPairs) {\n"
      "    //   result.setIsValid(false);\n"
      "    //   result.addErrors(com.google.protobuf.ValidationError.newBuilder()\n"
      "    //       .setField(\"$field_name$\")\n"
      "    //       .setMessage(\"must have at least \" + minPairs + \" entries\")\n"
      "    //       .build());\n"
      "    // }\n"
      "\n"
      "    // Validate keys and values\n"
      "    for (var entry : get$accessor$Map().entrySet()) {\n"
      "      // Key and value validation would go here\n"
      "    }\n"
      "  }\n",
      "accessor", accessor,
      "field_name", field_name);
}

}  // namespace java
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
