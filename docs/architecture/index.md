# Architecture Overview

This section documents the Protocol Buffers codebase architecture for contributors and developers who want to understand how protobuf works internally.

## Codebase Structure

The Protocol Buffers repository is organized into several main areas:

```
protobuf/
├── src/                    # C++ source code
│   └── google/protobuf/
│       ├── compiler/       # protoc compiler
│       ├── io/             # I/O utilities
│       └── ...             # Runtime library
├── java/                   # Java implementation
├── python/                 # Python implementation
├── csharp/                 # C# implementation
├── objectivec/             # Objective-C implementation
├── php/                    # PHP implementation
├── ruby/                   # Ruby implementation
├── upb/                    # micro-protobuf (C implementation)
└── docs/                   # Documentation
```

## Key Components

### [Compiler Pipeline](compiler.md)

The `protoc` compiler transforms `.proto` files into language-specific code:

- **Parser** - Reads `.proto` files
- **Descriptor Builder** - Creates internal representation
- **Code Generators** - Output language-specific code

### [Runtime Libraries](runtime.md)

Each language has a runtime library for:

- Message serialization/deserialization
- Reflection and descriptors
- Well-known types

### [Key Files Reference](key-files.md)

Quick reference to important source files.

## High-Level Architecture

```
┌─────────────────┐     ┌──────────────────┐     ┌──────────────────┐
│   .proto file   │────▶│     Compiler     │────▶│  Generated Code  │
└─────────────────┘     └──────────────────┘     └──────────────────┘
                                                          │
                                                          ▼
┌─────────────────┐     ┌──────────────────┐     ┌──────────────────┐
│   Binary Data   │◀───▶│  Runtime Library │◀───▶│   Application    │
└─────────────────┘     └──────────────────┘     └──────────────────┘
```

## Design Principles

### Language Neutrality

Protocol Buffers are designed to be language-neutral:

- Schema defined once in `.proto` files
- Code generated for each target language
- Binary format compatible across languages

### Backward Compatibility

Schema evolution is supported:

- New fields can be added
- Old fields can be deprecated
- Unknown fields are preserved

### Efficiency

Optimized for performance:

- Compact binary encoding
- Zero-copy parsing where possible
- Arena allocation for memory efficiency

### Extensibility

Multiple extension points:

- Custom options for code generation
- Plugin system for protoc
- Reflection for runtime introspection

## Repository Layout

### Source Code

| Directory | Description |
|-----------|-------------|
| `src/google/protobuf/` | Core C++ runtime |
| `src/google/protobuf/compiler/` | protoc compiler |
| `src/google/protobuf/compiler/cpp/` | C++ code generator |
| `src/google/protobuf/compiler/java/` | Java code generator |
| `src/google/protobuf/compiler/python/` | Python code generator |

### Language Runtimes

| Directory | Description |
|-----------|-------------|
| `java/core/` | Java runtime |
| `java/lite/` | Java Lite runtime |
| `python/google/protobuf/` | Python runtime |
| `csharp/src/` | C# runtime |
| `upb/` | Micro-protobuf C library |

### Build Files

| File | Description |
|------|-------------|
| `CMakeLists.txt` | CMake build configuration |
| `BUILD.bazel` | Bazel build files |
| `Makefile.am` | Autotools (legacy) |

## Getting Started with Development

1. [Development Setup](../contributing/setup.md) - Set up your environment
2. [Compiler Pipeline](compiler.md) - Understand how protoc works
3. [Runtime Libraries](runtime.md) - Learn runtime architecture
4. [Contributing Guide](../contributing/index.md) - Submit your changes

## Further Reading

- [Compiler Pipeline](compiler.md) - Deep dive into protoc
- [Runtime Libraries](runtime.md) - Runtime implementation details
- [Key Files Reference](key-files.md) - Important source files
