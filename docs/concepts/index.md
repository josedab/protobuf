# Core Concepts

This section covers the fundamental concepts of Protocol Buffers.

## Overview

Protocol Buffers work by:

1. **Defining schemas** - You write `.proto` files describing your data structures
2. **Generating code** - The `protoc` compiler generates language-specific classes
3. **Serializing data** - Your code uses generated classes to encode/decode data

## Key Concepts

### [Messages and Fields](messages.md)

The building blocks of Protocol Buffers:

- Message types define structured data
- Fields have types, names, and numbers
- Support for optional, repeated, and map fields
- Nested messages and enums

### [Wire Format](wire-format.md)

How Protocol Buffers encode data:

- Binary encoding for efficiency
- Varints for compact integers
- Length-delimited strings and bytes
- Tag-based field identification

### [Descriptors and Reflection](descriptors.md)

Runtime metadata and dynamic access:

- Descriptors describe message structure
- Reflection API for dynamic access
- Use cases: serialization, validation, debugging

### [Field Presence](field-presence.md)

How Protocol Buffers handle unset fields:

- Proto2 vs proto3 semantics
- Optional fields and has_* methods
- Default values and zero values

## Quick Reference

### Scalar Types

| Proto Type | Default | C++ Type | Java Type | Python Type |
|------------|---------|----------|-----------|-------------|
| `double` | 0 | `double` | `double` | `float` |
| `float` | 0 | `float` | `float` | `float` |
| `int32` | 0 | `int32_t` | `int` | `int` |
| `int64` | 0 | `int64_t` | `long` | `int` |
| `uint32` | 0 | `uint32_t` | `int`* | `int` |
| `uint64` | 0 | `uint64_t` | `long`* | `int` |
| `sint32` | 0 | `int32_t` | `int` | `int` |
| `sint64` | 0 | `int64_t` | `long` | `int` |
| `fixed32` | 0 | `uint32_t` | `int`* | `int` |
| `fixed64` | 0 | `uint64_t` | `long`* | `int` |
| `sfixed32` | 0 | `int32_t` | `int` | `int` |
| `sfixed64` | 0 | `int64_t` | `long` | `int` |
| `bool` | false | `bool` | `boolean` | `bool` |
| `string` | "" | `std::string` | `String` | `str` |
| `bytes` | "" | `std::string` | `ByteString` | `bytes` |

\* Unsigned values use signed types in Java; be careful with sign extension.

### Composite Types

| Type | Description | Example |
|------|-------------|---------|
| `message` | Structured data | `message Person { ... }` |
| `enum` | Enumerated values | `enum Status { ... }` |
| `oneof` | One of several fields | `oneof choice { A a = 1; B b = 2; }` |
| `map` | Key-value pairs | `map<string, int32> counts = 1;` |

### Field Rules

| Rule | Proto2 | Proto3 | Description |
|------|--------|--------|-------------|
| `optional` | Yes | Yes* | Zero or one value |
| `required` | Yes | No | Exactly one value |
| `repeated` | Yes | Yes | Zero or more values |

\* Proto3 fields are optional by default; `optional` keyword adds presence tracking.

## Schema Syntax

### Proto3 Example

```protobuf
syntax = "proto3";

package mypackage;

import "google/protobuf/timestamp.proto";

message MyMessage {
  // Scalar fields
  string name = 1;
  int32 count = 2;
  bool enabled = 3;

  // Enum field
  Status status = 4;

  // Nested message field
  Details details = 5;

  // Repeated field (list)
  repeated string tags = 6;

  // Map field
  map<string, int32> attributes = 7;

  // Oneof field
  oneof value {
    string text = 8;
    int32 number = 9;
  }

  // Well-known type
  google.protobuf.Timestamp created_at = 10;

  // Nested definitions
  enum Status {
    STATUS_UNSPECIFIED = 0;
    STATUS_ACTIVE = 1;
    STATUS_INACTIVE = 2;
  }

  message Details {
    string description = 1;
  }
}
```

## FAQ

### What's the difference between proto2 and proto3?

Proto3 simplifies the language by removing required fields, default values, and some other features. See the [field presence](field-presence.md) documentation for details.

### When should I use sint32 vs int32?

Use `sint32` or `sint64` when values are often negative. They use ZigZag encoding which is more efficient for negative numbers.

### How do I handle schema evolution?

See the [Next Steps](../getting-started/next-steps.md#schema-evolution) section for guidelines on safely evolving schemas.

### Can messages be self-referential?

Yes, messages can reference themselves:

```protobuf
message TreeNode {
  string value = 1;
  repeated TreeNode children = 2;
}
```
