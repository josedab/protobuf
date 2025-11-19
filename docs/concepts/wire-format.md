# Wire Format

The wire format is the binary encoding Protocol Buffers use to serialize data. Understanding it helps with debugging, optimization, and building custom implementations.

## Overview

Protocol Buffers encode messages as a sequence of key-value pairs:

```
[Tag][Value][Tag][Value]...
```

Each tag contains:
- **Field number** - Identifies which field
- **Wire type** - How to parse the value

## Wire Types

| Wire Type | Value | Used For |
|-----------|-------|----------|
| VARINT | 0 | int32, int64, uint32, uint64, sint32, sint64, bool, enum |
| I64 | 1 | fixed64, sfixed64, double |
| LEN | 2 | string, bytes, embedded messages, packed repeated |
| SGROUP | 3 | group start (deprecated) |
| EGROUP | 4 | group end (deprecated) |
| I32 | 5 | fixed32, sfixed32, float |

## Tag Encoding

Tags combine field number and wire type:

```
tag = (field_number << 3) | wire_type
```

Example for field number 1, wire type 0:

```
tag = (1 << 3) | 0 = 8
```

### Varint Encoding

Tags and many values use varints (variable-length integers):

- Use 7 bits per byte for data
- MSB indicates if more bytes follow

Example: Encoding `300`:

```
300 = 100101100 binary
     = 0000010 0101100 (split into 7-bit groups)

Encoded: 10101100 00000010 (little-endian, MSB set on first byte)
       = 0xAC 0x02
```

## Encoding Details

### Signed Integers

#### Standard Encoding (int32/int64)

Negative numbers use all 10 bytes (sign-extended):

```python
# -1 encoded as int32 uses 10 bytes!
# 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0xFF 0x01
```

#### ZigZag Encoding (sint32/sint64)

More efficient for negative numbers:

```
zigzag(n) = (n << 1) ^ (n >> 31)  // for sint32
```

| Original | ZigZag Encoded |
|----------|----------------|
| 0 | 0 |
| -1 | 1 |
| 1 | 2 |
| -2 | 3 |
| 2 | 4 |

!!! tip "Use sint32/sint64 for negative values"
    If your values are often negative, use `sint32` or `sint64` for smaller wire size.

### Fixed-Width Types

These use exactly 4 or 8 bytes (little-endian):

| Type | Bytes | Use Case |
|------|-------|----------|
| fixed32/sfixed32 | 4 | Values > 2^28 |
| fixed64/sfixed64 | 8 | Values > 2^56 |
| float | 4 | IEEE 754 |
| double | 8 | IEEE 754 |

### Strings and Bytes

Length-delimited with varint length prefix:

```
[length (varint)][data (length bytes)]
```

Example encoding "test":

```
0A         // tag: field 1, wire type 2 (LEN)
04         // length: 4 bytes
74 65 73 74 // "test"
```

### Embedded Messages

Same as strings - length-prefixed:

```protobuf
message Outer {
  Inner inner = 1;
}
message Inner {
  int32 value = 1;
}
```

Encoded as:

```
0A         // tag: field 1, wire type 2
03         // length: 3 bytes
  08       // inner tag: field 1, wire type 0
  96 01    // value: 150 (varint)
```

### Repeated Fields

#### Standard Encoding

Each element has its own tag:

```
[tag][value][tag][value][tag][value]...
```

#### Packed Encoding (Default in Proto3)

All elements in one length-delimited blob:

```
[tag (wire type 2)][total length][value][value][value]...
```

Example - repeated int32 with values [3, 270, 86942]:

```
22        // tag: field 4, wire type 2
06        // length: 6 bytes
03        // 3
8E 02     // 270
9E A7 05  // 86942
```

!!! note
    Packed encoding only works for primitive types, not messages.

### Map Fields

Maps encode as repeated key-value messages:

```protobuf
map<string, int32> my_map = 1;
```

Equivalent to:

```protobuf
message MapEntry {
  string key = 1;
  int32 value = 2;
}
repeated MapEntry my_map = 1;
```

## Example Encoding

Given this message:

```protobuf
message Test {
  int32 a = 1;
  string b = 2;
}
```

With values `a = 150`, `b = "testing"`:

```
08 96 01      // field 1, varint: 150
12 07         // field 2, length: 7
74 65 73 74   // "test"
69 6E 67      // "ing"
```

Breakdown:

| Bytes | Description |
|-------|-------------|
| `08` | Tag: field 1, wire type 0 (VARINT) |
| `96 01` | Value: 150 |
| `12` | Tag: field 2, wire type 2 (LEN) |
| `07` | Length: 7 |
| `74 65 73 74 69 6E 67` | "testing" |

## Field Order

Messages can be serialized in any field order. Parsers must handle:

- Fields in any order
- Multiple values for the same field (last wins for scalars, append for repeated)
- Unknown fields (preserved in proto3 since v3.5)

## Unknown Fields

When a parser encounters an unknown field number:

1. Uses wire type to determine how to skip the value
2. Preserves the bytes (since proto3 v3.5)
3. Includes them when re-serializing

This enables forward compatibility.

## Size Optimization

### Field Number Selection

| Field Numbers | Tag Bytes | Use For |
|---------------|-----------|---------|
| 1-15 | 1 | Frequently set fields |
| 16-2047 | 2 | Less frequent fields |
| 2048+ | 3+ | Rarely used fields |

### Type Selection

| Scenario | Best Type |
|----------|-----------|
| Small positive integers | int32/int64 |
| Often negative | sint32/sint64 |
| Large numbers (> 2^28) | fixed32/fixed64 |
| Repeated primitives | Default packed |

### Message Design

1. **Avoid deeply nested messages** - Each level adds length prefix overhead
2. **Use packed repeated** - Much smaller than unpacked
3. **Consider fixed types** - For large numbers, fixed can be smaller

## Debugging

### Decoding Raw Bytes

Use `protoc --decode_raw` to inspect binary data:

```bash
cat message.bin | protoc --decode_raw
```

Output:

```
1: 150
2: "testing"
```

### Calculating Sizes

Estimate wire size:

```cpp
size_t size = message.ByteSizeLong();
```

```python
size = message.ByteSize()
```

```java
int size = message.getSerializedSize();
```

## Deterministic Serialization

By default, serialization order may vary. For consistent output:

=== "C++"

    ```cpp
    google::protobuf::io::CodedOutputStream::SetSerializationDeterministic(true);
    ```

=== "Java"

    ```java
    // Not guaranteed deterministic
    ```

=== "Python"

    ```python
    message.SerializeToString(deterministic=True)
    ```

Use cases: caching, hashing, testing.

## Further Reading

- [Protocol Buffers Encoding Guide](https://protobuf.dev/programming-guides/encoding/)
- [Varint Specification](https://protobuf.dev/programming-guides/encoding/#varints)
