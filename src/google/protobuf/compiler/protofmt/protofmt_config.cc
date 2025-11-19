// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/compiler/protofmt/protofmt_config.h"

#include <fstream>
#include <sstream>
#include <string>

#include "absl/status/status.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/strip.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace protofmt {

namespace {

// Simple YAML parser for configuration files.
// Only supports basic key: value pairs, no nested structures.
absl::StatusOr<ProtofmtConfig> ParseSimpleYaml(const std::string& content) {
  ProtofmtConfig config = ProtofmtConfig::Default();

  std::vector<std::string> lines = absl::StrSplit(content, '\n');
  for (const std::string& line : lines) {
    std::string trimmed_line(absl::StripAsciiWhitespace(line));

    // Skip empty lines and comments
    if (trimmed_line.empty() || trimmed_line[0] == '#') {
      continue;
    }

    // Find the colon separator
    size_t colon_pos = trimmed_line.find(':');
    if (colon_pos == std::string::npos) {
      return absl::InvalidArgumentError(
          absl::StrCat("Invalid YAML line: ", trimmed_line));
    }

    std::string key(absl::StripAsciiWhitespace(
        trimmed_line.substr(0, colon_pos)));
    std::string value(absl::StripAsciiWhitespace(
        trimmed_line.substr(colon_pos + 1)));

    if (key == "indent") {
      int indent_value;
      if (!absl::SimpleAtoi(value, &indent_value)) {
        return absl::InvalidArgumentError(
            absl::StrCat("Invalid indent value: ", value));
      }
      if (indent_value < 1 || indent_value > 8) {
        return absl::InvalidArgumentError(
            "Indent must be between 1 and 8");
      }
      config.indent = indent_value;
    } else if (key == "align_fields") {
      config.align_fields = (value == "true" || value == "yes");
    } else if (key == "sort_imports") {
      config.sort_imports = (value == "true" || value == "yes");
    } else if (key == "max_line_length") {
      int max_length;
      if (!absl::SimpleAtoi(value, &max_length)) {
        return absl::InvalidArgumentError(
            absl::StrCat("Invalid max_line_length value: ", value));
      }
      if (max_length < 40 || max_length > 500) {
        return absl::InvalidArgumentError(
            "max_line_length must be between 40 and 500");
      }
      config.max_line_length = max_length;
    } else if (key == "preserve_comments") {
      config.preserve_comments = (value == "true" || value == "yes");
    } else if (key == "trailing_commas") {
      config.trailing_commas = (value == "true" || value == "yes");
    }
    // Unknown keys are ignored for forward compatibility
  }

  return config;
}

}  // namespace

absl::StatusOr<ProtofmtConfig> ProtofmtConfig::LoadFromFile(
    const std::string& file_path) {
  std::ifstream file(file_path);
  if (!file.is_open()) {
    return absl::NotFoundError(
        absl::StrCat("Could not open config file: ", file_path));
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return LoadFromString(buffer.str());
}

absl::StatusOr<ProtofmtConfig> ProtofmtConfig::LoadFromString(
    const std::string& yaml_content) {
  return ParseSimpleYaml(yaml_content);
}

ProtofmtConfig ProtofmtConfig::Default() {
  return ProtofmtConfig();
}

}  // namespace protofmt
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
