# Compiler Pipeline

The Protocol Buffers compiler (`protoc`) transforms `.proto` schema files into language-specific source code.

## Overview

```
┌────────────┐    ┌────────────┐    ┌────────────┐    ┌────────────┐
│   Parse    │───▶│  Validate  │───▶│  Resolve   │───▶│  Generate  │
│            │    │            │    │            │    │            │
└────────────┘    └────────────┘    └────────────┘    └────────────┘
```

## Pipeline Stages

### 1. Parsing

The parser reads `.proto` files and creates an AST.

**Key file**: `src/google/protobuf/compiler/parser.cc`

```cpp
// Parser converts text to FileDescriptorProto
class Parser {
  bool Parse(io::Tokenizer* input, FileDescriptorProto* file);
};
```

The parser handles:
- Tokenization
- Syntax validation
- Option parsing
- Import resolution

### 2. Descriptor Building

Converts parsed protos into `Descriptor` objects.

**Key file**: `src/google/protobuf/descriptor_builder.cc`

The `DescriptorBuilder`:
- Validates semantics
- Resolves type references
- Computes defaults
- Builds cross-references

```cpp
// DescriptorPool manages all descriptors
class DescriptorPool {
  const FileDescriptor* BuildFile(const FileDescriptorProto& proto);
};
```

### 3. Code Generation

Generators output language-specific code from descriptors.

**Key interface**: `src/google/protobuf/compiler/code_generator.h`

```cpp
class CodeGenerator {
  virtual bool Generate(
      const FileDescriptor* file,
      const std::string& parameter,
      GeneratorContext* context,
      std::string* error) const = 0;
};
```

## Built-in Code Generators

### C++ Generator

**Location**: `src/google/protobuf/compiler/cpp/`

Key files:
- `generator.cc` - Entry point
- `message.cc` - Message class generation
- `field.cc` - Field accessor generation
- `enum.cc` - Enum generation

Output:
- `*.pb.h` - Header file
- `*.pb.cc` - Implementation

### Java Generator

**Location**: `src/google/protobuf/compiler/java/`

Key files:
- `generator.cc` - Entry point
- `message.cc` - Message class generation
- `enum.cc` - Enum generation

Output:
- `*.java` - Single file or multiple files per message

### Python Generator

**Location**: `src/google/protobuf/compiler/python/`

Key files:
- `generator.cc` - Entry point
- `pyi_generator.cc` - Type stub generation

Output:
- `*_pb2.py` - Python module
- `*_pb2.pyi` - Type stubs (optional)

## Plugin System

External code generators run as separate processes.

### Plugin Protocol

1. protoc writes `CodeGeneratorRequest` to plugin stdin
2. Plugin processes the request
3. Plugin writes `CodeGeneratorResponse` to stdout

**Protocol definition**: `src/google/protobuf/compiler/plugin.proto`

```protobuf
message CodeGeneratorRequest {
  repeated string file_to_generate = 1;
  string parameter = 2;
  repeated FileDescriptorProto proto_file = 15;
}

message CodeGeneratorResponse {
  optional string error = 1;
  repeated File file = 15;

  message File {
    optional string name = 1;
    optional string content = 15;
  }
}
```

### Creating a Plugin

```cpp
#include <google/protobuf/compiler/plugin.h>
#include <google/protobuf/compiler/code_generator.h>

class MyGenerator : public CodeGenerator {
  bool Generate(const FileDescriptor* file,
                const std::string& parameter,
                GeneratorContext* context,
                std::string* error) const override {
    // Generate code...
    return true;
  }
};

int main(int argc, char* argv[]) {
  MyGenerator generator;
  return PluginMain(argc, argv, &generator);
}
```

Usage:

```bash
protoc --plugin=protoc-gen-mine=./my-plugin --mine_out=. file.proto
```

## Command-Line Interface

**Key file**: `src/google/protobuf/compiler/command_line_interface.cc`

Main options:

| Option | Description |
|--------|-------------|
| `--proto_path` / `-I` | Import search path |
| `--cpp_out` | Generate C++ |
| `--java_out` | Generate Java |
| `--python_out` | Generate Python |
| `--plugin` | Use external generator |
| `--encode` | Encode text to binary |
| `--decode` | Decode binary to text |
| `--decode_raw` | Decode without schema |

## Descriptor Model

The descriptor hierarchy:

```
FileDescriptor
├── MessageDescriptor
│   ├── FieldDescriptor
│   ├── OneofDescriptor
│   ├── EnumDescriptor
│   └── MessageDescriptor (nested)
├── EnumDescriptor
├── ServiceDescriptor
│   └── MethodDescriptor
└── Extension
```

**Key file**: `src/google/protobuf/descriptor.h`

## Important Classes

### FileDescriptor

Represents a `.proto` file:

```cpp
class FileDescriptor {
  const std::string& name() const;
  const std::string& package() const;
  int message_type_count() const;
  const Descriptor* message_type(int index) const;
  // ...
};
```

### Descriptor

Represents a message type:

```cpp
class Descriptor {
  const std::string& name() const;
  const std::string& full_name() const;
  int field_count() const;
  const FieldDescriptor* field(int index) const;
  const FieldDescriptor* FindFieldByName(const std::string& name) const;
  // ...
};
```

### FieldDescriptor

Represents a field:

```cpp
class FieldDescriptor {
  const std::string& name() const;
  int number() const;
  Type type() const;
  CppType cpp_type() const;
  bool is_repeated() const;
  bool is_optional() const;
  // ...
};
```

## Compilation Flow Example

For input `message.proto`:

```protobuf
syntax = "proto3";
package example;
message Person { string name = 1; }
```

1. **Parse**: Create `FileDescriptorProto`
2. **Build**: Create `FileDescriptor` with `Descriptor` for Person
3. **Generate**: Invoke C++ generator to create:
   - `message.pb.h`: Person class declaration
   - `message.pb.cc`: Person class implementation

## Debugging

### View Parsed Proto

```bash
protoc --encode=google.protobuf.FileDescriptorProto \
  google/protobuf/descriptor.proto < file.proto
```

### Decode Raw

```bash
cat data.bin | protoc --decode_raw
```

### Print Descriptor

```bash
protoc --descriptor_set_out=file.desc file.proto
```

## Source Code References

| Component | Location |
|-----------|----------|
| Main entry | `src/google/protobuf/compiler/main.cc` |
| CLI | `src/google/protobuf/compiler/command_line_interface.cc` |
| Parser | `src/google/protobuf/compiler/parser.cc` |
| Descriptor | `src/google/protobuf/descriptor.h` |
| C++ gen | `src/google/protobuf/compiler/cpp/` |
| Java gen | `src/google/protobuf/compiler/java/` |
| Python gen | `src/google/protobuf/compiler/python/` |

## See Also

- [Runtime Libraries](runtime.md)
- [Key Files Reference](key-files.md)
- [Contributing Guide](../contributing/index.md)
