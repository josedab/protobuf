// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// C++ specific helpers for generic/template message types.

#ifndef GOOGLE_PROTOBUF_COMPILER_CPP_GENERIC_HELPERS_H__
#define GOOGLE_PROTOBUF_COMPILER_CPP_GENERIC_HELPERS_H__

#include <string>
#include <vector>

#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/io/printer.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace cpp {

// Checks if a message is a generic (parameterized) message.
bool IsGenericMessage(const DescriptorProto& message);

// Gets C++ template parameter declaration string.
// E.g., "template <typename T, typename E = std::string>"
std::string GetCppTemplateDeclaration(const DescriptorProto& message);

// Gets C++ template parameter list for use in class definitions.
// E.g., "<T, E>"
std::string GetCppTemplateParameterList(const DescriptorProto& message);

// Gets the C++ type name for a type parameter.
// Handles constraint translation (e.g., "message" -> "google::protobuf::Message")
std::string GetCppTypeForParameter(const GenericTypeParameter& param);

// Gets the C++ default type for a type parameter.
std::string GetCppDefaultType(const GenericTypeParameter& param);

// Generates the C++ template class declaration for a generic message.
// This outputs the template<...> prefix and partial class definition.
void GenerateGenericMessageDeclaration(
    const DescriptorProto& message,
    const std::string& class_name,
    io::Printer* printer);

// Generates the forward declaration for a generic message template.
void GenerateGenericMessageForwardDeclaration(
    const DescriptorProto& message,
    const std::string& class_name,
    io::Printer* printer);

// Generates C++ type name for a field that uses a generic type parameter.
// Returns the template parameter name (e.g., "T") if the field uses one.
std::string GetCppFieldTypeName(
    const FieldDescriptorProto& field,
    const DescriptorProto& containing_message);

// Check if a field's type is a template parameter.
bool IsTemplateParameter(
    const FieldDescriptorProto& field,
    const DescriptorProto& containing_message);

// Get the C++ accessor method signature for a generic field type.
// For a field "T value = 1;", generates:
// - "const T& value() const"
// - "T* mutable_value()"
// etc.
std::string GetCppGenericAccessorSignature(
    const FieldDescriptorProto& field,
    const std::string& type_param_name,
    bool is_const);

// Generate instantiation typedef for a concrete generic type usage.
// E.g., "typedef Result<User> Result_User;"
void GenerateGenericInstantiation(
    const std::string& generic_name,
    const std::vector<std::string>& type_arguments,
    const std::string& instantiated_name,
    io::Printer* printer);

// Maps protobuf type names to C++ type names for use in templates.
std::string ProtobufTypeToCppType(const std::string& proto_type);

}  // namespace cpp
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_COMPILER_CPP_GENERIC_HELPERS_H__
