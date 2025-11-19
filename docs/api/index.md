# API Reference

Detailed API documentation for Protocol Buffers.

## Language-Specific References

Each language has comprehensive API documentation:

### C++

- [C++ API Reference](../languages/cpp/api-reference.md) - Local guide
- [Full C++ API Docs](https://protobuf.dev/reference/cpp/api-docs/) - Complete reference

Key namespaces:
- `google::protobuf` - Core classes
- `google::protobuf::io` - I/O streams
- `google::protobuf::util` - Utilities

### Java

- [Java API Reference](../languages/java/api-reference.md) - Local guide
- [Full Java API Docs](https://protobuf.dev/reference/java/api-docs/) - Complete reference

Key packages:
- `com.google.protobuf` - Core classes
- `com.google.protobuf.util` - Utilities

### Python

- [Python API Reference](../languages/python/api-reference.md) - Local guide
- [Full Python API Docs](https://googleapis.dev/python/protobuf/latest/) - Complete reference

Key modules:
- `google.protobuf.message` - Message base class
- `google.protobuf.descriptor` - Descriptors
- `google.protobuf.json_format` - JSON conversion

## Common APIs

### Serialization

All languages provide these core operations:

| Operation | C++ | Java | Python |
|-----------|-----|------|--------|
| To bytes | `SerializeToString()` | `toByteArray()` | `SerializeToString()` |
| From bytes | `ParseFromString()` | `parseFrom()` | `ParseFromString()` |
| To stream | `SerializeToOstream()` | `writeTo()` | N/A |
| From stream | `ParseFromIstream()` | `parseFrom()` | N/A |

### Field Access

| Operation | C++ | Java | Python |
|-----------|-----|------|--------|
| Get scalar | `field()` | `getField()` | `message.field` |
| Set scalar | `set_field()` | `setField()` | `message.field = x` |
| Has field | `has_field()` | `hasField()` | `HasField()` |
| Clear field | `clear_field()` | `clearField()` | `ClearField()` |

### Repeated Fields

| Operation | C++ | Java | Python |
|-----------|-----|------|--------|
| Size | `field_size()` | `getFieldCount()` | `len(field)` |
| Get | `field(i)` | `getField(i)` | `field[i]` |
| Add | `add_field()` | `addField()` | `field.add()` |
| Clear | `clear_field()` | `clearField()` | `del field[:]` |

## Proto File Options

Options that affect generated code:

### File Options

```protobuf
option java_package = "com.example";           // Java package
option java_outer_classname = "MyProtos";      // Java class name
option java_multiple_files = true;             // Separate Java files
option go_package = "github.com/example/pkg";  // Go package
option csharp_namespace = "Example";           // C# namespace
option optimize_for = LITE_RUNTIME;            // Optimization level
```

### Message Options

```protobuf
message MyMessage {
  option deprecated = true;
  option map_entry = false;
}
```

### Field Options

```protobuf
message MyMessage {
  string field = 1 [
    deprecated = true,
    json_name = "customName"
  ];

  repeated int32 values = 2 [packed = true];
}
```

## Well-Known Types

Common types provided by Protocol Buffers:

| Type | Import | Description |
|------|--------|-------------|
| `Any` | `google/protobuf/any.proto` | Arbitrary message container |
| `Timestamp` | `google/protobuf/timestamp.proto` | Point in time |
| `Duration` | `google/protobuf/duration.proto` | Time span |
| `Struct` | `google/protobuf/struct.proto` | Dynamic JSON-like |
| `Value` | `google/protobuf/struct.proto` | Dynamic value |
| `Empty` | `google/protobuf/empty.proto` | Empty message |
| `Int32Value` | `google/protobuf/wrappers.proto` | Nullable int32 |
| `StringValue` | `google/protobuf/wrappers.proto` | Nullable string |
| `BoolValue` | `google/protobuf/wrappers.proto` | Nullable bool |

### Using Well-Known Types

```protobuf
import "google/protobuf/timestamp.proto";
import "google/protobuf/any.proto";

message Event {
  google.protobuf.Timestamp time = 1;
  google.protobuf.Any payload = 2;
}
```

## JSON Mapping

Protocol Buffers can convert to/from JSON:

| Proto Type | JSON Type |
|------------|-----------|
| message | object |
| enum | string (name) |
| bool | boolean |
| int32, int64 | number |
| float, double | number |
| string | string |
| bytes | base64 string |
| repeated | array |
| map | object |

## Wire Format Reference

Binary encoding quick reference:

| Wire Type | Value | Used For |
|-----------|-------|----------|
| VARINT | 0 | int32, int64, uint32, uint64, sint32, sint64, bool, enum |
| I64 | 1 | fixed64, sfixed64, double |
| LEN | 2 | string, bytes, messages, packed repeated |
| I32 | 5 | fixed32, sfixed32, float |

## External Documentation

- [Protocol Buffers Language Guide](https://protobuf.dev/programming-guides/proto3/)
- [Style Guide](https://protobuf.dev/programming-guides/style/)
- [Encoding Reference](https://protobuf.dev/programming-guides/encoding/)

## See Also

- [Core Concepts](../concepts/index.md)
- [Language Guides](../languages/index.md)
