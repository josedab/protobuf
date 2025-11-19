// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Author: protofmt authors
//
// Configuration for the protobuf formatter.

#ifndef GOOGLE_PROTOBUF_COMPILER_PROTOFMT_PROTOFMT_CONFIG_H__
#define GOOGLE_PROTOBUF_COMPILER_PROTOFMT_PROTOFMT_CONFIG_H__

#include <string>

#include "absl/status/statusor.h"

namespace google {
namespace protobuf {
namespace compiler {
namespace protofmt {

// Configuration options for the protobuf formatter.
// These options can be loaded from a .protofmt.yaml file or set via CLI flags.
struct ProtofmtConfig {
  // Number of spaces for indentation (default: 2)
  int indent = 2;

  // Align field types and names in messages (default: false)
  bool align_fields = false;

  // Sort import statements alphabetically (default: true)
  bool sort_imports = true;

  // Maximum line length before wrapping (default: 100)
  int max_line_length = 100;

  // Preserve comments in output (default: true)
  bool preserve_comments = true;

  // Add trailing commas in option lists (default: false)
  bool trailing_commas = false;

  // Load configuration from a YAML file.
  // Returns error status if file cannot be read or parsed.
  static absl::StatusOr<ProtofmtConfig> LoadFromFile(
      const std::string& file_path);

  // Load configuration from YAML content string.
  static absl::StatusOr<ProtofmtConfig> LoadFromString(
      const std::string& yaml_content);

  // Get default configuration.
  static ProtofmtConfig Default();
};

}  // namespace protofmt
}  // namespace compiler
}  // namespace protobuf
}  // namespace google

#endif  // GOOGLE_PROTOBUF_COMPILER_PROTOFMT_PROTOFMT_CONFIG_H__
