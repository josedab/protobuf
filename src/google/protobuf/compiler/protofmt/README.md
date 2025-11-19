# protofmt - Protocol Buffer Formatter

An opinionated code formatter for Protocol Buffer schema files.

## Overview

`protofmt` formats `.proto` files to a consistent style, similar to how `gofmt` works for Go or `rustfmt` for Rust. It eliminates style debates and ensures consistent formatting across projects.

## Building

```bash
# Build using Bazel
bazel build //src/google/protobuf/compiler/protofmt:protofmt
```

## Usage

### Basic Formatting

```bash
# Format file and print to stdout
protofmt user.proto

# Format file in place
protofmt -w user.proto

# Format multiple files
protofmt -w path/to/*.proto
```

### Check Mode

```bash
# Check if files are formatted (useful for CI)
protofmt --check user.proto

# Exit code 0 = formatted, 1 = not formatted
```

### Diff Mode

```bash
# Show what would change
protofmt --diff user.proto
```

## Configuration

Configuration can be provided via a `.protofmt.yaml` file or command-line flags.

### Configuration File

Create a `.protofmt.yaml` file in your project:

```yaml
# Number of spaces for indentation (default: 2)
indent: 2

# Align field types and names (default: false)
align_fields: false

# Sort import statements alphabetically (default: true)
sort_imports: true

# Maximum line length before wrapping (default: 100)
max_line_length: 100

# Preserve comments in output (default: true)
preserve_comments: true

# Add trailing commas in option lists (default: false)
trailing_commas: false
```

### Command-Line Flags

```bash
# Override indent
protofmt --indent=4 user.proto

# Enable field alignment
protofmt --align_fields user.proto

# Disable import sorting
protofmt --sort_imports=false user.proto

# Custom config file
protofmt --config=myconfig.yaml user.proto
```

## Formatting Rules

### Indentation

- Uses 2 spaces by default (configurable)
- Consistent nesting for all block types

### Imports

- Sorted alphabetically by default
- Grouped by type (standard, public, weak)

### Messages

```protobuf
message User {
  string name = 1;
  int32 age = 2;
  repeated string tags = 3;
}
```

### Enums

```protobuf
enum Status {
  STATUS_UNKNOWN = 0;
  STATUS_ACTIVE = 1;
  STATUS_INACTIVE = 2;
}
```

### Services

```protobuf
service UserService {
  rpc GetUser(GetUserRequest) returns (User);
  rpc ListUsers(ListUsersRequest) returns (stream User);
}
```

### Field Alignment (Optional)

When `align_fields: true`:

```protobuf
message User {
  string          name    = 1;
  int32           age     = 2;
  repeated string tags    = 3;
  bool            active  = 4;
}
```

## Examples

### Before

```protobuf
syntax="proto3";package user;import "google/protobuf/timestamp.proto";
message User{string name=1;int32 age=2;google.protobuf.Timestamp created=3;}
```

### After

```protobuf
syntax = "proto3";

package user;

import "google/protobuf/timestamp.proto";

message User {
  string name = 1;
  int32 age = 2;
  google.protobuf.Timestamp created = 3;
}
```

## CI Integration

### GitHub Actions

```yaml
name: Format Check

on: pull_request

jobs:
  format:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Check formatting
        run: |
          bazel build //src/google/protobuf/compiler/protofmt:protofmt
          bazel-bin/src/google/protobuf/compiler/protofmt/protofmt --check **/*.proto
```

### Pre-commit Hook

```yaml
# .pre-commit-config.yaml
repos:
  - repo: local
    hooks:
      - id: protofmt
        name: protofmt
        entry: protofmt --check
        language: system
        types: [proto]
```

### Makefile

```makefile
.PHONY: format format-check

format:
	protofmt -w **/*.proto

format-check:
	protofmt --check **/*.proto
```

## Editor Integration

### VS Code

Add to `.vscode/settings.json`:

```json
{
  "editor.formatOnSave": true,
  "[proto3]": {
    "editor.defaultFormatter": "protobuf.protofmt"
  }
}
```

## Testing

```bash
# Run tests
bazel test //src/google/protobuf/compiler/protofmt:protofmt_test
bazel test //src/google/protobuf/compiler/protofmt:protofmt_config_test
```

## Architecture

```
protofmt/
├── protofmt_config.h    # Configuration structures
├── protofmt_config.cc   # Configuration parsing
├── protofmt.h           # Formatter interface
├── protofmt.cc          # Formatter implementation
├── protofmt_main.cc     # CLI entry point
├── BUILD.bazel          # Build configuration
└── README.md            # This file
```

The formatter works by:
1. Parsing the proto file using the protobuf compiler parser
2. Transforming the AST according to formatting rules
3. Printing the formatted output

## Limitations

- Comment preservation is best-effort
- Very long lines may not wrap perfectly
- Some advanced proto3 features may need additional handling

## Contributing

1. Make changes
2. Run tests: `bazel test //src/google/protobuf/compiler/protofmt/...`
3. Format your code: Use the project's standard formatting

## License

Same license as Protocol Buffers.
