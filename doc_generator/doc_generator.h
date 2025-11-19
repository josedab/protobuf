// Protocol Buffers - Google's data interchange format
// Copyright 2024 Google LLC.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Documentation generator plugin for Protocol Buffers.

#ifndef GOOGLE_PROTOBUF_DOC_GENERATOR_DOC_GENERATOR_H_
#define GOOGLE_PROTOBUF_DOC_GENERATOR_DOC_GENERATOR_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "google/protobuf/compiler/code_generator.h"
#include "google/protobuf/descriptor.h"
#include "doc_generator/doc_comment_parser.h"

namespace google {
namespace protobuf {
namespace doc_generator {

// Output format for generated documentation
enum class OutputFormat {
  HTML,      // Single HTML file
  MARKDOWN,  // Markdown file
  JSON,      // JSON for custom renderers
  SITE,      // Static documentation site
};

// Configuration for documentation generation
struct DocGeneratorOptions {
  OutputFormat format = OutputFormat::HTML;
  std::string output_file = "documentation";
  std::string title = "API Documentation";
  std::string version = "";
  bool include_source_info = true;
  bool generate_toc = true;
  bool dark_mode = false;
  std::string base_url = "";
  std::vector<std::string> custom_css;
};

// Represents documentation for a single element
struct DocElement {
  std::string name;
  std::string full_name;
  std::string type;  // message, enum, field, service, method
  ParsedDocComment doc;
  std::string source_file;
  int source_line = 0;

  // Nested elements
  std::vector<std::shared_ptr<DocElement>> children;
};

// Represents documentation for a complete file
struct DocFile {
  std::string name;
  std::string package;
  ParsedDocComment doc;
  std::vector<std::string> dependencies;
  std::vector<std::shared_ptr<DocElement>> messages;
  std::vector<std::shared_ptr<DocElement>> enums;
  std::vector<std::shared_ptr<DocElement>> services;
};

// Main documentation generator implementing CodeGenerator interface
class DocGenerator : public compiler::CodeGenerator {
 public:
  DocGenerator() = default;
  ~DocGenerator() override = default;

  // CodeGenerator interface
  bool Generate(const FileDescriptor* file, const std::string& parameter,
                compiler::GeneratorContext* generator_context,
                std::string* error) const override;

  uint64_t GetSupportedFeatures() const override {
    return FEATURE_PROTO3_OPTIONAL;
  }

 private:
  // Parse generator options from parameter string
  DocGeneratorOptions ParseOptions(const std::string& parameter) const;

  // Build documentation structure from file descriptor
  std::shared_ptr<DocFile> BuildDocFile(const FileDescriptor* file) const;

  // Build element documentation
  std::shared_ptr<DocElement> BuildMessageDoc(const Descriptor* message) const;
  std::shared_ptr<DocElement> BuildFieldDoc(
      const FieldDescriptor* field) const;
  std::shared_ptr<DocElement> BuildEnumDoc(const EnumDescriptor* enum_desc) const;
  std::shared_ptr<DocElement> BuildServiceDoc(
      const ServiceDescriptor* service) const;
  std::shared_ptr<DocElement> BuildMethodDoc(
      const MethodDescriptor* method) const;

  // Generate output in different formats
  std::string GenerateHtml(const std::vector<std::shared_ptr<DocFile>>& files,
                           const DocGeneratorOptions& options) const;
  std::string GenerateMarkdown(
      const std::vector<std::shared_ptr<DocFile>>& files,
      const DocGeneratorOptions& options) const;
  std::string GenerateJson(const std::vector<std::shared_ptr<DocFile>>& files,
                           const DocGeneratorOptions& options) const;
  void GenerateSite(const std::vector<std::shared_ptr<DocFile>>& files,
                    const DocGeneratorOptions& options,
                    compiler::GeneratorContext* context) const;

  // HTML generation helpers
  std::string GenerateHtmlHeader(const DocGeneratorOptions& options) const;
  std::string GenerateHtmlFooter() const;
  std::string GenerateHtmlToc(
      const std::vector<std::shared_ptr<DocFile>>& files) const;
  std::string GenerateHtmlMessage(const DocElement& element) const;
  std::string GenerateHtmlEnum(const DocElement& element) const;
  std::string GenerateHtmlService(const DocElement& element) const;
  std::string GenerateHtmlField(const DocElement& element) const;

  // Markdown generation helpers
  std::string GenerateMarkdownMessage(const DocElement& element,
                                      int heading_level) const;
  std::string GenerateMarkdownEnum(const DocElement& element,
                                   int heading_level) const;
  std::string GenerateMarkdownService(const DocElement& element,
                                      int heading_level) const;
  std::string GenerateMarkdownField(const DocElement& element) const;

  // JSON generation helpers
  std::string ElementToJson(const DocElement& element, int indent) const;
  std::string FileToJson(const DocFile& file, int indent) const;

  // Resolve cross-references to URLs
  void ResolveCrossReferences(
      const std::vector<std::shared_ptr<DocFile>>& files) const;

  // Utility functions
  std::string GetTypeUrl(const std::string& type_name) const;
  std::string GetFieldTypeString(const FieldDescriptor* field) const;

  mutable DocCommentParser parser_;
  mutable std::map<std::string, std::string> type_to_url_;
};

}  // namespace doc_generator
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_DOC_GENERATOR_DOC_GENERATOR_H_
