// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#ifndef GOOGLE_PROTOBUF_COMPILER_JAVA_VALIDATION_GENERATOR_H__
#define GOOGLE_PROTOBUF_COMPILER_JAVA_VALIDATION_GENERATOR_H__

#include <string>

#include "google/protobuf/compiler/java/context.h"
#include "google/protobuf/descriptor.h"
#include "google/protobuf/io/printer.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace java {

// JavaValidationGenerator handles the generation of validation code for
// protobuf messages in Java.
class JavaValidationGenerator {
 public:
  JavaValidationGenerator(const Descriptor* descriptor, Context* context);
  ~JavaValidationGenerator() = default;

  JavaValidationGenerator(const JavaValidationGenerator&) = delete;
  JavaValidationGenerator& operator=(const JavaValidationGenerator&) = delete;

  // Returns true if this message has any validation rules.
  bool HasValidationRules() const;

  // Generate the validation method declarations.
  void GenerateValidateMethods(io::Printer* printer);

  // Generate the validation method implementation.
  void GenerateValidateMethodBodies(io::Printer* printer);

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

  // Get the Java accessor name for a field.
  std::string GetAccessorName(const FieldDescriptor* field) const;

  const Descriptor* descriptor_;
  Context* context_;
};

}  // namespace java
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_COMPILER_JAVA_VALIDATION_GENERATOR_H__
