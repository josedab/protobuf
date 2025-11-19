// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#ifndef GOOGLE_PROTOBUF_COMPILER_CPP_VALIDATION_GENERATOR_H__
#define GOOGLE_PROTOBUF_COMPILER_CPP_VALIDATION_GENERATOR_H__

#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "google/protobuf/compiler/cpp/options.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/io/printer.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace cpp {

// ValidationGenerator handles the generation of validation code for protobuf
// messages in C++.
class ValidationGenerator {
 public:
  ValidationGenerator(const Descriptor* descriptor, const Options& options);
  ~ValidationGenerator() = default;

  ValidationGenerator(const ValidationGenerator&) = delete;
  ValidationGenerator& operator=(const ValidationGenerator&) = delete;

  // Returns true if this message has any validation rules.
  bool HasValidationRules() const;

  // Generate the declaration of the Validate() method in the class definition.
  void GenerateValidateDeclaration(io::Printer* p);

  // Generate the implementation of the Validate() method.
  void GenerateValidateImplementation(io::Printer* p);

 private:
  // Generate validation code for a single field.
  void GenerateFieldValidation(io::Printer* p, const FieldDescriptor* field);

  // Generate validation for string fields.
  void GenerateStringValidation(io::Printer* p, const FieldDescriptor* field,
                                const std::string& field_name);

  // Generate validation for numeric fields.
  void GenerateNumericValidation(io::Printer* p, const FieldDescriptor* field,
                                 const std::string& field_name);

  // Generate validation for message fields.
  void GenerateMessageValidation(io::Printer* p, const FieldDescriptor* field,
                                 const std::string& field_name);

  // Generate validation for repeated fields.
  void GenerateRepeatedValidation(io::Printer* p, const FieldDescriptor* field,
                                  const std::string& field_name);

  // Generate validation for map fields.
  void GenerateMapValidation(io::Printer* p, const FieldDescriptor* field,
                             const std::string& field_name);

  // Generate validation for bytes fields.
  void GenerateBytesValidation(io::Printer* p, const FieldDescriptor* field,
                               const std::string& field_name);

  // Generate validation for enum fields.
  void GenerateEnumValidation(io::Printer* p, const FieldDescriptor* field,
                              const std::string& field_name);

  // Generate validation for bool fields.
  void GenerateBoolValidation(io::Printer* p, const FieldDescriptor* field,
                              const std::string& field_name);

  // Helper to check if a field has validation rules.
  bool FieldHasValidationRules(const FieldDescriptor* field) const;

  // Get the C++ field name for a given field descriptor.
  std::string GetFieldName(const FieldDescriptor* field) const;

  const Descriptor* descriptor_;
  const Options& options_;
};

}  // namespace cpp
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_COMPILER_CPP_VALIDATION_GENERATOR_H__
