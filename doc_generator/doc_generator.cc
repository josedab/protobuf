// Protocol Buffers - Google's data interchange format
// Copyright 2024 Google LLC.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "doc_generator/doc_generator.h"

#include <sstream>
#include <algorithm>

#include "google/protobuf/io/printer.h"
#include "google/protobuf/io/zero_copy_stream.h"

namespace google {
namespace protobuf {
namespace doc_generator {

namespace {

std::string Indent(int level) {
  return std::string(level * 2, ' ');
}

std::string JsonEscape(const std::string& str) {
  std::string result;
  for (char c : str) {
    switch (c) {
      case '"': result += "\\\""; break;
      case '\\': result += "\\\\"; break;
      case '\n': result += "\\n"; break;
      case '\r': result += "\\r"; break;
      case '\t': result += "\\t"; break;
      default: result += c;
    }
  }
  return result;
}

std::vector<std::string> SplitOptions(const std::string& parameter) {
  std::vector<std::string> result;
  std::istringstream stream(parameter);
  std::string token;
  while (std::getline(stream, token, ',')) {
    result.push_back(token);
  }
  return result;
}

}  // namespace

bool DocGenerator::Generate(const FileDescriptor* file,
                            const std::string& parameter,
                            compiler::GeneratorContext* generator_context,
                            std::string* error) const {
  DocGeneratorOptions options = ParseOptions(parameter);

  // Build documentation for this file
  auto doc_file = BuildDocFile(file);
  std::vector<std::shared_ptr<DocFile>> files = {doc_file};

  // Resolve cross-references
  ResolveCrossReferences(files);

  // Generate output based on format
  std::string output;
  std::string extension;

  switch (options.format) {
    case OutputFormat::HTML:
      output = GenerateHtml(files, options);
      extension = ".html";
      break;
    case OutputFormat::MARKDOWN:
      output = GenerateMarkdown(files, options);
      extension = ".md";
      break;
    case OutputFormat::JSON:
      output = GenerateJson(files, options);
      extension = ".json";
      break;
    case OutputFormat::SITE:
      GenerateSite(files, options, generator_context);
      return true;
  }

  // Write output file
  std::string output_file = options.output_file + extension;
  std::unique_ptr<io::ZeroCopyOutputStream> stream(
      generator_context->Open(output_file));
  io::Printer printer(stream.get(), '$');
  printer.PrintRaw(output);

  return true;
}

DocGeneratorOptions DocGenerator::ParseOptions(
    const std::string& parameter) const {
  DocGeneratorOptions options;

  auto parts = SplitOptions(parameter);
  for (const auto& part : parts) {
    size_t eq_pos = part.find('=');
    std::string key, value;
    if (eq_pos != std::string::npos) {
      key = part.substr(0, eq_pos);
      value = part.substr(eq_pos + 1);
    } else {
      key = part;
    }

    if (key == "html") {
      options.format = OutputFormat::HTML;
      if (!value.empty()) options.output_file = value;
    } else if (key == "markdown" || key == "md") {
      options.format = OutputFormat::MARKDOWN;
      if (!value.empty()) options.output_file = value;
    } else if (key == "json") {
      options.format = OutputFormat::JSON;
      if (!value.empty()) options.output_file = value;
    } else if (key == "site") {
      options.format = OutputFormat::SITE;
    } else if (key == "title") {
      options.title = value;
    } else if (key == "version") {
      options.version = value;
    } else if (key == "dark") {
      options.dark_mode = true;
    }
  }

  return options;
}

std::shared_ptr<DocFile> DocGenerator::BuildDocFile(
    const FileDescriptor* file) const {
  auto doc_file = std::make_shared<DocFile>();
  doc_file->name = file->name();
  doc_file->package = file->package();
  doc_file->doc = parser_.ParseFromDescriptor(file);

  // Dependencies
  for (int i = 0; i < file->dependency_count(); ++i) {
    doc_file->dependencies.push_back(file->dependency(i)->name());
  }

  // Messages
  for (int i = 0; i < file->message_type_count(); ++i) {
    doc_file->messages.push_back(BuildMessageDoc(file->message_type(i)));
  }

  // Enums
  for (int i = 0; i < file->enum_type_count(); ++i) {
    doc_file->enums.push_back(BuildEnumDoc(file->enum_type(i)));
  }

  // Services
  for (int i = 0; i < file->service_count(); ++i) {
    doc_file->services.push_back(BuildServiceDoc(file->service(i)));
  }

  return doc_file;
}

std::shared_ptr<DocElement> DocGenerator::BuildMessageDoc(
    const Descriptor* message) const {
  auto element = std::make_shared<DocElement>();
  element->name = message->name();
  element->full_name = message->full_name();
  element->type = "message";
  element->doc = parser_.ParseFromDescriptor(message);

  SourceLocation location;
  if (message->GetSourceLocation(&location)) {
    element->source_file = message->file()->name();
    element->source_line = location.start_line + 1;
  }

  // Fields
  for (int i = 0; i < message->field_count(); ++i) {
    element->children.push_back(BuildFieldDoc(message->field(i)));
  }

  // Nested types
  for (int i = 0; i < message->nested_type_count(); ++i) {
    element->children.push_back(BuildMessageDoc(message->nested_type(i)));
  }

  // Nested enums
  for (int i = 0; i < message->enum_type_count(); ++i) {
    element->children.push_back(BuildEnumDoc(message->enum_type(i)));
  }

  // Register for cross-referencing
  type_to_url_[element->full_name] = "#" + element->full_name;

  return element;
}

std::shared_ptr<DocElement> DocGenerator::BuildFieldDoc(
    const FieldDescriptor* field) const {
  auto element = std::make_shared<DocElement>();
  element->name = field->name();
  element->full_name = field->full_name();
  element->type = "field";
  element->doc = parser_.ParseFromDescriptor(field);

  SourceLocation location;
  if (field->GetSourceLocation(&location)) {
    element->source_file = field->file()->name();
    element->source_line = location.start_line + 1;
  }

  return element;
}

std::shared_ptr<DocElement> DocGenerator::BuildEnumDoc(
    const EnumDescriptor* enum_desc) const {
  auto element = std::make_shared<DocElement>();
  element->name = enum_desc->name();
  element->full_name = enum_desc->full_name();
  element->type = "enum";
  element->doc = parser_.ParseFromDescriptor(enum_desc);

  SourceLocation location;
  if (enum_desc->GetSourceLocation(&location)) {
    element->source_file = enum_desc->file()->name();
    element->source_line = location.start_line + 1;
  }

  // Enum values
  for (int i = 0; i < enum_desc->value_count(); ++i) {
    auto value_element = std::make_shared<DocElement>();
    auto value = enum_desc->value(i);
    value_element->name = value->name();
    value_element->full_name = value->full_name();
    value_element->type = "enum_value";
    value_element->doc = parser_.ParseFromDescriptor(value);
    element->children.push_back(value_element);
  }

  // Register for cross-referencing
  type_to_url_[element->full_name] = "#" + element->full_name;

  return element;
}

std::shared_ptr<DocElement> DocGenerator::BuildServiceDoc(
    const ServiceDescriptor* service) const {
  auto element = std::make_shared<DocElement>();
  element->name = service->name();
  element->full_name = service->full_name();
  element->type = "service";
  element->doc = parser_.ParseFromDescriptor(service);

  SourceLocation location;
  if (service->GetSourceLocation(&location)) {
    element->source_file = service->file()->name();
    element->source_line = location.start_line + 1;
  }

  // Methods
  for (int i = 0; i < service->method_count(); ++i) {
    element->children.push_back(BuildMethodDoc(service->method(i)));
  }

  // Register for cross-referencing
  type_to_url_[element->full_name] = "#" + element->full_name;

  return element;
}

std::shared_ptr<DocElement> DocGenerator::BuildMethodDoc(
    const MethodDescriptor* method) const {
  auto element = std::make_shared<DocElement>();
  element->name = method->name();
  element->full_name = method->full_name();
  element->type = "method";
  element->doc = parser_.ParseFromDescriptor(method);

  SourceLocation location;
  if (method->GetSourceLocation(&location)) {
    element->source_file = method->file()->name();
    element->source_line = location.start_line + 1;
  }

  return element;
}

// HTML Generation
std::string DocGenerator::GenerateHtml(
    const std::vector<std::shared_ptr<DocFile>>& files,
    const DocGeneratorOptions& options) const {
  std::ostringstream out;

  out << GenerateHtmlHeader(options);

  if (options.generate_toc) {
    out << GenerateHtmlToc(files);
  }

  out << "<main class=\"content\">\n";

  for (const auto& file : files) {
    out << "<section class=\"file\" id=\"" << file->name << "\">\n";
    out << "<h2>File: " << file->name << "</h2>\n";

    if (!file->package.empty()) {
      out << "<p class=\"package\">Package: <code>" << file->package
          << "</code></p>\n";
    }

    if (!file->doc.description.empty()) {
      out << "<div class=\"description\">"
          << DocCommentParser::MarkdownToHtml(file->doc.description)
          << "</div>\n";
    }

    // Messages
    if (!file->messages.empty()) {
      out << "<h3>Messages</h3>\n";
      for (const auto& msg : file->messages) {
        out << GenerateHtmlMessage(*msg);
      }
    }

    // Enums
    if (!file->enums.empty()) {
      out << "<h3>Enums</h3>\n";
      for (const auto& enum_elem : file->enums) {
        out << GenerateHtmlEnum(*enum_elem);
      }
    }

    // Services
    if (!file->services.empty()) {
      out << "<h3>Services</h3>\n";
      for (const auto& svc : file->services) {
        out << GenerateHtmlService(*svc);
      }
    }

    out << "</section>\n";
  }

  out << "</main>\n";
  out << GenerateHtmlFooter();

  return out.str();
}

std::string DocGenerator::GenerateHtmlHeader(
    const DocGeneratorOptions& options) const {
  std::ostringstream out;

  out << "<!DOCTYPE html>\n";
  out << "<html lang=\"en\">\n";
  out << "<head>\n";
  out << "  <meta charset=\"UTF-8\">\n";
  out << "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n";
  out << "  <title>" << DocCommentParser::EscapeHtml(options.title)
      << "</title>\n";
  out << "  <style>\n";
  out << R"(
    :root {
      --bg-color: #ffffff;
      --text-color: #333333;
      --code-bg: #f5f5f5;
      --border-color: #e0e0e0;
      --link-color: #0066cc;
      --heading-color: #222222;
      --deprecated-color: #cc0000;
    }
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
      margin: 0;
      padding: 20px;
      background: var(--bg-color);
      color: var(--text-color);
      line-height: 1.6;
    }
    .container { max-width: 1200px; margin: 0 auto; }
    header { border-bottom: 1px solid var(--border-color); padding-bottom: 20px; margin-bottom: 20px; }
    h1 { color: var(--heading-color); margin: 0; }
    h2 { color: var(--heading-color); border-bottom: 1px solid var(--border-color); padding-bottom: 10px; }
    h3 { color: var(--heading-color); }
    a { color: var(--link-color); text-decoration: none; }
    a:hover { text-decoration: underline; }
    code { background: var(--code-bg); padding: 2px 6px; border-radius: 3px; font-family: 'Fira Code', monospace; }
    pre { background: var(--code-bg); padding: 15px; border-radius: 5px; overflow-x: auto; }
    pre code { padding: 0; background: none; }
    .message, .enum, .service {
      border: 1px solid var(--border-color);
      border-radius: 5px;
      margin: 15px 0;
      padding: 15px;
    }
    .field { padding: 10px; border-bottom: 1px solid var(--border-color); }
    .field:last-child { border-bottom: none; }
    .field-name { font-weight: bold; color: var(--heading-color); }
    .field-type { color: #666; }
    .deprecated { color: var(--deprecated-color); font-style: italic; }
    .badge {
      display: inline-block;
      padding: 2px 8px;
      border-radius: 3px;
      font-size: 0.8em;
      margin-left: 5px;
    }
    .badge-since { background: #e3f2fd; color: #1565c0; }
    .badge-deprecated { background: #ffebee; color: #c62828; }
    .badge-required { background: #fff3e0; color: #e65100; }
    .toc {
      position: fixed;
      left: 0;
      top: 0;
      bottom: 0;
      width: 250px;
      background: var(--code-bg);
      padding: 20px;
      overflow-y: auto;
    }
    .toc ul { list-style: none; padding-left: 15px; }
    .toc > ul { padding-left: 0; }
    .content { margin-left: 290px; }
    @media (max-width: 768px) {
      .toc { display: none; }
      .content { margin-left: 0; }
    }
  )";

  if (options.dark_mode) {
    out << R"(
    :root {
      --bg-color: #1a1a1a;
      --text-color: #e0e0e0;
      --code-bg: #2d2d2d;
      --border-color: #404040;
      --link-color: #6eb5ff;
      --heading-color: #ffffff;
    }
    )";
  }

  out << "  </style>\n";
  out << "</head>\n";
  out << "<body>\n";
  out << "<div class=\"container\">\n";
  out << "<header>\n";
  out << "  <h1>" << DocCommentParser::EscapeHtml(options.title) << "</h1>\n";
  if (!options.version.empty()) {
    out << "  <p>Version: " << DocCommentParser::EscapeHtml(options.version)
        << "</p>\n";
  }
  out << "</header>\n";

  return out.str();
}

std::string DocGenerator::GenerateHtmlFooter() const {
  return "</div>\n</body>\n</html>\n";
}

std::string DocGenerator::GenerateHtmlToc(
    const std::vector<std::shared_ptr<DocFile>>& files) const {
  std::ostringstream out;

  out << "<nav class=\"toc\">\n";
  out << "<h3>Table of Contents</h3>\n";
  out << "<ul>\n";

  for (const auto& file : files) {
    out << "<li><a href=\"#" << file->name << "\">" << file->name << "</a>\n";
    out << "<ul>\n";

    if (!file->messages.empty()) {
      out << "<li>Messages\n<ul>\n";
      for (const auto& msg : file->messages) {
        out << "<li><a href=\"#" << msg->full_name << "\">" << msg->name
            << "</a></li>\n";
      }
      out << "</ul></li>\n";
    }

    if (!file->enums.empty()) {
      out << "<li>Enums\n<ul>\n";
      for (const auto& e : file->enums) {
        out << "<li><a href=\"#" << e->full_name << "\">" << e->name
            << "</a></li>\n";
      }
      out << "</ul></li>\n";
    }

    if (!file->services.empty()) {
      out << "<li>Services\n<ul>\n";
      for (const auto& s : file->services) {
        out << "<li><a href=\"#" << s->full_name << "\">" << s->name
            << "</a></li>\n";
      }
      out << "</ul></li>\n";
    }

    out << "</ul></li>\n";
  }

  out << "</ul>\n</nav>\n";

  return out.str();
}

std::string DocGenerator::GenerateHtmlMessage(const DocElement& element) const {
  std::ostringstream out;

  out << "<div class=\"message\" id=\"" << element.full_name << "\">\n";
  out << "<h4>" << element.name;

  if (!element.doc.since_version.empty()) {
    out << "<span class=\"badge badge-since\">since "
        << element.doc.since_version << "</span>";
  }
  if (!element.doc.deprecated_version.empty()) {
    out << "<span class=\"badge badge-deprecated\">deprecated</span>";
  }

  out << "</h4>\n";

  if (!element.doc.description.empty()) {
    out << "<div class=\"description\">"
        << DocCommentParser::MarkdownToHtml(element.doc.description)
        << "</div>\n";
  }

  if (!element.doc.deprecated_version.empty()) {
    out << "<p class=\"deprecated\">Deprecated in "
        << element.doc.deprecated_version;
    if (!element.doc.deprecated_reason.empty()) {
      out << ": " << element.doc.deprecated_reason;
    }
    out << "</p>\n";
  }

  // Fields
  bool has_fields = false;
  for (const auto& child : element.children) {
    if (child->type == "field") {
      if (!has_fields) {
        out << "<h5>Fields</h5>\n";
        has_fields = true;
      }
      out << GenerateHtmlField(*child);
    }
  }

  // Examples
  if (!element.doc.examples.empty()) {
    out << "<h5>Examples</h5>\n";
    for (const auto& ex : element.doc.examples) {
      if (!ex.language.empty()) {
        out << "<pre><code class=\"language-" << ex.language << "\">";
      } else {
        out << "<pre><code>";
      }
      out << DocCommentParser::EscapeHtml(ex.code) << "</code></pre>\n";
    }
  }

  // Nested types
  for (const auto& child : element.children) {
    if (child->type == "message") {
      out << GenerateHtmlMessage(*child);
    } else if (child->type == "enum") {
      out << GenerateHtmlEnum(*child);
    }
  }

  out << "</div>\n";

  return out.str();
}

std::string DocGenerator::GenerateHtmlField(const DocElement& element) const {
  std::ostringstream out;

  out << "<div class=\"field\">\n";
  out << "<span class=\"field-name\">" << element.name << "</span>";

  if (element.doc.required) {
    out << "<span class=\"badge badge-required\">required</span>";
  }

  out << "\n";

  if (!element.doc.description.empty()) {
    out << "<p>" << DocCommentParser::EscapeHtml(element.doc.description)
        << "</p>\n";
  }

  // Field metadata
  std::vector<std::string> meta;
  if (!element.doc.format.empty()) {
    meta.push_back("Format: " + element.doc.format);
  }
  if (!element.doc.example.empty()) {
    meta.push_back("Example: <code>" + DocCommentParser::EscapeHtml(element.doc.example) + "</code>");
  }
  if (element.doc.min_length >= 0) {
    meta.push_back("Min length: " + std::to_string(element.doc.min_length));
  }
  if (element.doc.max_length >= 0) {
    meta.push_back("Max length: " + std::to_string(element.doc.max_length));
  }

  if (!meta.empty()) {
    out << "<ul class=\"field-meta\">\n";
    for (const auto& m : meta) {
      out << "<li>" << m << "</li>\n";
    }
    out << "</ul>\n";
  }

  out << "</div>\n";

  return out.str();
}

std::string DocGenerator::GenerateHtmlEnum(const DocElement& element) const {
  std::ostringstream out;

  out << "<div class=\"enum\" id=\"" << element.full_name << "\">\n";
  out << "<h4>" << element.name << "</h4>\n";

  if (!element.doc.description.empty()) {
    out << "<div class=\"description\">"
        << DocCommentParser::MarkdownToHtml(element.doc.description)
        << "</div>\n";
  }

  out << "<h5>Values</h5>\n";
  out << "<table>\n<tr><th>Name</th><th>Description</th></tr>\n";
  for (const auto& value : element.children) {
    out << "<tr><td><code>" << value->name << "</code></td>";
    out << "<td>" << value->doc.description << "</td></tr>\n";
  }
  out << "</table>\n";

  out << "</div>\n";

  return out.str();
}

std::string DocGenerator::GenerateHtmlService(const DocElement& element) const {
  std::ostringstream out;

  out << "<div class=\"service\" id=\"" << element.full_name << "\">\n";
  out << "<h4>" << element.name << "</h4>\n";

  if (!element.doc.description.empty()) {
    out << "<div class=\"description\">"
        << DocCommentParser::MarkdownToHtml(element.doc.description)
        << "</div>\n";
  }

  out << "<h5>Methods</h5>\n";
  for (const auto& method : element.children) {
    out << "<div class=\"method\" id=\"" << method->full_name << "\">\n";
    out << "<h6>" << method->name << "</h6>\n";
    if (!method->doc.description.empty()) {
      out << "<p>" << method->doc.description << "</p>\n";
    }

    // Error responses
    if (!method->doc.throws.empty()) {
      out << "<p><strong>Throws:</strong></p>\n<ul>\n";
      for (const auto& err : method->doc.throws) {
        out << "<li>" << err << "</li>\n";
      }
      out << "</ul>\n";
    }

    out << "</div>\n";
  }

  out << "</div>\n";

  return out.str();
}

// Markdown Generation
std::string DocGenerator::GenerateMarkdown(
    const std::vector<std::shared_ptr<DocFile>>& files,
    const DocGeneratorOptions& options) const {
  std::ostringstream out;

  out << "# " << options.title << "\n\n";

  if (!options.version.empty()) {
    out << "Version: " << options.version << "\n\n";
  }

  // Table of contents
  if (options.generate_toc) {
    out << "## Table of Contents\n\n";
    for (const auto& file : files) {
      out << "- [" << file->name << "](#" << file->name << ")\n";
      for (const auto& msg : file->messages) {
        out << "  - [" << msg->name << "](#" << msg->name << ")\n";
      }
      for (const auto& e : file->enums) {
        out << "  - [" << e->name << "](#" << e->name << ")\n";
      }
      for (const auto& s : file->services) {
        out << "  - [" << s->name << "](#" << s->name << ")\n";
      }
    }
    out << "\n";
  }

  for (const auto& file : files) {
    out << "## " << file->name << "\n\n";

    if (!file->package.empty()) {
      out << "**Package:** `" << file->package << "`\n\n";
    }

    if (!file->doc.description.empty()) {
      out << file->doc.description << "\n\n";
    }

    // Messages
    if (!file->messages.empty()) {
      out << "### Messages\n\n";
      for (const auto& msg : file->messages) {
        out << GenerateMarkdownMessage(*msg, 4);
      }
    }

    // Enums
    if (!file->enums.empty()) {
      out << "### Enums\n\n";
      for (const auto& e : file->enums) {
        out << GenerateMarkdownEnum(*e, 4);
      }
    }

    // Services
    if (!file->services.empty()) {
      out << "### Services\n\n";
      for (const auto& s : file->services) {
        out << GenerateMarkdownService(*s, 4);
      }
    }
  }

  return out.str();
}

std::string DocGenerator::GenerateMarkdownMessage(const DocElement& element,
                                                   int heading_level) const {
  std::ostringstream out;
  std::string heading(heading_level, '#');

  out << heading << " " << element.name << "\n\n";

  if (!element.doc.description.empty()) {
    out << element.doc.description << "\n\n";
  }

  if (!element.doc.since_version.empty()) {
    out << "**Since:** " << element.doc.since_version << "\n\n";
  }

  if (!element.doc.deprecated_version.empty()) {
    out << "> **Deprecated** in " << element.doc.deprecated_version;
    if (!element.doc.deprecated_reason.empty()) {
      out << ": " << element.doc.deprecated_reason;
    }
    out << "\n\n";
  }

  // Fields table
  bool has_fields = false;
  for (const auto& child : element.children) {
    if (child->type == "field") {
      if (!has_fields) {
        out << "**Fields:**\n\n";
        out << "| Field | Type | Description |\n";
        out << "|-------|------|-------------|\n";
        has_fields = true;
      }
      out << GenerateMarkdownField(*child);
    }
  }

  if (has_fields) {
    out << "\n";
  }

  // Examples
  if (!element.doc.examples.empty()) {
    out << "**Examples:**\n\n";
    for (const auto& ex : element.doc.examples) {
      out << "```" << ex.language << "\n" << ex.code << "\n```\n\n";
    }
  }

  // Nested types
  for (const auto& child : element.children) {
    if (child->type == "message") {
      out << GenerateMarkdownMessage(*child, heading_level + 1);
    } else if (child->type == "enum") {
      out << GenerateMarkdownEnum(*child, heading_level + 1);
    }
  }

  return out.str();
}

std::string DocGenerator::GenerateMarkdownField(
    const DocElement& element) const {
  std::ostringstream out;

  out << "| `" << element.name << "` | ";

  // Type info
  if (!element.doc.format.empty()) {
    out << element.doc.format;
  } else {
    out << "string";  // placeholder
  }

  out << " | " << element.doc.description;

  if (element.doc.required) {
    out << " *(required)*";
  }

  out << " |\n";

  return out.str();
}

std::string DocGenerator::GenerateMarkdownEnum(const DocElement& element,
                                                int heading_level) const {
  std::ostringstream out;
  std::string heading(heading_level, '#');

  out << heading << " " << element.name << "\n\n";

  if (!element.doc.description.empty()) {
    out << element.doc.description << "\n\n";
  }

  out << "**Values:**\n\n";
  out << "| Value | Description |\n";
  out << "|-------|-------------|\n";

  for (const auto& value : element.children) {
    out << "| `" << value->name << "` | " << value->doc.description << " |\n";
  }

  out << "\n";

  return out.str();
}

std::string DocGenerator::GenerateMarkdownService(const DocElement& element,
                                                   int heading_level) const {
  std::ostringstream out;
  std::string heading(heading_level, '#');

  out << heading << " " << element.name << "\n\n";

  if (!element.doc.description.empty()) {
    out << element.doc.description << "\n\n";
  }

  out << "**Methods:**\n\n";

  for (const auto& method : element.children) {
    out << heading << "# " << method->name << "\n\n";
    if (!method->doc.description.empty()) {
      out << method->doc.description << "\n\n";
    }
  }

  return out.str();
}

// JSON Generation
std::string DocGenerator::GenerateJson(
    const std::vector<std::shared_ptr<DocFile>>& files,
    const DocGeneratorOptions& options) const {
  std::ostringstream out;

  out << "{\n";
  out << "  \"title\": \"" << JsonEscape(options.title) << "\",\n";
  out << "  \"version\": \"" << JsonEscape(options.version) << "\",\n";
  out << "  \"files\": [\n";

  bool first_file = true;
  for (const auto& file : files) {
    if (!first_file) out << ",\n";
    first_file = false;
    out << FileToJson(*file, 2);
  }

  out << "\n  ]\n";
  out << "}\n";

  return out.str();
}

std::string DocGenerator::FileToJson(const DocFile& file, int indent) const {
  std::ostringstream out;
  std::string pad = Indent(indent);

  out << pad << "{\n";
  out << pad << "  \"name\": \"" << JsonEscape(file.name) << "\",\n";
  out << pad << "  \"package\": \"" << JsonEscape(file.package) << "\",\n";
  out << pad << "  \"description\": \"" << JsonEscape(file.doc.description) << "\",\n";

  // Messages
  out << pad << "  \"messages\": [\n";
  bool first = true;
  for (const auto& msg : file.messages) {
    if (!first) out << ",\n";
    first = false;
    out << ElementToJson(*msg, indent + 2);
  }
  out << "\n" << pad << "  ],\n";

  // Enums
  out << pad << "  \"enums\": [\n";
  first = true;
  for (const auto& e : file.enums) {
    if (!first) out << ",\n";
    first = false;
    out << ElementToJson(*e, indent + 2);
  }
  out << "\n" << pad << "  ],\n";

  // Services
  out << pad << "  \"services\": [\n";
  first = true;
  for (const auto& s : file.services) {
    if (!first) out << ",\n";
    first = false;
    out << ElementToJson(*s, indent + 2);
  }
  out << "\n" << pad << "  ]\n";

  out << pad << "}";

  return out.str();
}

std::string DocGenerator::ElementToJson(const DocElement& element,
                                        int indent) const {
  std::ostringstream out;
  std::string pad = Indent(indent);

  out << pad << "{\n";
  out << pad << "  \"name\": \"" << JsonEscape(element.name) << "\",\n";
  out << pad << "  \"fullName\": \"" << JsonEscape(element.full_name) << "\",\n";
  out << pad << "  \"type\": \"" << JsonEscape(element.type) << "\",\n";
  out << pad << "  \"description\": \"" << JsonEscape(element.doc.description) << "\"";

  if (!element.doc.since_version.empty()) {
    out << ",\n" << pad << "  \"since\": \"" << JsonEscape(element.doc.since_version) << "\"";
  }

  if (!element.doc.deprecated_version.empty()) {
    out << ",\n" << pad << "  \"deprecated\": \"" << JsonEscape(element.doc.deprecated_version) << "\"";
  }

  if (!element.doc.example.empty()) {
    out << ",\n" << pad << "  \"example\": \"" << JsonEscape(element.doc.example) << "\"";
  }

  if (!element.doc.format.empty()) {
    out << ",\n" << pad << "  \"format\": \"" << JsonEscape(element.doc.format) << "\"";
  }

  if (element.doc.required) {
    out << ",\n" << pad << "  \"required\": true";
  }

  // Children
  if (!element.children.empty()) {
    out << ",\n" << pad << "  \"children\": [\n";
    bool first = true;
    for (const auto& child : element.children) {
      if (!first) out << ",\n";
      first = false;
      out << ElementToJson(*child, indent + 2);
    }
    out << "\n" << pad << "  ]";
  }

  out << "\n" << pad << "}";

  return out.str();
}

// Static Site Generation
void DocGenerator::GenerateSite(
    const std::vector<std::shared_ptr<DocFile>>& files,
    const DocGeneratorOptions& options,
    compiler::GeneratorContext* context) const {

  // Generate index.html
  {
    std::unique_ptr<io::ZeroCopyOutputStream> stream(
        context->Open("index.html"));
    io::Printer printer(stream.get(), '$');
    printer.PrintRaw(GenerateHtml(files, options));
  }

  // Generate search.json for search functionality
  {
    std::unique_ptr<io::ZeroCopyOutputStream> stream(
        context->Open("search.json"));
    io::Printer printer(stream.get(), '$');

    std::ostringstream search_json;
    search_json << "[\n";

    bool first = true;
    for (const auto& file : files) {
      for (const auto& msg : file->messages) {
        if (!first) search_json << ",\n";
        first = false;
        search_json << "  {\"name\": \"" << JsonEscape(msg->name)
                    << "\", \"type\": \"message\", \"url\": \"index.html#"
                    << msg->full_name << "\"}";
      }
      for (const auto& e : file->enums) {
        if (!first) search_json << ",\n";
        first = false;
        search_json << "  {\"name\": \"" << JsonEscape(e->name)
                    << "\", \"type\": \"enum\", \"url\": \"index.html#"
                    << e->full_name << "\"}";
      }
      for (const auto& s : file->services) {
        if (!first) search_json << ",\n";
        first = false;
        search_json << "  {\"name\": \"" << JsonEscape(s->name)
                    << "\", \"type\": \"service\", \"url\": \"index.html#"
                    << s->full_name << "\"}";
      }
    }

    search_json << "\n]\n";
    printer.PrintRaw(search_json.str());
  }
}

void DocGenerator::ResolveCrossReferences(
    const std::vector<std::shared_ptr<DocFile>>& files) const {
  // Build type map from all files
  for (const auto& file : files) {
    for (const auto& msg : file->messages) {
      type_to_url_[msg->full_name] = "#" + msg->full_name;
    }
    for (const auto& e : file->enums) {
      type_to_url_[e->full_name] = "#" + e->full_name;
    }
    for (const auto& s : file->services) {
      type_to_url_[s->full_name] = "#" + s->full_name;
    }
  }
}

std::string DocGenerator::GetTypeUrl(const std::string& type_name) const {
  auto it = type_to_url_.find(type_name);
  if (it != type_to_url_.end()) {
    return it->second;
  }
  return "";
}

std::string DocGenerator::GetFieldTypeString(
    const FieldDescriptor* field) const {
  switch (field->type()) {
    case FieldDescriptor::TYPE_DOUBLE:
      return "double";
    case FieldDescriptor::TYPE_FLOAT:
      return "float";
    case FieldDescriptor::TYPE_INT64:
      return "int64";
    case FieldDescriptor::TYPE_UINT64:
      return "uint64";
    case FieldDescriptor::TYPE_INT32:
      return "int32";
    case FieldDescriptor::TYPE_FIXED64:
      return "fixed64";
    case FieldDescriptor::TYPE_FIXED32:
      return "fixed32";
    case FieldDescriptor::TYPE_BOOL:
      return "bool";
    case FieldDescriptor::TYPE_STRING:
      return "string";
    case FieldDescriptor::TYPE_BYTES:
      return "bytes";
    case FieldDescriptor::TYPE_UINT32:
      return "uint32";
    case FieldDescriptor::TYPE_SFIXED32:
      return "sfixed32";
    case FieldDescriptor::TYPE_SFIXED64:
      return "sfixed64";
    case FieldDescriptor::TYPE_SINT32:
      return "sint32";
    case FieldDescriptor::TYPE_SINT64:
      return "sint64";
    case FieldDescriptor::TYPE_MESSAGE:
      return field->message_type()->name();
    case FieldDescriptor::TYPE_ENUM:
      return field->enum_type()->name();
    default:
      return "unknown";
  }
}

}  // namespace doc_generator
}  // namespace protobuf
}  // namespace google
