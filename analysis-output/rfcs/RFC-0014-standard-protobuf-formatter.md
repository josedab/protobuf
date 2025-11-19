# RFC-0014: Standard Protobuf Formatter

**Status:** Draft
**Author:** Codebase Analysis
**Created:** November 19, 2025
**Effort:** 4 weeks
**Category:** Strategic

---

## Summary

Create `protofmt`, an official opinionated code formatter for Protocol Buffer schema files, ending style debates and ensuring consistent formatting across the ecosystem, similar to gofmt for Go or rustfmt for Rust.

## Motivation

### Problem Statement

No standard formatting leads to:

1. **Bikeshedding** - Code review time spent on style debates
2. **Inconsistent styles** - Different formatting across projects
3. **Difficult diffs** - Formatting changes mixed with logic changes
4. **Merge conflicts** - Different developers format differently
5. **Manual effort** - Time spent formatting instead of coding

### Industry Precedent

Successful formatters:
- **Go:** gofmt (universal adoption)
- **Rust:** rustfmt (required for contributions)
- **JavaScript:** prettier (dominant in ecosystem)
- **Python:** black (growing adoption)

### Current State

```protobuf
// Developer A's style
message User{string name=1;int32 age=2;}

// Developer B's style
message User {
    string name = 1;
    int32 age = 2;
}

// Developer C's style
message User
{
  string name = 1 ;
  int32  age  = 2 ;
}
```

## Detailed Design

### Formatter Rules

#### Indentation
- 2 spaces (not tabs)
- Consistent nesting

#### Field Formatting
```protobuf
// Formatted output
message User {
  string name = 1;
  int32 age = 2;
  repeated string tags = 3;
}
```

#### Field Alignment (Optional)
```protobuf
// With alignment enabled
message User {
  string          name    = 1;
  int32           age     = 2;
  repeated string tags    = 3;
  bool            active  = 4;
}
```

#### Options Formatting
```protobuf
// Single option
string email = 1 [deprecated = true];

// Multiple options
string phone = 2 [
  deprecated = true,
  (custom.option) = "value"
];
```

#### Comments
```protobuf
// Leading comment preserved
message User {
  // Field comment
  string name = 1;  // Trailing comment
}
```

#### Import Sorting
```protobuf
// Sorted imports
import "google/protobuf/any.proto";
import "google/protobuf/timestamp.proto";
import "mycompany/common.proto";
```

#### Blank Lines
```protobuf
syntax = "proto3";

package mycompany.users;

import "google/protobuf/timestamp.proto";

option java_package = "com.mycompany.users";

// Messages separated by blank line
message User {
  string name = 1;
  int32 age = 2;
}

message UserList {
  repeated User users = 1;
}
```

### Configuration

Minimal, opinionated configuration:

```yaml
# .protofmt.yaml
indent: 2                    # Spaces (default: 2)
align_fields: false          # Align = signs (default: false)
sort_imports: true           # Sort imports (default: true)
max_line_length: 100         # Wrap long lines (default: 100)
preserve_comments: true      # Keep comments (default: true)
trailing_commas: false       # Options trailing comma (default: false)
```

### CLI Interface

```bash
# Format file in place
protofmt -w user.proto

# Format multiple files
protofmt -w **/*.proto

# Check formatting (for CI)
protofmt --check **/*.proto

# Output to stdout
protofmt user.proto

# Show diff
protofmt --diff user.proto

# Use specific config
protofmt --config=.protofmt.yaml user.proto
```

### Protoc Plugin Mode

```bash
# Run as protoc plugin
protoc --protofmt_out=. user.proto

# With options
protoc --protofmt_out=align_fields=true:. user.proto
```

### Implementation Architecture

```
┌─────────────────────────────────────────┐
│              protofmt CLI               │
├─────────────────────────────────────────┤
│          Configuration Loader           │
├─────────────────────────────────────────┤
│     Proto Parser (reuse from protoc)    │
├─────────────────────────────────────────┤
│           AST Transformer               │
│  (applies formatting rules to AST)      │
├─────────────────────────────────────────┤
│            Code Printer                 │
│  (AST → formatted string)               │
└─────────────────────────────────────────┘
```

### Before/After Examples

#### Example 1: Basic Formatting

**Before:**
```protobuf
syntax="proto3";package user;import "google/protobuf/timestamp.proto";
message User{string name=1;int32 age=2;google.protobuf.Timestamp created=3;}
```

**After:**
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

#### Example 2: Options

**Before:**
```protobuf
message Config{string value=1[deprecated=true,(custom.opt)={field:"value",other:123}];}
```

**After:**
```protobuf
message Config {
  string value = 1 [
    deprecated = true,
    (custom.opt) = {
      field: "value",
      other: 123
    }
  ];
}
```

#### Example 3: Services

**Before:**
```protobuf
service UserService{rpc GetUser(GetUserRequest)returns(User);rpc ListUsers(ListUsersRequest)returns(stream User);}
```

**After:**
```protobuf
service UserService {
  rpc GetUser(GetUserRequest) returns (User);
  rpc ListUsers(ListUsersRequest) returns (stream User);
}
```

## Example Usage

### Format on Save (VS Code)

```json
// .vscode/settings.json
{
  "editor.formatOnSave": true,
  "[proto3]": {
    "editor.defaultFormatter": "protobuf.protofmt"
  }
}
```

### Pre-commit Hook

```yaml
# .pre-commit-config.yaml
repos:
  - repo: https://github.com/protocolbuffers/protobuf
    rev: v4.0.0
    hooks:
      - id: protofmt
        args: [--check]
```

### CI Integration

```yaml
# .github/workflows/format.yml
name: Format Check

on: pull_request

jobs:
  format:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Install protofmt
        run: |
          curl -L https://github.com/protocolbuffers/protobuf/releases/download/v4.0.0/protofmt-linux-amd64 -o protofmt
          chmod +x protofmt

      - name: Check formatting
        run: ./protofmt --check **/*.proto
```

### Makefile Integration

```makefile
.PHONY: format format-check

format:
	protofmt -w **/*.proto

format-check:
	protofmt --check **/*.proto
```

## Implementation Plan

### Week 1: Core Formatter
- [ ] Implement proto parser (or reuse)
- [ ] Build AST transformation rules
- [ ] Implement code printer

### Week 2: CLI and Configuration
- [ ] Build CLI interface
- [ ] Configuration file support
- [ ] Stdin/stdout support

### Week 3: Editor Integration
- [ ] VS Code extension
- [ ] IntelliJ plugin
- [ ] vim/neovim support

### Week 4: Testing and Release
- [ ] Format test suite
- [ ] Performance testing
- [ ] Documentation
- [ ] Release binaries

## Backwards Compatibility

### Non-Breaking
- Purely additive tool
- Existing protos work unchanged
- Opt-in usage

### Ecosystem Integration
- Works with existing build systems
- Compatible with protoc plugins
- Editor integration

## Alternatives Considered

### Alternative 1: clang-format
- **Pro:** Existing tool
- **Con:** Not proto-aware
- **Decision:** Need proto-specific formatter

### Alternative 2: Multiple Community Formatters
- **Pro:** Already exist
- **Con:** Fragments ecosystem, inconsistent
- **Decision:** Need official standard

### Alternative 3: Configurable Only
- **Pro:** Everyone can customize
- **Con:** Defeats purpose of standardization
- **Decision:** Opinionated with minimal config

## Open Questions

1. **Field ordering** - Enforce numeric order?
   - Suggestion: Optional, off by default

2. **Comment preservation** - How to handle edge cases?
   - Suggestion: Best effort, document limitations

3. **Very long enums** - How to format?
   - Suggestion: One per line, with alignment option

## Success Criteria

- [ ] Formats all valid proto3 files correctly
- [ ] Round-trip idempotent (format twice = same output)
- [ ] Performance: <100ms for typical file
- [ ] Editor plugins for top 3 editors
- [ ] Adopted by official protobuf examples

## Effort Estimation

| Task | Days |
|------|------|
| Core formatter | 8 |
| CLI | 3 |
| Configuration | 2 |
| Editor plugins | 4 |
| Testing | 3 |
| Documentation | 2 |
| **Total** | **22** (4 weeks) |

---

## References

- [gofmt](https://golang.org/cmd/gofmt/)
- [rustfmt](https://github.com/rust-lang/rustfmt)
- [prettier](https://prettier.io/)
- [black](https://github.com/psf/black)
- [buf format](https://buf.build/docs/format/usage)
