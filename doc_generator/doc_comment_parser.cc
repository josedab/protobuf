// Protocol Buffers - Google's data interchange format
// Copyright 2024 Google LLC.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "doc_generator/doc_comment_parser.h"

#include <algorithm>
#include <cmath>
#include <regex>
#include <sstream>

namespace google {
namespace protobuf {
namespace doc_generator {

namespace {

// Helper to get source location comment
std::string GetComment(const SourceLocation& location) {
  if (!location.leading_comments.empty()) {
    return location.leading_comments;
  }
  return location.trailing_comments;
}

// Helper to trim whitespace
std::string Trim(const std::string& str) {
  size_t start = str.find_first_not_of(" \t\n\r");
  if (start == std::string::npos) return "";
  size_t end = str.find_last_not_of(" \t\n\r");
  return str.substr(start, end - start + 1);
}

// Split string by delimiter
std::vector<std::string> Split(const std::string& str, char delimiter) {
  std::vector<std::string> result;
  std::istringstream stream(str);
  std::string token;
  while (std::getline(stream, token, delimiter)) {
    result.push_back(token);
  }
  return result;
}

}  // namespace

ParsedDocComment DocCommentParser::Parse(const std::string& comment) {
  ParsedDocComment result;
  result.raw_comment = comment;
  result.is_doc_comment = IsDocComment(comment);

  std::string cleaned = CleanComment(comment);
  ParseAnnotations(cleaned, &result);
  ParseCrossReferences(result.description, &result);

  return result;
}

template <typename DescriptorType>
ParsedDocComment ParseFromDescriptorImpl(DocCommentParser* parser,
                                         const DescriptorType* descriptor) {
  SourceLocation location;
  if (descriptor->GetSourceLocation(&location)) {
    return parser->Parse(GetComment(location));
  }
  return ParsedDocComment();
}

ParsedDocComment DocCommentParser::ParseFromDescriptor(
    const Descriptor* descriptor) {
  return ParseFromDescriptorImpl(this, descriptor);
}

ParsedDocComment DocCommentParser::ParseFromDescriptor(
    const FieldDescriptor* descriptor) {
  return ParseFromDescriptorImpl(this, descriptor);
}

ParsedDocComment DocCommentParser::ParseFromDescriptor(
    const EnumDescriptor* descriptor) {
  return ParseFromDescriptorImpl(this, descriptor);
}

ParsedDocComment DocCommentParser::ParseFromDescriptor(
    const EnumValueDescriptor* descriptor) {
  return ParseFromDescriptorImpl(this, descriptor);
}

ParsedDocComment DocCommentParser::ParseFromDescriptor(
    const ServiceDescriptor* descriptor) {
  return ParseFromDescriptorImpl(this, descriptor);
}

ParsedDocComment DocCommentParser::ParseFromDescriptor(
    const MethodDescriptor* descriptor) {
  return ParseFromDescriptorImpl(this, descriptor);
}

ParsedDocComment DocCommentParser::ParseFromDescriptor(
    const FileDescriptor* descriptor) {
  SourceLocation location;
  // File descriptors require special handling
  if (descriptor->GetSourceLocation(&location)) {
    return Parse(GetComment(location));
  }
  return ParsedDocComment();
}

std::string DocCommentParser::MarkdownToHtml(const std::string& markdown) {
  std::string result = markdown;

  // Headers
  result = std::regex_replace(result, std::regex("^### (.+)$"), "<h3>$1</h3>");
  result = std::regex_replace(result, std::regex("^## (.+)$"), "<h2>$1</h2>");
  result = std::regex_replace(result, std::regex("^# (.+)$"), "<h1>$1</h1>");

  // Bold and italic
  result = std::regex_replace(result, std::regex("\\*\\*(.+?)\\*\\*"),
                               "<strong>$1</strong>");
  result = std::regex_replace(result, std::regex("\\*(.+?)\\*"), "<em>$1</em>");

  // Inline code
  result = std::regex_replace(result, std::regex("`([^`]+)`"),
                               "<code>$1</code>");

  // Code blocks with language
  result = std::regex_replace(
      result, std::regex("```(\\w+)?\\n([\\s\\S]*?)```"),
      "<pre><code class=\"language-$1\">$2</code></pre>");

  // Links
  result = std::regex_replace(result, std::regex("\\[([^\\]]+)\\]\\(([^)]+)\\)"),
                               "<a href=\"$2\">$1</a>");

  // Blockquotes
  result = std::regex_replace(result, std::regex("^> (.+)$"),
                               "<blockquote>$1</blockquote>");

  // Unordered lists
  result = std::regex_replace(result, std::regex("^- (.+)$"), "<li>$1</li>");

  // Ordered lists
  result = std::regex_replace(result, std::regex("^\\d+\\. (.+)$"),
                               "<li>$1</li>");

  // Paragraphs (double newlines)
  result = std::regex_replace(result, std::regex("\n\n"), "</p><p>");

  // Line breaks
  result = std::regex_replace(result, std::regex("\n"), "<br/>");

  return "<p>" + result + "</p>";
}

std::string DocCommentParser::EscapeHtml(const std::string& text) {
  std::string result;
  result.reserve(text.length());
  for (char c : text) {
    switch (c) {
      case '&':
        result += "&amp;";
        break;
      case '<':
        result += "&lt;";
        break;
      case '>':
        result += "&gt;";
        break;
      case '"':
        result += "&quot;";
        break;
      case '\'':
        result += "&#39;";
        break;
      default:
        result += c;
    }
  }
  return result;
}

std::string DocCommentParser::Dedent(const std::string& text) {
  std::vector<std::string> lines = Split(text, '\n');
  if (lines.empty()) return text;

  // Find minimum indentation
  size_t min_indent = std::string::npos;
  for (const auto& line : lines) {
    if (Trim(line).empty()) continue;
    size_t indent = line.find_first_not_of(" \t");
    if (indent != std::string::npos) {
      min_indent = std::min(min_indent, indent);
    }
  }

  if (min_indent == std::string::npos || min_indent == 0) {
    return text;
  }

  // Remove common indentation
  std::string result;
  for (const auto& line : lines) {
    if (line.length() > min_indent) {
      result += line.substr(min_indent) + "\n";
    } else {
      result += "\n";
    }
  }

  return result;
}

void DocCommentParser::ParseAnnotations(const std::string& comment,
                                        ParsedDocComment* result) {
  std::vector<std::string> lines = Split(comment, '\n');
  std::string description;
  bool in_example = false;
  std::string example_content;
  std::string example_lang;

  for (const auto& line : lines) {
    std::string trimmed = Trim(line);

    if (in_example) {
      if (trimmed == "```" || trimmed.substr(0, 3) == "```") {
        if (!example_content.empty()) {
          ParsedDocComment::Example ex;
          ex.language = example_lang;
          ex.code = Trim(example_content);
          result->examples.push_back(ex);
        }
        in_example = false;
        example_content.clear();
        example_lang.clear();
      } else {
        example_content += line + "\n";
      }
      continue;
    }

    // Check for code block start
    if (trimmed.substr(0, 3) == "```") {
      in_example = true;
      if (trimmed.length() > 3) {
        example_lang = trimmed.substr(3);
      }
      continue;
    }

    // Check for annotation
    if (trimmed[0] == '@') {
      ExtractAnnotation(trimmed, result);
      continue;
    }

    // Regular description text
    description += line + "\n";
  }

  result->description = Trim(description);
}

void DocCommentParser::ExtractAnnotation(const std::string& line,
                                         ParsedDocComment* result) {
  // Parse @tag value format
  std::regex annotation_regex("@(\\w+)\\s*(.*)");
  std::smatch match;

  if (std::regex_match(line, match, annotation_regex)) {
    std::string tag = match[1].str();
    std::string value = Trim(match[2].str());

    if (tag == "since") {
      result->since_version = value;
    } else if (tag == "deprecated") {
      // Parse "v1.0.0 Use X instead" format
      size_t space_pos = value.find(' ');
      if (space_pos != std::string::npos) {
        result->deprecated_version = value.substr(0, space_pos);
        result->deprecated_reason = value.substr(space_pos + 1);
      } else {
        result->deprecated_version = value;
      }
    } else if (tag == "author") {
      result->author = value;
    } else if (tag == "see") {
      result->see_also.push_back(value);
    } else if (tag == "throws") {
      result->throws.push_back(value);
    } else if (tag == "format") {
      result->format = value;
    } else if (tag == "example") {
      result->example = value;
    } else if (tag == "default") {
      result->default_value = value;
    } else if (tag == "required") {
      result->required = true;
    } else if (tag == "minLength") {
      try {
        result->min_length = std::stoi(value);
      } catch (...) {
      }
    } else if (tag == "maxLength") {
      try {
        result->max_length = std::stoi(value);
      } catch (...) {
      }
    } else if (tag == "min") {
      try {
        result->min_value = std::stod(value);
      } catch (...) {
      }
    } else if (tag == "max") {
      try {
        result->max_value = std::stod(value);
      } catch (...) {
      }
    } else if (tag == "pattern") {
      result->pattern = value;
    }
  }
}

void DocCommentParser::ParseCrossReferences(const std::string& text,
                                            ParsedDocComment* result) {
  // Find all {@link ...} patterns
  std::regex link_regex("\\{@link\\s+([^}]+)\\}");
  auto begin = std::sregex_iterator(text.begin(), text.end(), link_regex);
  auto end = std::sregex_iterator();

  for (auto it = begin; it != end; ++it) {
    std::smatch match = *it;
    std::string target = Trim(match[1].str());

    CrossReference ref;
    ref.target = target;
    ref.display_text = target;
    ref.type = DetermineReferenceType(target);

    result->cross_references.push_back(ref);
  }
}

CrossReference::Type DocCommentParser::DetermineReferenceType(
    const std::string& target) {
  // Local field: #field_name
  if (target[0] == '#') {
    return CrossReference::LOCAL_FIELD;
  }

  // Field reference: MessageName#field_name
  if (target.find('#') != std::string::npos) {
    return CrossReference::FIELD;
  }

  // Method reference: ServiceName.MethodName
  if (target.find('.') != std::string::npos) {
    // Could be method or enum value - we'll need context to determine
    // For now, assume method if it looks like UpperCase.UpperCase
    return CrossReference::METHOD;
  }

  // Simple type reference - could be message, service, or enum
  // Default to message
  return CrossReference::MESSAGE;
}

std::string DocCommentParser::CleanComment(const std::string& comment) {
  std::vector<std::string> lines = Split(comment, '\n');
  std::string result;

  for (const auto& line : lines) {
    std::string cleaned = line;

    // Remove leading whitespace
    size_t start = cleaned.find_first_not_of(" \t");
    if (start != std::string::npos) {
      cleaned = cleaned.substr(start);
    } else {
      cleaned = "";
    }

    // Remove doc-comment markers
    if (cleaned.substr(0, 3) == "/**") {
      cleaned = cleaned.substr(3);
    } else if (cleaned.substr(0, 3) == "///") {
      cleaned = cleaned.substr(3);
    } else if (cleaned.substr(0, 2) == "//") {
      cleaned = cleaned.substr(2);
    } else if (cleaned.substr(0, 2) == "*/") {
      continue;
    } else if (cleaned.substr(0, 1) == "*") {
      cleaned = cleaned.substr(1);
    }

    // Remove leading space after marker
    if (!cleaned.empty() && cleaned[0] == ' ') {
      cleaned = cleaned.substr(1);
    }

    result += cleaned + "\n";
  }

  return Trim(result);
}

bool DocCommentParser::IsDocComment(const std::string& comment) {
  std::string trimmed = Trim(comment);
  return trimmed.substr(0, 3) == "/**" || trimmed.substr(0, 3) == "///";
}

}  // namespace doc_generator
}  // namespace protobuf
}  // namespace google
