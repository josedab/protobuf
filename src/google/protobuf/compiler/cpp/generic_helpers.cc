// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/compiler/cpp/generic_helpers.h"

#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace cpp {

bool IsGenericMessage(const DescriptorProto& message) {
  return message.type_parameter_size() > 0;
}

std::string GetCppTemplateDeclaration(const DescriptorProto& message) {
  if (!IsGenericMessage(message)) {
    return "";
  }

  std::vector<std::string> params;
  for (const auto& param : message.type_parameter()) {
    std::string param_decl = "typename " + param.name();
    if (param.has_default_type()) {
      param_decl += " = " + GetCppDefaultType(param);
    }
    params.push_back(param_decl);
  }

  return "template <" + absl::StrJoin(params, ", ") + ">";
}

std::string GetCppTemplateParameterList(const DescriptorProto& message) {
  if (!IsGenericMessage(message)) {
    return "";
  }

  std::vector<std::string> params;
  for (const auto& param : message.type_parameter()) {
    params.push_back(param.name());
  }

  return "<" + absl::StrJoin(params, ", ") + ">";
}

std::string GetCppTypeForParameter(const GenericTypeParameter& param) {
  if (param.has_constraint()) {
    // Handle constraint translation
    if (param.constraint() == "message") {
      return "google::protobuf::Message";
    }
    // For other constraints, use as-is (interface name)
    return param.constraint();
  }
  // No constraint - any type is allowed
  return "typename";
}

std::string GetCppDefaultType(const GenericTypeParameter& param) {
  if (!param.has_default_type()) {
    return "";
  }

  const std::string& default_type = param.default_type();

  // Map common protobuf types to C++ types
  if (default_type == "string") {
    return "std::string";
  } else if (default_type == "bytes") {
    return "std::string";
  } else if (default_type == "int32") {
    return "int32_t";
  } else if (default_type == "int64") {
    return "int64_t";
  } else if (default_type == "uint32") {
    return "uint32_t";
  } else if (default_type == "uint64") {
    return "uint64_t";
  } else if (default_type == "bool") {
    return "bool";
  } else if (default_type == "float") {
    return "float";
  } else if (default_type == "double") {
    return "double";
  }

  // For message types, return as-is
  return default_type;
}

void GenerateGenericMessageDeclaration(
    const DescriptorProto& message,
    const std::string& class_name,
    io::Printer* printer) {
  if (!IsGenericMessage(message)) {
    return;
  }

  printer->Print("// Generic message template\n");
  printer->Print("$template_decl$\n",
                 "template_decl", GetCppTemplateDeclaration(message));
  printer->Print("class $class_name$ : public ::google::protobuf::Message {\n",
                 "class_name", class_name);
  printer->Print(" public:\n");

  // Generate enum for oneof cases if needed
  for (const auto& oneof : message.oneof_decl()) {
    printer->Print("  enum $oneof_name$Case {\n",
                   "oneof_name", oneof.name());
    // Field cases would be added here
    printer->Print("    $upper_name$_NOT_SET = 0,\n",
                   "upper_name", oneof.name());
    printer->Print("  };\n\n");
  }

  // Generate field accessors for each type parameter
  for (const auto& field : message.field()) {
    if (field.has_generic_type_parameter()) {
      const std::string& type_param = field.generic_type_parameter();

      // Const accessor
      printer->Print(
          "  const $type$& $name$() const { return $name$_; }\n",
          "type", type_param, "name", field.name());

      // Mutable accessor
      printer->Print(
          "  $type$* mutable_$name$() { return &$name$_; }\n",
          "type", type_param, "name", field.name());

      // Setter
      printer->Print(
          "  void set_$name$(const $type$& value) { $name$_ = value; }\n",
          "type", type_param, "name", field.name());

      // Move setter
      printer->Print(
          "  void set_$name$($type$&& value) { $name$_ = std::move(value); }\n",
          "type", type_param, "name", field.name());
    }
  }

  printer->Print("\n private:\n");

  // Generate private members for each field
  for (const auto& field : message.field()) {
    if (field.has_generic_type_parameter()) {
      printer->Print("  $type$ $name$_;\n",
                     "type", field.generic_type_parameter(),
                     "name", field.name());
    }
  }

  printer->Print("};\n\n");
}

void GenerateGenericMessageForwardDeclaration(
    const DescriptorProto& message,
    const std::string& class_name,
    io::Printer* printer) {
  if (!IsGenericMessage(message)) {
    return;
  }

  printer->Print("$template_decl$ class $class_name$;\n",
                 "template_decl", GetCppTemplateDeclaration(message),
                 "class_name", class_name);
}

std::string GetCppFieldTypeName(
    const FieldDescriptorProto& field,
    const DescriptorProto& containing_message) {
  if (field.has_generic_type_parameter()) {
    return field.generic_type_parameter();
  }
  return field.type_name();
}

bool IsTemplateParameter(
    const FieldDescriptorProto& field,
    const DescriptorProto& containing_message) {
  if (!field.has_generic_type_parameter()) {
    return false;
  }

  const std::string& type_param = field.generic_type_parameter();
  for (const auto& param : containing_message.type_parameter()) {
    if (param.name() == type_param) {
      return true;
    }
  }
  return false;
}

std::string GetCppGenericAccessorSignature(
    const FieldDescriptorProto& field,
    const std::string& type_param_name,
    bool is_const) {
  if (is_const) {
    return absl::StrCat("const ", type_param_name, "& ", field.name(), "() const");
  } else {
    return absl::StrCat(type_param_name, "* mutable_", field.name(), "()");
  }
}

void GenerateGenericInstantiation(
    const std::string& generic_name,
    const std::vector<std::string>& type_arguments,
    const std::string& instantiated_name,
    io::Printer* printer) {
  std::string type_args = "<" + absl::StrJoin(type_arguments, ", ") + ">";
  printer->Print("using $instantiated$ = $generic$$type_args$;\n",
                 "instantiated", instantiated_name,
                 "generic", generic_name,
                 "type_args", type_args);
}

std::string ProtobufTypeToCppType(const std::string& proto_type) {
  if (proto_type == "string") {
    return "std::string";
  } else if (proto_type == "bytes") {
    return "std::string";
  } else if (proto_type == "int32") {
    return "int32_t";
  } else if (proto_type == "int64") {
    return "int64_t";
  } else if (proto_type == "uint32") {
    return "uint32_t";
  } else if (proto_type == "uint64") {
    return "uint64_t";
  } else if (proto_type == "sint32") {
    return "int32_t";
  } else if (proto_type == "sint64") {
    return "int64_t";
  } else if (proto_type == "fixed32") {
    return "uint32_t";
  } else if (proto_type == "fixed64") {
    return "uint64_t";
  } else if (proto_type == "sfixed32") {
    return "int32_t";
  } else if (proto_type == "sfixed64") {
    return "int64_t";
  } else if (proto_type == "bool") {
    return "bool";
  } else if (proto_type == "float") {
    return "float";
  } else if (proto_type == "double") {
    return "double";
  }

  // For message types, assume it's a fully qualified name
  return proto_type;
}

}  // namespace cpp
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
