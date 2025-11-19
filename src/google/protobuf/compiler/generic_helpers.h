// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Generic/Template message type helpers for protobuf compiler.
// This file provides utilities for working with generic message types,
// including monomorphization (generating concrete types from generic templates).

#ifndef GOOGLE_PROTOBUF_COMPILER_GENERIC_HELPERS_H__
#define GOOGLE_PROTOBUF_COMPILER_GENERIC_HELPERS_H__

#include <map>
#include <string>
#include <vector>

#include "absl/strings/string_view.h"
#include "google/protobuf/descriptor.pb.h"

namespace google {
namespace protobuf {
namespace compiler {

// Checks if a message type is a generic (parameterized) message.
inline bool IsGenericMessage(const DescriptorProto& message) {
  return message.type_parameter_size() > 0;
}

// Checks if a field uses a generic type parameter as its type.
inline bool IsGenericTypeParameter(const FieldDescriptorProto& field) {
  return field.has_generic_type_parameter() &&
         !field.generic_type_parameter().empty();
}

// Checks if a field uses a generic instantiation (e.g., Result<User>).
inline bool HasGenericTypeArguments(const FieldDescriptorProto& field) {
  return field.generic_type_arguments_size() > 0;
}

// Generates a monomorphized (concrete) message name from a generic message
// and type arguments. E.g., "Result" + ["User"] -> "Result_User"
std::string GenerateMonomorphizedName(
    const std::string& generic_name,
    const std::vector<std::string>& type_arguments);

// Generates a monomorphized message name from GenericTypeArguments.
std::string GenerateMonomorphizedName(
    const std::string& generic_name,
    const RepeatedPtrField<GenericTypeArgument>& arguments);

// Creates a monomorphized message descriptor from a generic message template.
// This creates a copy of the generic message with:
// - A new name (e.g., Result_User)
// - All type parameter references replaced with concrete types
// - generic_source_name set to the original generic message name
// - type_arguments populated with the instantiation arguments
void MonomorphizeMessage(
    const DescriptorProto& generic_message,
    const std::vector<std::string>& type_arguments,
    DescriptorProto* result);

// Builds a mapping from type parameter names to their concrete type names.
std::map<std::string, std::string> BuildTypeParameterMap(
    const DescriptorProto& generic_message,
    const std::vector<std::string>& type_arguments);

// Resolves a type name that might be a type parameter.
// If the type_name is a type parameter, returns the concrete type from the map.
// Otherwise, returns the type_name unchanged.
std::string ResolveTypeParameter(
    const std::string& type_name,
    const std::map<std::string, std::string>& type_param_map);

// Gets the list of type parameter names from a generic message.
std::vector<std::string> GetTypeParameterNames(const DescriptorProto& message);

// Creates a type argument list string for code generation.
// E.g., ["User", "ErrorCode"] -> "<User, ErrorCode>"
std::string FormatTypeArgumentList(
    const std::vector<std::string>& type_arguments);

// Parses a monomorphized name back to its components.
// E.g., "Result_User_ErrorCode" -> ("Result", ["User", "ErrorCode"])
// Note: This is a heuristic and may not work for all cases.
bool ParseMonomorphizedName(
    const std::string& monomorphized_name,
    std::string* generic_name,
    std::vector<std::string>* type_arguments);

// Helper class for managing generic type instantiations in a file.
class GenericTypeRegistry {
 public:
  // Register a generic message definition.
  void RegisterGenericMessage(const DescriptorProto& message);

  // Check if a message name is a registered generic message.
  bool IsGenericMessage(const std::string& name) const;

  // Get the generic message definition by name.
  const DescriptorProto* GetGenericMessage(const std::string& name) const;

  // Register an instantiation of a generic message.
  // Returns the monomorphized name.
  std::string RegisterInstantiation(
      const std::string& generic_name,
      const std::vector<std::string>& type_arguments);

  // Get all registered instantiations.
  const std::map<std::string, std::pair<std::string, std::vector<std::string>>>&
  GetInstantiations() const {
    return instantiations_;
  }

  // Get all generic message definitions.
  const std::map<std::string, const DescriptorProto*>& GetGenericMessages()
      const {
    return generic_messages_;
  }

 private:
  std::map<std::string, const DescriptorProto*> generic_messages_;
  // Maps monomorphized name -> (generic_name, type_arguments)
  std::map<std::string, std::pair<std::string, std::vector<std::string>>>
      instantiations_;
};

}  // namespace compiler
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_COMPILER_GENERIC_HELPERS_H__
