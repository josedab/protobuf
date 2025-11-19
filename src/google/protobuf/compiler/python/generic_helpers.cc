// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/compiler/python/generic_helpers.h"

#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace python {

bool IsGenericMessage(const DescriptorProto& message) {
  return message.type_parameter_size() > 0;
}

std::string GetPythonGenericDeclaration(const DescriptorProto& message) {
  if (!IsGenericMessage(message)) {
    return "";
  }

  std::vector<std::string> params;
  for (const auto& param : message.type_parameter()) {
    params.push_back(param.name());
  }

  return "Generic[" + absl::StrJoin(params, ", ") + "]";
}

void GeneratePythonTypeVars(
    const DescriptorProto& message,
    io::Printer* printer) {
  if (!IsGenericMessage(message)) {
    return;
  }

  for (const auto& param : message.type_parameter()) {
    std::string bound;
    if (param.has_constraint()) {
      if (param.constraint() == "message") {
        bound = ", bound=google.protobuf.message.Message";
      } else {
        bound = absl::StrCat(", bound=", param.constraint());
      }
    }

    printer->Print(
        "$name$ = TypeVar('$name$'$bound$)\n",
        "name", param.name(),
        "bound", bound);
  }
  printer->Print("\n");
}

std::string GetPythonTypeHint(const GenericTypeParameter& param) {
  if (param.has_constraint()) {
    if (param.constraint() == "message") {
      return "google.protobuf.message.Message";
    }
    return param.constraint();
  }
  return "Any";
}

std::string GetPythonDefaultType(const GenericTypeParameter& param) {
  if (!param.has_default_type()) {
    return "";
  }
  return ProtobufTypeToPythonType(param.default_type());
}

void GeneratePythonGenericClass(
    const DescriptorProto& message,
    const std::string& class_name,
    io::Printer* printer) {
  if (!IsGenericMessage(message)) {
    return;
  }

  // Generate TypeVars
  GeneratePythonTypeVars(message, printer);

  // Generate class definition
  printer->Print(
      "class $class_name$($generic_decl$, google.protobuf.message.Message):\n",
      "class_name", class_name,
      "generic_decl", GetPythonGenericDeclaration(message));

  printer->Print("    \"\"\"Generic message: $class_name$\"\"\"\n\n",
                 "class_name", class_name);

  // Generate __init__
  printer->Print("    def __init__(self):\n");
  for (const auto& field : message.field()) {
    if (field.has_generic_type_parameter()) {
      printer->Print(
          "        self._$name$: Optional[$type$] = None\n",
          "name", field.name(),
          "type", field.generic_type_parameter());
    }
  }
  printer->Print("\n");

  // Generate properties for each generic field
  for (const auto& field : message.field()) {
    if (field.has_generic_type_parameter()) {
      GeneratePythonGenericProperty(field, field.generic_type_parameter(),
                                    printer);
    }
  }

  printer->Print("\n");
}

void GeneratePythonGenericProperty(
    const FieldDescriptorProto& field,
    const std::string& type_param_name,
    io::Printer* printer) {
  // Property getter
  printer->Print(
      "    @property\n"
      "    def $name$(self) -> $type$:\n"
      "        \"\"\"Gets the $name$ field.\"\"\"\n"
      "        return self._$name$\n\n",
      "name", field.name(),
      "type", type_param_name);

  // Property setter
  printer->Print(
      "    @$name$.setter\n"
      "    def $name$(self, value: $type$) -> None:\n"
      "        \"\"\"Sets the $name$ field.\"\"\"\n"
      "        self._$name$ = value\n\n",
      "name", field.name(),
      "type", type_param_name);
}

void GeneratePythonGenericStub(
    const DescriptorProto& message,
    const std::string& class_name,
    io::Printer* printer) {
  if (!IsGenericMessage(message)) {
    return;
  }

  // Import statements for typing
  printer->Print(
      "from typing import Generic, TypeVar, Optional\n"
      "import google.protobuf.message\n\n");

  // Generate TypeVars
  GeneratePythonTypeVars(message, printer);

  // Generate class stub
  printer->Print(
      "class $class_name$($generic_decl$, google.protobuf.message.Message):\n",
      "class_name", class_name,
      "generic_decl", GetPythonGenericDeclaration(message));

  // Generate property stubs
  for (const auto& field : message.field()) {
    if (field.has_generic_type_parameter()) {
      printer->Print(
          "    @property\n"
          "    def $name$(self) -> $type$: ...\n"
          "    @$name$.setter\n"
          "    def $name$(self, value: $type$) -> None: ...\n",
          "name", field.name(),
          "type", field.generic_type_parameter());
    }
  }

  printer->Print("\n");
}

std::string ProtobufTypeToPythonType(const std::string& proto_type) {
  if (proto_type == "string") {
    return "str";
  } else if (proto_type == "bytes") {
    return "bytes";
  } else if (proto_type == "int32" || proto_type == "sint32" ||
             proto_type == "sfixed32" || proto_type == "uint32" ||
             proto_type == "fixed32" || proto_type == "int64" ||
             proto_type == "sint64" || proto_type == "sfixed64" ||
             proto_type == "uint64" || proto_type == "fixed64") {
    return "int";
  } else if (proto_type == "bool") {
    return "bool";
  } else if (proto_type == "float" || proto_type == "double") {
    return "float";
  }

  // For message types, return as-is
  return proto_type;
}

std::string ProtobufTypeToPythonTyping(const std::string& proto_type) {
  if (proto_type == "string") {
    return "str";
  } else if (proto_type == "bytes") {
    return "bytes";
  } else if (proto_type == "int32" || proto_type == "sint32" ||
             proto_type == "sfixed32" || proto_type == "uint32" ||
             proto_type == "fixed32" || proto_type == "int64" ||
             proto_type == "sint64" || proto_type == "sfixed64" ||
             proto_type == "uint64" || proto_type == "fixed64") {
    return "int";
  } else if (proto_type == "bool") {
    return "bool";
  } else if (proto_type == "float" || proto_type == "double") {
    return "float";
  }

  // For message types, this would need proper import resolution
  return proto_type;
}

}  // namespace python
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
