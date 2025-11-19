// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#ifndef GOOGLE_PROTOBUF_COMPILER_PYTHON_VALIDATION_GENERATOR_H__
#define GOOGLE_PROTOBUF_COMPILER_PYTHON_VALIDATION_GENERATOR_H__

#include <string>

#include "google/protobuf/descriptor.h"
#include "google/protobuf/io/printer.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace python {

// PythonValidationGenerator handles the generation of validation code for
// protobuf messages in Python.
class PythonValidationGenerator {
 public:
  explicit PythonValidationGenerator(const Descriptor* descriptor);
  ~PythonValidationGenerator() = default;

  PythonValidationGenerator(const PythonValidationGenerator&) = delete;
  PythonValidationGenerator& operator=(const PythonValidationGenerator&) = delete;

  // Returns true if this message has any validation rules.
  bool HasValidationRules() const;

  // Generate the validation helper imports.
  void GenerateImports(io::Printer* printer);

  // Generate the validation method for the message class.
  void GenerateValidateMethod(io::Printer* printer);

 private:
  // Generate validation code for a single field.
  void GenerateFieldValidation(io::Printer* printer,
                                const FieldDescriptor* field);

  // Generate validation for string fields.
  void GenerateStringValidation(io::Printer* printer,
                                 const FieldDescriptor* field);

  // Generate validation for numeric fields.
  void GenerateNumericValidation(io::Printer* printer,
                                  const FieldDescriptor* field);

  // Generate validation for message fields.
  void GenerateMessageValidation(io::Printer* printer,
                                  const FieldDescriptor* field);

  // Generate validation for repeated fields.
  void GenerateRepeatedValidation(io::Printer* printer,
                                   const FieldDescriptor* field);

  // Generate validation for map fields.
  void GenerateMapValidation(io::Printer* printer,
                              const FieldDescriptor* field);

  // Helper to check if a field has validation rules.
  bool FieldHasValidationRules(const FieldDescriptor* field) const;

  // Get the Python field name.
  std::string GetFieldName(const FieldDescriptor* field) const;

  const Descriptor* descriptor_;
};

}  // namespace python
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_COMPILER_PYTHON_VALIDATION_GENERATOR_H__
