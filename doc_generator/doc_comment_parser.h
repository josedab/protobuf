// Protocol Buffers - Google's data interchange format
// Copyright 2024 Google LLC.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Parser for enhanced documentation comments with Markdown and annotations.

#ifndef GOOGLE_PROTOBUF_DOC_GENERATOR_DOC_COMMENT_PARSER_H_
#define GOOGLE_PROTOBUF_DOC_GENERATOR_DOC_COMMENT_PARSER_H_

#include <map>
#include <string>
#include <vector>

#include "google/protobuf/descriptor.h"

namespace google {
namespace protobuf {
namespace doc_generator {

// Represents a cross-reference link in documentation
struct CrossReference {
  enum Type {
    MESSAGE,      // {@link MessageName}
    SERVICE,      // {@link ServiceName}
    METHOD,       // {@link ServiceName.MethodName}
    FIELD,        // {@link MessageName#field_name}
    ENUM,         // {@link EnumName}
    ENUM_VALUE,   // {@link EnumName.VALUE}
    LOCAL_FIELD,  // {@link #field_name}
  };

  Type type;
  std::string target;       // Full target path
  std::string display_text; // Text to display
  std::string resolved_url; // Resolved URL for linking
};

// Represents a parsed documentation comment
struct ParsedDocComment {
  // Main description (may contain Markdown)
  std::string description;

  // Structured annotations
  std::string since_version;
  std::string deprecated_version;
  std::string deprecated_reason;
  std::string author;
  std::vector<std::string> see_also;
  std::vector<std::string> throws;

  // Field-specific annotations
  std::string format;
  std::string example;
  std::string default_value;
  bool required = false;
  int32_t min_length = -1;
  int32_t max_length = -1;
  double min_value = std::numeric_limits<double>::quiet_NaN();
  double max_value = std::numeric_limits<double>::quiet_NaN();
  std::string pattern;

  // Code examples (from @example blocks)
  struct Example {
    std::string language;
    std::string code;
    std::string description;
  };
  std::vector<Example> examples;

  // Cross-references found in the text
  std::vector<CrossReference> cross_references;

  // Original raw comment
  std::string raw_comment;

  // Is the comment using doc-comment style (/** or ///)
  bool is_doc_comment = false;
};

// Parser for enhanced documentation comments
class DocCommentParser {
 public:
  DocCommentParser() = default;

  // Parse a comment string into structured documentation
  ParsedDocComment Parse(const std::string& comment);

  // Parse comments from a descriptor's source location
  ParsedDocComment ParseFromDescriptor(const Descriptor* descriptor);
  ParsedDocComment ParseFromDescriptor(const FieldDescriptor* descriptor);
  ParsedDocComment ParseFromDescriptor(const EnumDescriptor* descriptor);
  ParsedDocComment ParseFromDescriptor(const EnumValueDescriptor* descriptor);
  ParsedDocComment ParseFromDescriptor(const ServiceDescriptor* descriptor);
  ParsedDocComment ParseFromDescriptor(const MethodDescriptor* descriptor);
  ParsedDocComment ParseFromDescriptor(const FileDescriptor* descriptor);

  // Convert Markdown to HTML
  static std::string MarkdownToHtml(const std::string& markdown);

  // Escape HTML entities
  static std::string EscapeHtml(const std::string& text);

  // Strip leading whitespace from multiline strings (dedent)
  static std::string Dedent(const std::string& text);

 private:
  // Parse annotations from comment text
  void ParseAnnotations(const std::string& comment, ParsedDocComment* result);

  // Extract @tag annotations
  void ExtractAnnotation(const std::string& line, ParsedDocComment* result);

  // Parse @example blocks
  void ParseExampleBlock(const std::string& content, ParsedDocComment* result);

  // Find and parse {@link ...} references
  void ParseCrossReferences(const std::string& text, ParsedDocComment* result);

  // Determine cross-reference type from target string
  CrossReference::Type DetermineReferenceType(const std::string& target);

  // Clean up comment formatting (strip leading *, trim whitespace)
  std::string CleanComment(const std::string& comment);

  // Check if comment is doc-comment style
  bool IsDocComment(const std::string& comment);
};

}  // namespace doc_generator
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_DOC_GENERATOR_DOC_COMMENT_PARSER_H_
