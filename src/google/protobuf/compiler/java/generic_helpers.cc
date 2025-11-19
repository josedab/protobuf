// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/compiler/java/generic_helpers.h"

#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace java {

bool IsGenericMessage(const DescriptorProto& message) {
  return message.type_parameter_size() > 0;
}

std::string GetJavaGenericDeclaration(const DescriptorProto& message) {
  if (!IsGenericMessage(message)) {
    return "";
  }

  std::vector<std::string> params;
  for (const auto& param : message.type_parameter()) {
    std::string param_decl = param.name();
    if (param.has_constraint()) {
      param_decl += " extends " + GetJavaTypeForConstraint(param);
    } else {
      // Default to Message for message fields
      param_decl += " extends com.google.protobuf.Message";
    }
    params.push_back(param_decl);
  }

  return "<" + absl::StrJoin(params, ", ") + ">";
}

std::string GetJavaTypeParameterList(const DescriptorProto& message) {
  if (!IsGenericMessage(message)) {
    return "";
  }

  std::vector<std::string> params;
  for (const auto& param : message.type_parameter()) {
    params.push_back(param.name());
  }

  return "<" + absl::StrJoin(params, ", ") + ">";
}

std::string GetJavaTypeForConstraint(const GenericTypeParameter& param) {
  if (!param.has_constraint()) {
    return "com.google.protobuf.Message";
  }

  const std::string& constraint = param.constraint();
  if (constraint == "message") {
    return "com.google.protobuf.Message";
  }
  // Return the constraint as a Java type name
  return constraint;
}

std::string GetJavaDefaultType(const GenericTypeParameter& param) {
  if (!param.has_default_type()) {
    return "";
  }

  return ProtobufTypeToJavaType(param.default_type());
}

void GenerateJavaGenericClassDeclaration(
    const DescriptorProto& message,
    const std::string& class_name,
    io::Printer* printer) {
  if (!IsGenericMessage(message)) {
    return;
  }

  std::string generic_decl = GetJavaGenericDeclaration(message);
  std::string type_params = GetJavaTypeParameterList(message);

  printer->Print(
      "/**\n"
      " * Generic message: $class_name$$generic_decl$\n"
      " */\n",
      "class_name", class_name,
      "generic_decl", generic_decl);

  printer->Print(
      "public class $class_name$$generic_decl$ extends\n"
      "    com.google.protobuf.GeneratedMessage {\n",
      "class_name", class_name,
      "generic_decl", generic_decl);

  // Generate private fields for type parameters
  for (const auto& field : message.field()) {
    if (field.has_generic_type_parameter()) {
      printer->Print(
          "  private $type$ $name$_;\n",
          "type", field.generic_type_parameter(),
          "name", field.name());
    }
  }

  printer->Print("\n");

  // Generate getters and setters for each generic field
  for (const auto& field : message.field()) {
    if (field.has_generic_type_parameter()) {
      GenerateJavaGenericGetter(field, field.generic_type_parameter(), printer);
      printer->Print("\n");
    }
  }

  // Generate Builder class
  GenerateJavaGenericBuilderMethods(message, class_name, printer);

  printer->Print("}\n\n");
}

void GenerateJavaGenericGetter(
    const FieldDescriptorProto& field,
    const std::string& type_param_name,
    io::Printer* printer) {
  printer->Print(
      "  /**\n"
      "   * Gets the $name$ field.\n"
      "   */\n"
      "  public $type$ get$capitalized_name$() {\n"
      "    return $name$_;\n"
      "  }\n",
      "name", field.name(),
      "type", type_param_name,
      "capitalized_name", field.name());  // Should capitalize first letter
}

void GenerateJavaGenericSetter(
    const FieldDescriptorProto& field,
    const std::string& type_param_name,
    io::Printer* printer) {
  printer->Print(
      "  /**\n"
      "   * Sets the $name$ field.\n"
      "   */\n"
      "  public Builder$type_params$ set$capitalized_name$($type$ value) {\n"
      "    $name$_ = value;\n"
      "    return this;\n"
      "  }\n",
      "name", field.name(),
      "type", type_param_name,
      "type_params", "",  // Should be filled with actual type params
      "capitalized_name", field.name());  // Should capitalize first letter
}

void GenerateJavaGenericBuilderMethods(
    const DescriptorProto& message,
    const std::string& class_name,
    io::Printer* printer) {
  std::string type_params = GetJavaTypeParameterList(message);

  printer->Print(
      "  /**\n"
      "   * Builder for $class_name$.\n"
      "   */\n"
      "  public static class Builder$generic_decl$ extends\n"
      "      com.google.protobuf.GeneratedMessage.Builder<Builder$type_params$> {\n",
      "class_name", class_name,
      "generic_decl", GetJavaGenericDeclaration(message),
      "type_params", type_params);

  // Generate builder fields and methods
  for (const auto& field : message.field()) {
    if (field.has_generic_type_parameter()) {
      printer->Print(
          "    private $type$ $name$_;\n",
          "type", field.generic_type_parameter(),
          "name", field.name());
    }
  }

  printer->Print("\n");

  // Generate builder setters
  for (const auto& field : message.field()) {
    if (field.has_generic_type_parameter()) {
      GenerateJavaGenericSetter(field, field.generic_type_parameter(), printer);
      printer->Print("\n");
    }
  }

  // Generate build method
  printer->Print(
      "    public $class_name$$type_params$ build() {\n"
      "      $class_name$$type_params$ result = new $class_name$$type_params$();\n",
      "class_name", class_name,
      "type_params", type_params);

  for (const auto& field : message.field()) {
    if (field.has_generic_type_parameter()) {
      printer->Print(
          "      result.$name$_ = $name$_;\n",
          "name", field.name());
    }
  }

  printer->Print(
      "      return result;\n"
      "    }\n"
      "  }\n\n");

  // Generate newBuilder method
  printer->Print(
      "  public static $generic_decl$ Builder$type_params$ newBuilder() {\n"
      "    return new Builder$type_params$();\n"
      "  }\n",
      "generic_decl", GetJavaGenericDeclaration(message),
      "type_params", type_params);
}

std::string ProtobufTypeToJavaType(const std::string& proto_type) {
  if (proto_type == "string") {
    return "String";
  } else if (proto_type == "bytes") {
    return "com.google.protobuf.ByteString";
  } else if (proto_type == "int32" || proto_type == "sint32" ||
             proto_type == "sfixed32") {
    return "int";
  } else if (proto_type == "int64" || proto_type == "sint64" ||
             proto_type == "sfixed64") {
    return "long";
  } else if (proto_type == "uint32" || proto_type == "fixed32") {
    return "int";
  } else if (proto_type == "uint64" || proto_type == "fixed64") {
    return "long";
  } else if (proto_type == "bool") {
    return "boolean";
  } else if (proto_type == "float") {
    return "float";
  } else if (proto_type == "double") {
    return "double";
  }

  // For message types, assume it's a Java class name
  return proto_type;
}

std::string GetBoxedJavaType(const std::string& java_type) {
  if (java_type == "int") {
    return "Integer";
  } else if (java_type == "long") {
    return "Long";
  } else if (java_type == "boolean") {
    return "Boolean";
  } else if (java_type == "float") {
    return "Float";
  } else if (java_type == "double") {
    return "Double";
  } else if (java_type == "byte") {
    return "Byte";
  } else if (java_type == "short") {
    return "Short";
  } else if (java_type == "char") {
    return "Character";
  }
  return java_type;
}

}  // namespace java
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
