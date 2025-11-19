// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/compiler/generic_helpers.h"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"

namespace google {
namespace protobuf {
namespace compiler {

std::string GenerateMonomorphizedName(
    const std::string& generic_name,
    const std::vector<std::string>& type_arguments) {
  if (type_arguments.empty()) {
    return generic_name;
  }

  // Replace dots with underscores in type arguments for valid identifier
  std::vector<std::string> sanitized_args;
  for (const auto& arg : type_arguments) {
    std::string sanitized = absl::StrReplaceAll(arg, {{".", "_"}, {"<", "_"}, {">", ""}, {",", "_"}});
    sanitized_args.push_back(sanitized);
  }

  return absl::StrCat(generic_name, "_", absl::StrJoin(sanitized_args, "_"));
}

std::string GenerateMonomorphizedName(
    const std::string& generic_name,
    const RepeatedPtrField<GenericTypeArgument>& arguments) {
  std::vector<std::string> type_args;
  for (const auto& arg : arguments) {
    type_args.push_back(arg.type_name());
  }
  return GenerateMonomorphizedName(generic_name, type_args);
}

void MonomorphizeMessage(
    const DescriptorProto& generic_message,
    const std::vector<std::string>& type_arguments,
    DescriptorProto* result) {
  // Copy the generic message
  *result = generic_message;

  // Build the type parameter mapping
  auto type_param_map = BuildTypeParameterMap(generic_message, type_arguments);

  // Set the new name
  result->set_name(
      GenerateMonomorphizedName(generic_message.name(), type_arguments));

  // Set the source generic message name
  result->set_generic_source_name(generic_message.name());

  // Clear type parameters (this is now a concrete type)
  result->clear_type_parameter();

  // Set type arguments
  for (size_t i = 0; i < type_arguments.size() &&
                     i < static_cast<size_t>(generic_message.type_parameter_size());
       ++i) {
    auto* arg = result->add_type_argument();
    arg->set_parameter_name(generic_message.type_parameter(i).name());
    arg->set_type_name(type_arguments[i]);
  }

  // Replace all type parameter references in fields
  for (int i = 0; i < result->field_size(); ++i) {
    auto* field = result->mutable_field(i);

    // If the field's type is a generic type parameter
    if (field->has_generic_type_parameter()) {
      auto it = type_param_map.find(field->generic_type_parameter());
      if (it != type_param_map.end()) {
        // Replace with concrete type
        field->set_type_name(it->second);
        field->clear_generic_type_parameter();
      }
    }

    // If the field's type_name references a type parameter
    if (field->has_type_name()) {
      std::string resolved = ResolveTypeParameter(field->type_name(), type_param_map);
      if (resolved != field->type_name()) {
        field->set_type_name(resolved);
      }
    }
  }

  // Also update nested types recursively if they reference type parameters
  // (This would be needed for full generic support)
}

std::map<std::string, std::string> BuildTypeParameterMap(
    const DescriptorProto& generic_message,
    const std::vector<std::string>& type_arguments) {
  std::map<std::string, std::string> result;

  for (int i = 0; i < generic_message.type_parameter_size(); ++i) {
    const auto& param = generic_message.type_parameter(i);
    std::string value;

    if (i < static_cast<int>(type_arguments.size())) {
      value = type_arguments[i];
    } else if (param.has_default_type()) {
      value = param.default_type();
    }

    if (!value.empty()) {
      result[param.name()] = value;
    }
  }

  return result;
}

std::string ResolveTypeParameter(
    const std::string& type_name,
    const std::map<std::string, std::string>& type_param_map) {
  auto it = type_param_map.find(type_name);
  if (it != type_param_map.end()) {
    return it->second;
  }
  return type_name;
}

std::vector<std::string> GetTypeParameterNames(const DescriptorProto& message) {
  std::vector<std::string> result;
  for (const auto& param : message.type_parameter()) {
    result.push_back(param.name());
  }
  return result;
}

std::string FormatTypeArgumentList(
    const std::vector<std::string>& type_arguments) {
  if (type_arguments.empty()) {
    return "";
  }
  return absl::StrCat("<", absl::StrJoin(type_arguments, ", "), ">");
}

bool ParseMonomorphizedName(
    const std::string& monomorphized_name,
    std::string* generic_name,
    std::vector<std::string>* type_arguments) {
  // Find the first underscore that separates generic name from type arguments
  size_t first_underscore = monomorphized_name.find('_');
  if (first_underscore == std::string::npos) {
    *generic_name = monomorphized_name;
    type_arguments->clear();
    return true;
  }

  *generic_name = monomorphized_name.substr(0, first_underscore);
  std::string args_part = monomorphized_name.substr(first_underscore + 1);

  // Split by underscore to get type arguments
  // Note: This is a heuristic and won't work for nested generics
  *type_arguments = absl::StrSplit(args_part, '_');

  return true;
}

void GenericTypeRegistry::RegisterGenericMessage(const DescriptorProto& message) {
  if (IsGenericMessage(message)) {
    generic_messages_[message.name()] = &message;
  }
}

bool GenericTypeRegistry::IsGenericMessage(const std::string& name) const {
  return generic_messages_.find(name) != generic_messages_.end();
}

const DescriptorProto* GenericTypeRegistry::GetGenericMessage(
    const std::string& name) const {
  auto it = generic_messages_.find(name);
  if (it != generic_messages_.end()) {
    return it->second;
  }
  return nullptr;
}

std::string GenericTypeRegistry::RegisterInstantiation(
    const std::string& generic_name,
    const std::vector<std::string>& type_arguments) {
  std::string monomorphized_name =
      GenerateMonomorphizedName(generic_name, type_arguments);
  instantiations_[monomorphized_name] = std::make_pair(generic_name, type_arguments);
  return monomorphized_name;
}

}  // namespace compiler
}  // namespace protobuf
}  // namespace google
