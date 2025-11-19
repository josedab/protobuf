// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Author: protofmt authors
//
// Main entry point for the protofmt command-line tool.

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/log/globals.h"
#include "absl/log/initialize.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "google/protobuf/compiler/protofmt/protofmt.h"
#include "google/protobuf/compiler/protofmt/protofmt_config.h"

// Command-line flags
ABSL_FLAG(bool, w, false,
          "Write result to source file(s) instead of stdout");

ABSL_FLAG(bool, check, false,
          "Check if file(s) are formatted correctly. Exit with non-zero status "
          "if any file is not formatted.");

ABSL_FLAG(bool, diff, false,
          "Display diff of formatting changes instead of actual output");

ABSL_FLAG(std::string, config, "",
          "Path to configuration file (default: .protofmt.yaml)");

ABSL_FLAG(int, indent, 2,
          "Number of spaces for indentation (default: 2)");

ABSL_FLAG(bool, align_fields, false,
          "Align field types and names (default: false)");

ABSL_FLAG(bool, sort_imports, true,
          "Sort import statements alphabetically (default: true)");

ABSL_FLAG(int, max_line_length, 100,
          "Maximum line length before wrapping (default: 100)");

ABSL_FLAG(bool, version, false,
          "Print version information and exit");

ABSL_FLAG(bool, help_all, false,
          "Show all available flags");

namespace {

constexpr const char* kVersion = "0.1.0";

// Read file contents into a string.
bool ReadFile(const std::string& path, std::string* content) {
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Error: Could not open file: " << path << "\n";
    return false;
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  *content = buffer.str();
  return true;
}

// Write string contents to a file.
bool WriteFile(const std::string& path, const std::string& content) {
  std::ofstream file(path);
  if (!file.is_open()) {
    std::cerr << "Error: Could not write to file: " << path << "\n";
    return false;
  }

  file << content;
  return true;
}

// Load configuration from file or flags.
google::protobuf::compiler::protofmt::ProtofmtConfig LoadConfig() {
  using google::protobuf::compiler::protofmt::ProtofmtConfig;

  ProtofmtConfig config = ProtofmtConfig::Default();

  // Try to load from config file if specified or default exists
  std::string config_path = absl::GetFlag(FLAGS_config);
  if (config_path.empty()) {
    // Check for default config file
    std::ifstream test_file(".protofmt.yaml");
    if (test_file.is_open()) {
      config_path = ".protofmt.yaml";
      test_file.close();
    }
  }

  if (!config_path.empty()) {
    auto config_or = ProtofmtConfig::LoadFromFile(config_path);
    if (config_or.ok()) {
      config = config_or.value();
    } else {
      std::cerr << "Warning: Could not load config from " << config_path
                << ": " << config_or.status().message() << "\n";
    }
  }

  // Override with command-line flags
  // Only override if flags are explicitly set
  config.indent = absl::GetFlag(FLAGS_indent);
  config.align_fields = absl::GetFlag(FLAGS_align_fields);
  config.sort_imports = absl::GetFlag(FLAGS_sort_imports);
  config.max_line_length = absl::GetFlag(FLAGS_max_line_length);

  return config;
}

// Process a single proto file.
bool ProcessFile(const std::string& path,
                 const google::protobuf::compiler::protofmt::ProtofmtConfig& config,
                 bool write_in_place, bool check_only, bool show_diff,
                 bool* any_unformatted) {
  using google::protobuf::compiler::protofmt::ProtobufFormatter;

  std::string content;
  if (!ReadFile(path, &content)) {
    return false;
  }

  ProtobufFormatter formatter(config);

  if (check_only) {
    auto is_formatted_or = formatter.IsFormatted(content);
    if (!is_formatted_or.ok()) {
      std::cerr << "Error formatting " << path << ": "
                << is_formatted_or.status().message() << "\n";
      return false;
    }

    if (!is_formatted_or.value()) {
      std::cerr << path << " is not formatted correctly\n";
      *any_unformatted = true;
    }
    return true;
  }

  if (show_diff) {
    auto diff_or = formatter.GetDiff(content);
    if (!diff_or.ok()) {
      std::cerr << "Error formatting " << path << ": "
                << diff_or.status().message() << "\n";
      return false;
    }

    if (!diff_or.value().empty()) {
      std::cout << "--- " << path << " (original)\n";
      std::cout << "+++ " << path << " (formatted)\n";
      std::cout << diff_or.value();
    }
    return true;
  }

  auto result_or = formatter.Format(content);
  if (!result_or.ok()) {
    std::cerr << "Error formatting " << path << ": "
              << result_or.status().message() << "\n";
    return false;
  }

  const std::string& formatted = result_or.value().output;

  if (write_in_place) {
    if (formatted != content) {
      if (!WriteFile(path, formatted)) {
        return false;
      }
      std::cerr << "Formatted: " << path << "\n";
    }
  } else {
    std::cout << formatted;
  }

  return true;
}

void PrintVersion() {
  std::cout << "protofmt version " << kVersion << "\n";
  std::cout << "Protocol Buffer formatter\n";
}

void PrintUsage() {
  std::cout << "Usage: protofmt [flags] [file.proto ...]\n\n";
  std::cout << "Format Protocol Buffer schema files.\n\n";
  std::cout << "Examples:\n";
  std::cout << "  protofmt user.proto           Format and print to stdout\n";
  std::cout << "  protofmt -w user.proto        Format in place\n";
  std::cout << "  protofmt -w **/*.proto        Format all .proto files\n";
  std::cout << "  protofmt --check user.proto   Check if formatted\n";
  std::cout << "  protofmt --diff user.proto    Show formatting diff\n";
  std::cout << "\nFlags:\n";
}

}  // namespace

int main(int argc, char* argv[]) {
  absl::SetProgramUsageMessage(
      "Format Protocol Buffer schema files.\n"
      "Usage: protofmt [flags] [file.proto ...]");

  // Initialize logging
  absl::InitializeLog();

  std::vector<char*> positional_args = absl::ParseCommandLine(argc, argv);

  // Handle version flag
  if (absl::GetFlag(FLAGS_version)) {
    PrintVersion();
    return 0;
  }

  // Collect input files (skip program name)
  std::vector<std::string> input_files;
  for (size_t i = 1; i < positional_args.size(); ++i) {
    input_files.push_back(positional_args[i]);
  }

  // If no files specified, check for stdin
  if (input_files.empty()) {
    PrintUsage();
    std::cerr << "\nError: No input files specified.\n";
    return 1;
  }

  // Load configuration
  auto config = LoadConfig();

  // Get mode flags
  bool write_in_place = absl::GetFlag(FLAGS_w);
  bool check_only = absl::GetFlag(FLAGS_check);
  bool show_diff = absl::GetFlag(FLAGS_diff);

  // Validate flag combinations
  if (check_only && write_in_place) {
    std::cerr << "Error: Cannot use both --check and -w flags\n";
    return 1;
  }
  if (check_only && show_diff) {
    std::cerr << "Error: Cannot use both --check and --diff flags\n";
    return 1;
  }

  bool success = true;
  bool any_unformatted = false;

  for (const auto& file : input_files) {
    // Verify it's a .proto file
    if (!absl::EndsWith(file, ".proto")) {
      std::cerr << "Warning: Skipping non-.proto file: " << file << "\n";
      continue;
    }

    if (!ProcessFile(file, config, write_in_place, check_only, show_diff,
                     &any_unformatted)) {
      success = false;
    }
  }

  if (check_only && any_unformatted) {
    return 1;
  }

  return success ? 0 : 1;
}
