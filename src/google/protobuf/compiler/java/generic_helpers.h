// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Java specific helpers for generic/template message types.

#ifndef GOOGLE_PROTOBUF_COMPILER_JAVA_GENERIC_HELPERS_H__
#define GOOGLE_PROTOBUF_COMPILER_JAVA_GENERIC_HELPERS_H__

#include <string>
#include <vector>

#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/io/printer.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace java {

// Checks if a message is a generic (parameterized) message.
bool IsGenericMessage(const DescriptorProto& message);

// Gets Java generic type parameter declaration string.
// E.g., "<T extends Message, E>"
std::string GetJavaGenericDeclaration(const DescriptorProto& message);

// Gets Java type parameter list.
// E.g., "<T, E>"
std::string GetJavaTypeParameterList(const DescriptorProto& message);

// Gets the Java type for a type parameter constraint.
// E.g., "message" -> "com.google.protobuf.Message"
std::string GetJavaTypeForConstraint(const GenericTypeParameter& param);

// Gets the Java default type for a type parameter.
std::string GetJavaDefaultType(const GenericTypeParameter& param);

// Generates the Java generic class declaration.
void GenerateJavaGenericClassDeclaration(
    const DescriptorProto& message,
    const std::string& class_name,
    io::Printer* printer);

// Generates Java getter method for a generic type field.
void GenerateJavaGenericGetter(
    const FieldDescriptorProto& field,
    const std::string& type_param_name,
    io::Printer* printer);

// Generates Java setter method for a generic type field.
void GenerateJavaGenericSetter(
    const FieldDescriptorProto& field,
    const std::string& type_param_name,
    io::Printer* printer);

// Generates Java Builder methods for a generic type.
void GenerateJavaGenericBuilderMethods(
    const DescriptorProto& message,
    const std::string& class_name,
    io::Printer* printer);

// Maps protobuf type names to Java type names.
std::string ProtobufTypeToJavaType(const std::string& proto_type);

// Get boxed Java type (e.g., "int" -> "Integer").
std::string GetBoxedJavaType(const std::string& java_type);

}  // namespace java
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_COMPILER_JAVA_GENERIC_HELPERS_H__
