// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/compiler/python/validation_generator.h"

#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_replace.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/io/printer.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace python {

PythonValidationGenerator::PythonValidationGenerator(
    const Descriptor* descriptor)
    : descriptor_(descriptor) {}

bool PythonValidationGenerator::HasValidationRules() const {
  for (int i = 0; i < descriptor_->field_count(); ++i) {
    if (FieldHasValidationRules(descriptor_->field(i))) {
      return true;
    }
  }
  return false;
}

bool PythonValidationGenerator::FieldHasValidationRules(
    const FieldDescriptor* field) const {
  return field->options().HasExtension(google::protobuf::validate);
}

std::string PythonValidationGenerator::GetFieldName(
    const FieldDescriptor* field) const {
  return field->name();
}

void PythonValidationGenerator::GenerateImports(io::Printer* printer) {
  printer->Print(
      "from google.protobuf import validation as _validation\n"
      "import re as _re\n");
}

void PythonValidationGenerator::GenerateValidateMethod(io::Printer* printer) {
  std::string class_name = descriptor_->name();

  printer->Print(
      "def validate(self):\n"
      "    \"\"\"Validates this message according to validation rules.\n"
      "\n"
      "    Returns:\n"
      "        ValidationResult: Contains validation errors if any.\n"
      "    \"\"\"\n"
      "    errors = []\n"
      "\n");

  // Generate validation for each field
  for (int i = 0; i < descriptor_->field_count(); ++i) {
    GenerateFieldValidation(printer, descriptor_->field(i));
  }

  printer->Print(
      "    return _validation.ValidationResult(errors=errors)\n"
      "\n"
      "def validate_or_raise(self):\n"
      "    \"\"\"Validates this message and raises if validation fails.\n"
      "\n"
      "    Raises:\n"
      "        ValidationException: If validation fails.\n"
      "    \"\"\"\n"
      "    result = self.validate()\n"
      "    if not result.is_valid:\n"
      "        raise _validation.ValidationException(result)\n"
      "\n");
}

void PythonValidationGenerator::GenerateFieldValidation(
    io::Printer* printer, const FieldDescriptor* field) {
  std::string field_name = GetFieldName(field);

  if (field->is_map()) {
    GenerateMapValidation(printer, field);
  } else if (field->is_repeated()) {
    GenerateRepeatedValidation(printer, field);
  } else {
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
        break;
    }
  }
}

void PythonValidationGenerator::GenerateStringValidation(
    io::Printer* printer, const FieldDescriptor* field) {
  std::string field_name = GetFieldName(field);

  printer->Print(
      "    # Validate string field: $field_name$\n"
      "    if self.$field_name$:\n"
      "        value = self.$field_name$\n"
      "        # String validation rules would be applied here\n"
      "        # Example: length validation, pattern matching\n"
      "        # if len(value) < min_len:\n"
      "        #     errors.append(_validation.ValidationError(\n"
      "        #         field='$field_name$',\n"
      "        #         message=f'length must be >= {min_len}'\n"
      "        #     ))\n"
      "        pass\n"
      "\n",
      "field_name", field_name);
}

void PythonValidationGenerator::GenerateNumericValidation(
    io::Printer* printer, const FieldDescriptor* field) {
  std::string field_name = GetFieldName(field);

  printer->Print(
      "    # Validate numeric field: $field_name$\n"
      "    {\n"
      "        value = self.$field_name$\n"
      "        # Numeric validation rules would be applied here\n"
      "        # Example: range validation (gte, lte, gt, lt)\n"
      "        # if value < gte:\n"
      "        #     errors.append(_validation.ValidationError(\n"
      "        #         field='$field_name$',\n"
      "        #         message=f'must be >= {gte}'\n"
      "        #     ))\n"
      "    }\n"
      "\n",
      "field_name", field_name);
}

void PythonValidationGenerator::GenerateMessageValidation(
    io::Printer* printer, const FieldDescriptor* field) {
  std::string field_name = GetFieldName(field);

  printer->Print(
      "    # Validate message field: $field_name$\n"
      "    # Required validation\n"
      "    # if required and not self.HasField('$field_name$'):\n"
      "    #     errors.append(_validation.ValidationError(\n"
      "    #         field='$field_name$',\n"
      "    #         message='field is required'\n"
      "    #     ))\n"
      "\n"
      "    # Nested validation\n"
      "    if self.HasField('$field_name$'):\n"
      "        nested_result = self.$field_name$.validate()\n"
      "        if not nested_result.is_valid:\n"
      "            for error in nested_result.errors:\n"
      "                errors.append(_validation.ValidationError(\n"
      "                    field=f'$field_name$.{error.field}',\n"
      "                    message=error.message\n"
      "                ))\n"
      "\n",
      "field_name", field_name);
}

void PythonValidationGenerator::GenerateRepeatedValidation(
    io::Printer* printer, const FieldDescriptor* field) {
  std::string field_name = GetFieldName(field);

  printer->Print(
      "    # Validate repeated field: $field_name$\n"
      "    {\n"
      "        count = len(self.$field_name$)\n"
      "        # Min/max items validation\n"
      "        # if count < min_items:\n"
      "        #     errors.append(_validation.ValidationError(\n"
      "        #         field='$field_name$',\n"
      "        #         message=f'must have at least {min_items} items'\n"
      "        #     ))\n"
      "\n"
      "        # Validate each item\n"
      "        for i, item in enumerate(self.$field_name$):\n"
      "            # Item validation would go here\n"
      "            pass\n"
      "    }\n"
      "\n",
      "field_name", field_name);
}

void PythonValidationGenerator::GenerateMapValidation(
    io::Printer* printer, const FieldDescriptor* field) {
  std::string field_name = GetFieldName(field);

  printer->Print(
      "    # Validate map field: $field_name$\n"
      "    {\n"
      "        count = len(self.$field_name$)\n"
      "        # Min/max pairs validation\n"
      "        # if count < min_pairs:\n"
      "        #     errors.append(_validation.ValidationError(\n"
      "        #         field='$field_name$',\n"
      "        #         message=f'must have at least {min_pairs} entries'\n"
      "        #     ))\n"
      "\n"
      "        # Validate keys and values\n"
      "        for key, value in self.$field_name$.items():\n"
      "            # Key and value validation would go here\n"
      "            pass\n"
      "    }\n"
      "\n",
      "field_name", field_name);
}

}  // namespace python
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
