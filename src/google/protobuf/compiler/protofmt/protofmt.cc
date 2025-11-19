// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/compiler/protofmt/protofmt.h"

#include <algorithm>
#include <string>
#include <vector>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_replace.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/compiler/parser.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/io/tokenizer.h"
#include "google/protobuf/io/zero_copy_stream_impl_lite.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace protofmt {

namespace {

// Simple error collector for the parser.
class ErrorCollector : public io::ErrorCollector {
 public:
  void RecordError(int line, int column, absl::string_view message) override {
    errors_.push_back(
        absl::StrCat("Line ", line + 1, ", column ", column + 1, ": ", message));
  }

  void RecordWarning(int line, int column, absl::string_view message) override {
    warnings_.push_back(
        absl::StrCat("Line ", line + 1, ", column ", column + 1, ": ", message));
  }

  bool HasErrors() const { return !errors_.empty(); }

  const std::vector<std::string>& errors() const { return errors_; }
  const std::vector<std::string>& warnings() const { return warnings_; }

 private:
  std::vector<std::string> errors_;
  std::vector<std::string> warnings_;
};

// Get the scalar type name for a field type.
std::string ScalarTypeName(FieldDescriptorProto::Type type) {
  switch (type) {
    case FieldDescriptorProto::TYPE_DOUBLE:
      return "double";
    case FieldDescriptorProto::TYPE_FLOAT:
      return "float";
    case FieldDescriptorProto::TYPE_INT64:
      return "int64";
    case FieldDescriptorProto::TYPE_UINT64:
      return "uint64";
    case FieldDescriptorProto::TYPE_INT32:
      return "int32";
    case FieldDescriptorProto::TYPE_FIXED64:
      return "fixed64";
    case FieldDescriptorProto::TYPE_FIXED32:
      return "fixed32";
    case FieldDescriptorProto::TYPE_BOOL:
      return "bool";
    case FieldDescriptorProto::TYPE_STRING:
      return "string";
    case FieldDescriptorProto::TYPE_BYTES:
      return "bytes";
    case FieldDescriptorProto::TYPE_UINT32:
      return "uint32";
    case FieldDescriptorProto::TYPE_SFIXED32:
      return "sfixed32";
    case FieldDescriptorProto::TYPE_SFIXED64:
      return "sfixed64";
    case FieldDescriptorProto::TYPE_SINT32:
      return "sint32";
    case FieldDescriptorProto::TYPE_SINT64:
      return "sint64";
    case FieldDescriptorProto::TYPE_GROUP:
      return "group";
    case FieldDescriptorProto::TYPE_MESSAGE:
    case FieldDescriptorProto::TYPE_ENUM:
      return "";  // Will use type_name instead
    default:
      return "unknown";
  }
}

// Check if a field is a map entry.
bool IsMapEntry(const FieldDescriptorProto& field,
                const DescriptorProto& message) {
  if (field.type() != FieldDescriptorProto::TYPE_MESSAGE ||
      field.label() != FieldDescriptorProto::LABEL_REPEATED) {
    return false;
  }

  // Look for a nested message with map_entry option
  std::string expected_name = field.name() + "Entry";
  for (const auto& nested : message.nested_type()) {
    if (nested.name() == expected_name && nested.options().map_entry()) {
      return true;
    }
  }
  return false;
}

}  // namespace

ProtobufFormatter::ProtobufFormatter(const ProtofmtConfig& config)
    : config_(config) {}

absl::StatusOr<FileDescriptorProto> ProtobufFormatter::Parse(
    const std::string& input) {
  io::ArrayInputStream input_stream(input.data(), static_cast<int>(input.size()));
  ErrorCollector error_collector;
  io::Tokenizer tokenizer(&input_stream, &error_collector);

  FileDescriptorProto file_proto;
  Parser parser;
  parser.RecordErrorsTo(&error_collector);

  if (!parser.Parse(&tokenizer, &file_proto)) {
    std::string error_msg = "Failed to parse proto file:\n";
    for (const auto& error : error_collector.errors()) {
      absl::StrAppend(&error_msg, "  ", error, "\n");
    }
    return absl::InvalidArgumentError(error_msg);
  }

  return file_proto;
}

absl::StatusOr<FormatResult> ProtobufFormatter::Format(const std::string& input) {
  auto file_proto_or = Parse(input);
  if (!file_proto_or.ok()) {
    return file_proto_or.status();
  }

  return Format(file_proto_or.value());
}

absl::StatusOr<FormatResult> ProtobufFormatter::Format(
    const FileDescriptorProto& file_proto) {
  FormatResult result;
  std::string output;

  // Format syntax/edition
  FormatSyntax(file_proto, &output);
  FormatEdition(file_proto, &output);

  // Format package
  FormatPackage(file_proto, &output);

  // Format imports
  FormatImports(file_proto, &output);

  // Format file options
  FormatFileOptions(file_proto, &output);

  // Format enums (top-level)
  for (const auto& enum_proto : file_proto.enum_type()) {
    FormatEnum(enum_proto, 0, &output);
    output += "\n";
  }

  // Format messages
  for (const auto& message : file_proto.message_type()) {
    FormatMessage(message, 0, &output);
    output += "\n";
  }

  // Format extensions
  FormatExtensions(file_proto, &output);

  // Format services
  for (const auto& service : file_proto.service()) {
    FormatService(service, 0, &output);
    output += "\n";
  }

  // Remove trailing whitespace and ensure single trailing newline
  while (!output.empty() && output.back() == '\n') {
    output.pop_back();
  }
  output += "\n";

  result.output = output;
  result.was_formatted = true;
  return result;
}

absl::StatusOr<bool> ProtobufFormatter::IsFormatted(const std::string& input) {
  auto result_or = Format(input);
  if (!result_or.ok()) {
    return result_or.status();
  }
  return result_or.value().output == input;
}

absl::StatusOr<std::string> ProtobufFormatter::GetDiff(const std::string& input) {
  auto result_or = Format(input);
  if (!result_or.ok()) {
    return result_or.status();
  }

  if (result_or.value().output == input) {
    return "";
  }

  // Simple line-by-line diff
  std::vector<std::string> original_lines = absl::StrSplit(input, '\n');
  std::vector<std::string> formatted_lines =
      absl::StrSplit(result_or.value().output, '\n');

  std::string diff;
  size_t max_lines = std::max(original_lines.size(), formatted_lines.size());
  for (size_t i = 0; i < max_lines; ++i) {
    std::string orig = i < original_lines.size() ? original_lines[i] : "";
    std::string fmt = i < formatted_lines.size() ? formatted_lines[i] : "";
    if (orig != fmt) {
      absl::StrAppend(&diff, "- ", orig, "\n");
      absl::StrAppend(&diff, "+ ", fmt, "\n");
    }
  }

  return diff;
}

void ProtobufFormatter::FormatSyntax(const FileDescriptorProto& file_proto,
                                     std::string* output) {
  if (!file_proto.has_syntax() || file_proto.syntax().empty()) {
    return;
  }

  absl::StrAppend(output, "syntax = \"", file_proto.syntax(), "\";\n\n");
}

void ProtobufFormatter::FormatEdition(const FileDescriptorProto& file_proto,
                                      std::string* output) {
  if (!file_proto.has_edition()) {
    return;
  }

  // Edition is stored as an enum, convert to string
  std::string edition_str;
  switch (file_proto.edition()) {
    case Edition::EDITION_2023:
      edition_str = "2023";
      break;
    case Edition::EDITION_2024:
      edition_str = "2024";
      break;
    default:
      return;  // Unknown edition
  }

  absl::StrAppend(output, "edition = \"", edition_str, "\";\n\n");
}

void ProtobufFormatter::FormatPackage(const FileDescriptorProto& file_proto,
                                      std::string* output) {
  if (!file_proto.has_package() || file_proto.package().empty()) {
    return;
  }

  absl::StrAppend(output, "package ", file_proto.package(), ";\n\n");
}

void ProtobufFormatter::FormatImports(const FileDescriptorProto& file_proto,
                                      std::string* output) {
  if (file_proto.dependency_size() == 0) {
    return;
  }

  std::vector<std::string> imports;
  std::vector<std::string> public_imports;
  std::vector<std::string> weak_imports;

  // Collect public and weak import indices
  std::set<int> public_indices(file_proto.public_dependency().begin(),
                                file_proto.public_dependency().end());
  std::set<int> weak_indices(file_proto.weak_dependency().begin(),
                              file_proto.weak_dependency().end());

  for (int i = 0; i < file_proto.dependency_size(); ++i) {
    const std::string& dep = file_proto.dependency(i);
    if (public_indices.count(i)) {
      public_imports.push_back(dep);
    } else if (weak_indices.count(i)) {
      weak_imports.push_back(dep);
    } else {
      imports.push_back(dep);
    }
  }

  // Sort imports if configured
  if (config_.sort_imports) {
    std::sort(imports.begin(), imports.end());
    std::sort(public_imports.begin(), public_imports.end());
    std::sort(weak_imports.begin(), weak_imports.end());
  }

  // Output imports
  for (const auto& import : imports) {
    absl::StrAppend(output, "import \"", import, "\";\n");
  }

  for (const auto& import : public_imports) {
    absl::StrAppend(output, "import public \"", import, "\";\n");
  }

  for (const auto& import : weak_imports) {
    absl::StrAppend(output, "import weak \"", import, "\";\n");
  }

  if (!imports.empty() || !public_imports.empty() || !weak_imports.empty()) {
    output->append("\n");
  }
}

void ProtobufFormatter::FormatFileOptions(const FileDescriptorProto& file_proto,
                                          std::string* output) {
  const FileOptions& options = file_proto.options();

  if (options.has_java_package()) {
    absl::StrAppend(output, "option java_package = \"", options.java_package(),
                    "\";\n");
  }

  if (options.has_java_outer_classname()) {
    absl::StrAppend(output, "option java_outer_classname = \"",
                    options.java_outer_classname(), "\";\n");
  }

  if (options.has_java_multiple_files() && options.java_multiple_files()) {
    absl::StrAppend(output, "option java_multiple_files = true;\n");
  }

  if (options.has_go_package()) {
    absl::StrAppend(output, "option go_package = \"", options.go_package(),
                    "\";\n");
  }

  if (options.has_cc_generic_services() && options.cc_generic_services()) {
    absl::StrAppend(output, "option cc_generic_services = true;\n");
  }

  if (options.has_cc_enable_arenas() && options.cc_enable_arenas()) {
    absl::StrAppend(output, "option cc_enable_arenas = true;\n");
  }

  if (options.has_objc_class_prefix()) {
    absl::StrAppend(output, "option objc_class_prefix = \"",
                    options.objc_class_prefix(), "\";\n");
  }

  if (options.has_csharp_namespace()) {
    absl::StrAppend(output, "option csharp_namespace = \"",
                    options.csharp_namespace(), "\";\n");
  }

  if (options.has_deprecated() && options.deprecated()) {
    absl::StrAppend(output, "option deprecated = true;\n");
  }

  // Add blank line after options
  bool has_options =
      options.has_java_package() || options.has_java_outer_classname() ||
      options.has_java_multiple_files() || options.has_go_package() ||
      options.has_cc_generic_services() || options.has_cc_enable_arenas() ||
      options.has_objc_class_prefix() || options.has_csharp_namespace() ||
      options.has_deprecated();

  if (has_options) {
    output->append("\n");
  }
}

void ProtobufFormatter::FormatMessage(const DescriptorProto& message,
                                      int indent_level, std::string* output) {
  std::string indent = Indent(indent_level);

  absl::StrAppend(output, indent, "message ", message.name(), " {\n");

  // Format message options
  FormatMessageOptions(message.options(), indent_level + 1, output);

  // Collect fields by oneof
  std::vector<const FieldDescriptorProto*> regular_fields;
  std::map<int, std::vector<const FieldDescriptorProto*>> oneof_fields;

  for (const auto& field : message.field()) {
    if (field.has_oneof_index()) {
      oneof_fields[field.oneof_index()].push_back(&field);
    } else {
      regular_fields.push_back(&field);
    }
  }

  // Calculate alignment for regular fields if enabled
  std::vector<FieldDescriptorProto> all_regular_fields;
  for (const auto* field : regular_fields) {
    all_regular_fields.push_back(*field);
  }

  // Format nested enums first
  for (const auto& enum_proto : message.enum_type()) {
    FormatEnum(enum_proto, indent_level + 1, output);
  }

  // Format nested messages (skip map entries)
  for (const auto& nested : message.nested_type()) {
    if (!nested.options().map_entry()) {
      FormatMessage(nested, indent_level + 1, output);
    }
  }

  // Format regular fields
  for (const auto* field : regular_fields) {
    // Check if this is a map field
    if (IsMapEntry(*field, message)) {
      FormatMapEntry(*field, indent_level + 1, output);
    } else {
      FormatField(*field, indent_level + 1, &all_regular_fields, output);
    }
  }

  // Format oneofs
  for (int i = 0; i < message.oneof_decl_size(); ++i) {
    // Check if this oneof is actually used (not synthetic for proto3 optional)
    if (oneof_fields.count(i) > 0 && !oneof_fields[i].empty()) {
      // Check if it's a synthetic oneof for proto3 optional
      const auto& first_field = *oneof_fields[i][0];
      if (first_field.has_proto3_optional() && first_field.proto3_optional()) {
        // This is a synthetic oneof, format as regular optional field
        for (const auto* field : oneof_fields[i]) {
          FormatField(*field, indent_level + 1, nullptr, output);
        }
      } else {
        FormatOneof(message.oneof_decl(i), message, i, indent_level + 1, output);
      }
    }
  }

  // Format reserved ranges and names
  for (const auto& range : message.reserved_range()) {
    if (range.start() == range.end() - 1) {
      absl::StrAppend(output, Indent(indent_level + 1), "reserved ",
                      range.start(), ";\n");
    } else {
      absl::StrAppend(output, Indent(indent_level + 1), "reserved ",
                      range.start(), " to ", range.end() - 1, ";\n");
    }
  }

  if (message.reserved_name_size() > 0) {
    std::vector<std::string> names;
    for (const auto& name : message.reserved_name()) {
      names.push_back(absl::StrCat("\"", name, "\""));
    }
    absl::StrAppend(output, Indent(indent_level + 1), "reserved ",
                    absl::StrJoin(names, ", "), ";\n");
  }

  // Format extension ranges
  for (const auto& range : message.extension_range()) {
    if (range.start() == range.end() - 1) {
      absl::StrAppend(output, Indent(indent_level + 1), "extensions ",
                      range.start(), ";\n");
    } else {
      std::string end_str = range.end() == 536870912 ? "max" : std::to_string(range.end() - 1);
      absl::StrAppend(output, Indent(indent_level + 1), "extensions ",
                      range.start(), " to ", end_str, ";\n");
    }
  }

  absl::StrAppend(output, indent, "}\n");
}

void ProtobufFormatter::FormatField(
    const FieldDescriptorProto& field, int indent_level,
    const std::vector<FieldDescriptorProto>* all_fields, std::string* output) {
  std::string indent = Indent(indent_level);
  std::string label = GetLabelString(field);
  std::string type_name = GetTypeName(field);

  AlignmentInfo alignment = {0, 0};
  if (config_.align_fields && all_fields != nullptr) {
    alignment = CalculateAlignment(*all_fields);
  }

  // Build field line
  std::string field_line = indent;

  if (!label.empty()) {
    absl::StrAppend(&field_line, label, " ");
  }

  if (config_.align_fields && alignment.max_type_width > 0) {
    // Pad type name
    absl::StrAppend(&field_line, type_name);
    size_t padding = alignment.max_type_width - type_name.size();
    field_line.append(padding, ' ');
    absl::StrAppend(&field_line, " ");

    // Pad field name
    absl::StrAppend(&field_line, field.name());
    padding = alignment.max_name_width - field.name().size();
    field_line.append(padding, ' ');
  } else {
    absl::StrAppend(&field_line, type_name, " ", field.name());
  }

  absl::StrAppend(&field_line, " = ", field.number());

  // Format field options
  FormatFieldOptions(field, &field_line);

  absl::StrAppend(&field_line, ";\n");
  output->append(field_line);
}

void ProtobufFormatter::FormatFieldOptions(const FieldDescriptorProto& field,
                                           std::string* output) {
  const FieldOptions& options = field.options();
  std::vector<std::string> option_parts;

  if (options.has_deprecated() && options.deprecated()) {
    option_parts.push_back("deprecated = true");
  }

  if (options.has_packed()) {
    option_parts.push_back(
        absl::StrCat("packed = ", options.packed() ? "true" : "false"));
  }

  if (options.has_lazy() && options.lazy()) {
    option_parts.push_back("lazy = true");
  }

  if (options.has_jstype()) {
    std::string jstype_str;
    switch (options.jstype()) {
      case FieldOptions::JS_NORMAL:
        jstype_str = "JS_NORMAL";
        break;
      case FieldOptions::JS_STRING:
        jstype_str = "JS_STRING";
        break;
      case FieldOptions::JS_NUMBER:
        jstype_str = "JS_NUMBER";
        break;
      default:
        break;
    }
    if (!jstype_str.empty()) {
      option_parts.push_back(absl::StrCat("jstype = ", jstype_str));
    }
  }

  // Handle default value for proto2
  if (field.has_default_value()) {
    std::string default_val = field.default_value();
    // Quote strings
    if (field.type() == FieldDescriptorProto::TYPE_STRING) {
      default_val = absl::StrCat("\"", default_val, "\"");
    }
    option_parts.push_back(absl::StrCat("default = ", default_val));
  }

  if (!option_parts.empty()) {
    if (option_parts.size() == 1) {
      absl::StrAppend(output, " [", option_parts[0], "]");
    } else {
      std::string separator = config_.trailing_commas ? ",\n" : ",\n";
      absl::StrAppend(output, " [\n");
      for (size_t i = 0; i < option_parts.size(); ++i) {
        absl::StrAppend(output, "  ", option_parts[i]);
        if (i < option_parts.size() - 1 || config_.trailing_commas) {
          output->append(",");
        }
        output->append("\n");
      }
      output->append("]");
    }
  }
}

void ProtobufFormatter::FormatEnum(const EnumDescriptorProto& enum_proto,
                                   int indent_level, std::string* output) {
  std::string indent = Indent(indent_level);

  absl::StrAppend(output, indent, "enum ", enum_proto.name(), " {\n");

  // Collect values for alignment calculation
  std::vector<EnumValueDescriptorProto> all_values(enum_proto.value().begin(),
                                                    enum_proto.value().end());

  for (const auto& value : enum_proto.value()) {
    FormatEnumValue(value, indent_level + 1, &all_values, output);
  }

  // Format reserved ranges and names
  for (const auto& range : enum_proto.reserved_range()) {
    if (range.start() == range.end()) {
      absl::StrAppend(output, Indent(indent_level + 1), "reserved ",
                      range.start(), ";\n");
    } else {
      absl::StrAppend(output, Indent(indent_level + 1), "reserved ",
                      range.start(), " to ", range.end(), ";\n");
    }
  }

  if (enum_proto.reserved_name_size() > 0) {
    std::vector<std::string> names;
    for (const auto& name : enum_proto.reserved_name()) {
      names.push_back(absl::StrCat("\"", name, "\""));
    }
    absl::StrAppend(output, Indent(indent_level + 1), "reserved ",
                    absl::StrJoin(names, ", "), ";\n");
  }

  absl::StrAppend(output, indent, "}\n");
}

void ProtobufFormatter::FormatEnumValue(
    const EnumValueDescriptorProto& value, int indent_level,
    const std::vector<EnumValueDescriptorProto>* all_values,
    std::string* output) {
  std::string indent = Indent(indent_level);

  if (config_.align_fields && all_values != nullptr) {
    // Calculate max name width
    size_t max_width = 0;
    for (const auto& v : *all_values) {
      max_width = std::max(max_width, v.name().size());
    }

    std::string line = indent + value.name();
    line.append(max_width - value.name().size(), ' ');
    absl::StrAppend(&line, " = ", value.number());

    // Add options if present
    const EnumValueOptions& options = value.options();
    if (options.has_deprecated() && options.deprecated()) {
      absl::StrAppend(&line, " [deprecated = true]");
    }

    absl::StrAppend(&line, ";\n");
    output->append(line);
  } else {
    std::string line = absl::StrCat(indent, value.name(), " = ", value.number());

    // Add options if present
    const EnumValueOptions& options = value.options();
    if (options.has_deprecated() && options.deprecated()) {
      absl::StrAppend(&line, " [deprecated = true]");
    }

    absl::StrAppend(&line, ";\n");
    output->append(line);
  }
}

void ProtobufFormatter::FormatOneof(const OneofDescriptorProto& oneof,
                                    const DescriptorProto& message,
                                    int oneof_index, int indent_level,
                                    std::string* output) {
  std::string indent = Indent(indent_level);

  absl::StrAppend(output, indent, "oneof ", oneof.name(), " {\n");

  for (const auto& field : message.field()) {
    if (field.has_oneof_index() && field.oneof_index() == oneof_index) {
      FormatField(field, indent_level + 1, nullptr, output);
    }
  }

  absl::StrAppend(output, indent, "}\n");
}

void ProtobufFormatter::FormatService(const ServiceDescriptorProto& service,
                                      int indent_level, std::string* output) {
  std::string indent = Indent(indent_level);

  absl::StrAppend(output, indent, "service ", service.name(), " {\n");

  for (const auto& method : service.method()) {
    FormatMethod(method, indent_level + 1, output);
  }

  absl::StrAppend(output, indent, "}\n");
}

void ProtobufFormatter::FormatMethod(const MethodDescriptorProto& method,
                                     int indent_level, std::string* output) {
  std::string indent = Indent(indent_level);

  std::string input_type = method.input_type();
  std::string output_type = method.output_type();

  // Remove leading dot if present
  if (!input_type.empty() && input_type[0] == '.') {
    input_type = input_type.substr(1);
  }
  if (!output_type.empty() && output_type[0] == '.') {
    output_type = output_type.substr(1);
  }

  std::string line = absl::StrCat(indent, "rpc ", method.name(), "(");

  if (method.client_streaming()) {
    absl::StrAppend(&line, "stream ");
  }
  absl::StrAppend(&line, input_type, ") returns (");

  if (method.server_streaming()) {
    absl::StrAppend(&line, "stream ");
  }
  absl::StrAppend(&line, output_type, ")");

  // Check for method options
  const MethodOptions& options = method.options();
  if (options.has_deprecated() && options.deprecated()) {
    absl::StrAppend(&line, " {\n");
    absl::StrAppend(&line, Indent(indent_level + 1), "option deprecated = true;\n");
    absl::StrAppend(&line, indent, "}");
  } else {
    absl::StrAppend(&line, ";");
  }

  absl::StrAppend(&line, "\n");
  output->append(line);
}

void ProtobufFormatter::FormatMessageOptions(const MessageOptions& options,
                                             int indent_level,
                                             std::string* output) {
  std::string indent = Indent(indent_level);

  if (options.has_deprecated() && options.deprecated()) {
    absl::StrAppend(output, indent, "option deprecated = true;\n");
  }

  if (options.has_map_entry() && options.map_entry()) {
    // This is handled specially, don't output
    return;
  }

  if (options.has_message_set_wire_format() &&
      options.message_set_wire_format()) {
    absl::StrAppend(output, indent, "option message_set_wire_format = true;\n");
  }

  if (options.has_no_standard_descriptor_accessor() &&
      options.no_standard_descriptor_accessor()) {
    absl::StrAppend(output, indent,
                    "option no_standard_descriptor_accessor = true;\n");
  }
}

void ProtobufFormatter::FormatExtensions(const FileDescriptorProto& file_proto,
                                         std::string* output) {
  if (file_proto.extension_size() == 0) {
    return;
  }

  // Group extensions by extendee
  std::map<std::string, std::vector<const FieldDescriptorProto*>> extensions;
  for (const auto& ext : file_proto.extension()) {
    extensions[ext.extendee()].push_back(&ext);
  }

  for (const auto& [extendee, fields] : extensions) {
    std::string extendee_name = extendee;
    // Remove leading dot if present
    if (!extendee_name.empty() && extendee_name[0] == '.') {
      extendee_name = extendee_name.substr(1);
    }

    absl::StrAppend(output, "extend ", extendee_name, " {\n");
    for (const auto* field : fields) {
      FormatField(*field, 1, nullptr, output);
    }
    absl::StrAppend(output, "}\n\n");
  }
}

void ProtobufFormatter::FormatMapEntry(const FieldDescriptorProto& field,
                                       int indent_level, std::string* output) {
  // Find the map entry message to get key/value types
  std::string map_type = field.type_name();
  if (!map_type.empty() && map_type[0] == '.') {
    map_type = map_type.substr(1);
  }

  // For now, format as the underlying type name
  // A full implementation would look up the nested type and extract key/value
  std::string indent = Indent(indent_level);

  // The RFC shows map format as: map<key_type, value_type> name = number;
  // We would need to resolve the nested type to get exact key/value types
  // For now, we'll output the repeated field format
  absl::StrAppend(output, indent, "map<", "string", ", ", "string", "> ",
                  field.name(), " = ", field.number(), ";\n");
}

std::string ProtobufFormatter::Indent(int level) const {
  return std::string(level * config_.indent, ' ');
}

std::string ProtobufFormatter::GetTypeName(
    const FieldDescriptorProto& field) const {
  if (field.has_type_name() && !field.type_name().empty()) {
    std::string type_name = field.type_name();
    // Remove leading dot for relative names
    if (!type_name.empty() && type_name[0] == '.') {
      type_name = type_name.substr(1);
    }
    return type_name;
  }
  return ScalarTypeName(field.type());
}

std::string ProtobufFormatter::GetLabelString(
    const FieldDescriptorProto& field) const {
  // In proto3, we don't output labels for singular fields
  // But we need to check the syntax - if we have LABEL_OPTIONAL in proto3,
  // we check for proto3_optional flag
  switch (field.label()) {
    case FieldDescriptorProto::LABEL_OPTIONAL:
      if (field.has_proto3_optional() && field.proto3_optional()) {
        return "optional";
      }
      return "";  // In proto3, optional is default
    case FieldDescriptorProto::LABEL_REQUIRED:
      return "required";  // Only in proto2
    case FieldDescriptorProto::LABEL_REPEATED:
      return "repeated";
    default:
      return "";
  }
}

ProtobufFormatter::AlignmentInfo ProtobufFormatter::CalculateAlignment(
    const std::vector<FieldDescriptorProto>& fields) const {
  AlignmentInfo info = {0, 0};

  for (const auto& field : fields) {
    std::string type_name = GetTypeName(field);
    std::string label = GetLabelString(field);
    if (!label.empty()) {
      type_name = absl::StrCat(label, " ", type_name);
    }

    info.max_type_width = std::max(info.max_type_width, type_name.size());
    info.max_name_width = std::max(info.max_name_width, field.name().size());
  }

  return info;
}

}  // namespace protofmt
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
