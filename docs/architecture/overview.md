# Architecture Overview

This document provides a comprehensive overview of the Protocol Buffers architecture.

## System Architecture

Protocol Buffers consists of three main components:

1. **Schema Definition** - `.proto` files
2. **Compiler** - `protoc` and code generators
3. **Runtime Libraries** - Language-specific implementations

```
┌─────────────────────────────────────────────────────────────┐
│                    Development Time                          │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌─────────────┐     ┌──────────────┐     ┌──────────────┐ │
│  │ .proto      │────▶│   protoc     │────▶│  Generated   │ │
│  │ schemas     │     │  compiler    │     │  code        │ │
│  └─────────────┘     └──────────────┘     └──────────────┘ │
│                                                              │
├─────────────────────────────────────────────────────────────┤
│                      Runtime                                 │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐  │
│  │  Generated   │    │   Runtime    │    │   Wire       │  │
│  │  classes     │◀──▶│   library    │◀──▶│   format     │  │
│  └──────────────┘    └──────────────┘    └──────────────┘  │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

## Component Details

### Schema Layer

`.proto` files define:

- Message types
- Field types and numbers
- Enums and nested types
- Services (for RPC)
- Options and annotations

```protobuf
syntax = "proto3";
package myapp;

message User {
  string name = 1;
  int32 id = 2;
  repeated string tags = 3;
}
```

### Compiler Layer

The `protoc` compiler:

1. **Parses** `.proto` files
2. **Validates** schema correctness
3. **Resolves** imports and dependencies
4. **Invokes** code generators (built-in or plugins)

Code generators:

| Generator | Output |
|-----------|--------|
| `--cpp_out` | C++ classes |
| `--java_out` | Java classes |
| `--python_out` | Python modules |
| `--csharp_out` | C# classes |
| Plugins | Custom outputs |

### Runtime Layer

Runtime libraries provide:

- **Serialization** - Encode to wire format
- **Deserialization** - Decode from wire format
- **Reflection** - Runtime type information
- **Utilities** - JSON, text format, etc.

## Wire Format

The binary wire format uses:

- **Varints** - Variable-length integers
- **Tags** - Field number + wire type
- **Length-delimited** - Strings, bytes, messages

Example encoding:

```
Field 1 (string): "hello"
[0A][05][68 65 6C 6C 6F]
 │   │   └── "hello" UTF-8
 │   └── Length: 5
 └── Tag: field 1, wire type 2 (LEN)
```

## Language Implementations

### C++ (Reference Implementation)

The C++ implementation is the reference:

- Full-featured runtime
- Highest performance
- Arena allocation
- All protobuf features

### Java

Builder-based API:

```java
Person person = Person.newBuilder()
    .setName("Alice")
    .build();
```

Features:
- Immutable messages
- Builder pattern
- Lite runtime for Android

### Python

Pythonic API:

```python
person = Person()
person.name = "Alice"
```

Features:
- Native attribute access
- C++ extension for performance
- Dynamic message support

### Other Languages

Each language follows its conventions while maintaining wire format compatibility.

## Extension Points

### Custom Options

Add metadata to schemas:

```protobuf
extend google.protobuf.MessageOptions {
  bool deprecated = 50000;
}

message OldMessage {
  option (deprecated) = true;
}
```

### Plugins

Create custom code generators:

```bash
protoc --plugin=protoc-gen-custom=./my-plugin --custom_out=. file.proto
```

### Reflection

Access type information at runtime:

```cpp
const Descriptor* desc = message.GetDescriptor();
for (int i = 0; i < desc->field_count(); i++) {
  const FieldDescriptor* field = desc->field(i);
  // Process field...
}
```

## Performance Considerations

### Serialization

- **Zero-copy** where possible
- **Lazy parsing** for large messages
- **Packed encoding** for repeated scalars

### Memory

- **Arena allocation** for batch processing
- **String deduplication** in some implementations
- **COW (Copy-on-Write)** strings

### CPU

- **SIMD** for varint encoding
- **Branch prediction** optimization
- **Cache-friendly** layouts

## Evolution and Compatibility

### Forward Compatibility

New code can read old data:
- Unknown fields are preserved
- New fields have defaults

### Backward Compatibility

Old code can read new data:
- New fields are ignored
- Removed fields use defaults

### Schema Evolution Rules

Safe changes:
- Add new fields
- Rename fields
- Add new enum values

Unsafe changes:
- Change field numbers
- Change field types
- Remove fields without reserving

## See Also

- [Compiler Pipeline](compiler.md)
- [Runtime Libraries](runtime.md)
- [Wire Format](../concepts/wire-format.md)
