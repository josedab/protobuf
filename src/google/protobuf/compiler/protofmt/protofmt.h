// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Author: protofmt authors
//
// Main formatter interface for Protocol Buffer schema files.

#ifndef GOOGLE_PROTOBUF_COMPILER_PROTOFMT_PROTOFMT_H__
#define GOOGLE_PROTOBUF_COMPILER_PROTOFMT_PROTOFMT_H__

#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "google/protobuf/compiler/protofmt/protofmt_config.h"
#include "google/protobuf/descriptor.pb.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace protofmt {

// Result of formatting a file.
struct FormatResult {
  // The formatted output.
  std::string output;

  // Whether the file was already formatted correctly.
  bool was_formatted;

  // Any warnings generated during formatting.
  std::vector<std::string> warnings;
};

// The main protobuf formatter class.
// This class formats .proto files according to the configured style rules.
class ProtobufFormatter {
 public:
  explicit ProtobufFormatter(const ProtofmtConfig& config);

  // Format a proto file from its content.
  // Returns the formatted content or an error status.
  absl::StatusOr<FormatResult> Format(const std::string& input);

  // Format a FileDescriptorProto.
  // This is useful when you already have a parsed proto.
  absl::StatusOr<FormatResult> Format(const FileDescriptorProto& file_proto);

  // Check if the input is already formatted correctly.
  // Returns true if no formatting changes are needed.
  absl::StatusOr<bool> IsFormatted(const std::string& input);

  // Get the diff between the input and formatted output.
  // Returns an empty string if no differences.
  absl::StatusOr<std::string> GetDiff(const std::string& input);

 private:
  // Format the syntax declaration.
  void FormatSyntax(const FileDescriptorProto& file_proto, std::string* output);

  // Format the edition declaration.
  void FormatEdition(const FileDescriptorProto& file_proto, std::string* output);

  // Format the package declaration.
  void FormatPackage(const FileDescriptorProto& file_proto, std::string* output);

  // Format import statements.
  void FormatImports(const FileDescriptorProto& file_proto, std::string* output);

  // Format file-level options.
  void FormatFileOptions(const FileDescriptorProto& file_proto,
                         std::string* output);

  // Format a message type.
  void FormatMessage(const DescriptorProto& message, int indent_level,
                     std::string* output);

  // Format a field.
  void FormatField(const FieldDescriptorProto& field, int indent_level,
                   const std::vector<FieldDescriptorProto>* all_fields,
                   std::string* output);

  // Format an enum type.
  void FormatEnum(const EnumDescriptorProto& enum_proto, int indent_level,
                  std::string* output);

  // Format an enum value.
  void FormatEnumValue(const EnumValueDescriptorProto& value, int indent_level,
                       const std::vector<EnumValueDescriptorProto>* all_values,
                       std::string* output);

  // Format a oneof.
  void FormatOneof(const OneofDescriptorProto& oneof,
                   const DescriptorProto& message, int oneof_index,
                   int indent_level, std::string* output);

  // Format a service.
  void FormatService(const ServiceDescriptorProto& service, int indent_level,
                     std::string* output);

  // Format an RPC method.
  void FormatMethod(const MethodDescriptorProto& method, int indent_level,
                    std::string* output);

  // Format field options.
  void FormatFieldOptions(const FieldDescriptorProto& field,
                          std::string* output);

  // Format message options.
  void FormatMessageOptions(const MessageOptions& options, int indent_level,
                            std::string* output);

  // Format extend blocks.
  void FormatExtensions(const FileDescriptorProto& file_proto,
                        std::string* output);

  // Format a map entry.
  void FormatMapEntry(const FieldDescriptorProto& field, int indent_level,
                      std::string* output);

  // Helper to generate indentation string.
  std::string Indent(int level) const;

  // Get the type name for a field.
  std::string GetTypeName(const FieldDescriptorProto& field) const;

  // Get the label string (optional, required, repeated).
  std::string GetLabelString(const FieldDescriptorProto& field) const;

  // Calculate field alignment widths for a message.
  struct AlignmentInfo {
    size_t max_type_width;
    size_t max_name_width;
  };
  AlignmentInfo CalculateAlignment(
      const std::vector<FieldDescriptorProto>& fields) const;

  // Parse the input string into a FileDescriptorProto.
  absl::StatusOr<FileDescriptorProto> Parse(const std::string& input);

  // Configuration for the formatter.
  ProtofmtConfig config_;
};

}  // namespace protofmt
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_COMPILER_PROTOFMT_PROTOFMT_H__
