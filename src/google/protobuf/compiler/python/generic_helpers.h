// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Python specific helpers for generic/template message types.

#ifndef GOOGLE_PROTOBUF_COMPILER_PYTHON_GENERIC_HELPERS_H__
#define GOOGLE_PROTOBUF_COMPILER_PYTHON_GENERIC_HELPERS_H__

#include <string>
#include <vector>

#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/io/printer.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace python {

// Checks if a message is a generic (parameterized) message.
bool IsGenericMessage(const DescriptorProto& message);

// Gets Python typing generic declaration.
// E.g., "Generic[T, E]"
std::string GetPythonGenericDeclaration(const DescriptorProto& message);

// Gets Python TypeVar declarations for the message.
// E.g., "T = TypeVar('T', bound=Message)"
void GeneratePythonTypeVars(
    const DescriptorProto& message,
    io::Printer* printer);

// Gets Python type hint for a type parameter.
std::string GetPythonTypeHint(const GenericTypeParameter& param);

// Gets the Python default type for a type parameter.
std::string GetPythonDefaultType(const GenericTypeParameter& param);

// Generates Python class definition for a generic message.
void GeneratePythonGenericClass(
    const DescriptorProto& message,
    const std::string& class_name,
    io::Printer* printer);

// Generates Python property getter for a generic field.
void GeneratePythonGenericProperty(
    const FieldDescriptorProto& field,
    const std::string& type_param_name,
    io::Printer* printer);

// Generates Python .pyi stub for a generic message.
void GeneratePythonGenericStub(
    const DescriptorProto& message,
    const std::string& class_name,
    io::Printer* printer);

// Maps protobuf type names to Python type names.
std::string ProtobufTypeToPythonType(const std::string& proto_type);

// Maps protobuf type names to Python typing module types.
std::string ProtobufTypeToPythonTyping(const std::string& proto_type);

}  // namespace python
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_COMPILER_PYTHON_GENERIC_HELPERS_H__
