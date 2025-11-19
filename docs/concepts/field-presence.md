# Field Presence

Field presence determines whether a field has been explicitly set. This concept differs between proto2 and proto3.

## Overview

**Presence** answers: "Was this field explicitly set, or does it just have the default value?"

| Syntax | Behavior |
|--------|----------|
| Proto2 | All fields have presence |
| Proto3 | Only some fields have presence |

## Proto3 Field Presence

In proto3, presence tracking depends on the field type:

### No Presence (Implicit Presence)

Regular singular fields have **no presence tracking**:

```protobuf
syntax = "proto3";

message User {
  string name = 1;    // No presence
  int32 age = 2;      // No presence
  bool active = 3;    // No presence
}
```

- Cannot distinguish "not set" from "set to default"
- Always serialized if non-default
- Never serialized if default value

### Has Presence (Explicit Presence)

These field types track presence:

1. **`optional` fields** (proto3 syntax)
2. **`oneof` fields**
3. **Message fields**

```protobuf
syntax = "proto3";

message User {
  optional string nickname = 1;  // Has presence
  oneof contact {                // Has presence
    string email = 2;
    string phone = 3;
  }
  Address address = 4;           // Has presence (message)
}
```

## The `optional` Keyword

Added in proto3 (v3.15+) to enable explicit presence:

```protobuf
syntax = "proto3";

message Config {
  optional int32 timeout = 1;
}
```

### API Differences

=== "C++"

    ```cpp
    // Without optional (no presence)
    config.timeout();        // Returns 0 if not set
    // No has_timeout() method

    // With optional (has presence)
    config.has_timeout();    // Returns true/false
    config.timeout();        // Returns value or default
    config.clear_timeout();  // Clears the field
    ```

=== "Java"

    ```java
    // Without optional (no presence)
    config.getTimeout();     // Returns 0 if not set
    // No hasTimeout() method

    // With optional (has presence)
    config.hasTimeout();     // Returns true/false
    config.getTimeout();     // Returns value or default
    config.clearTimeout();   // Clears the field
    ```

=== "Python"

    ```python
    # Without optional (no presence)
    config.timeout          # Returns 0 if not set
    # HasField() not available

    # With optional (has presence)
    config.HasField('timeout')  # Returns True/False
    config.timeout              # Returns value or default
    config.ClearField('timeout')
    ```

### When to Use `optional`

Use `optional` when you need to:

- Distinguish "not set" from "set to zero/empty"
- Implement nullable fields
- Support "unset" as a valid state

```protobuf
message Update {
  // Use optional for nullable semantics
  optional string new_name = 1;     // null = don't change
  optional int32 new_priority = 2;  // null = don't change
}
```

## Presence in Different Contexts

### Serialization

| Field State | Proto3 (no presence) | Proto3 (optional) |
|-------------|---------------------|-------------------|
| Not set | Not serialized | Not serialized |
| Set to default | Not serialized | Serialized |
| Set to non-default | Serialized | Serialized |

### JSON Mapping

| Field State | Proto3 (no presence) | Proto3 (optional) |
|-------------|---------------------|-------------------|
| Not set | Omitted | Omitted |
| Set to default | Omitted | Included |
| Set to non-default | Included | Included |

### Merging

When merging messages, presence affects behavior:

```cpp
Message merged;
merged.MergeFrom(base);
merged.MergeFrom(override);

// For fields with presence:
// - If override has field, it replaces base
// - If override doesn't have field, base is kept

// For fields without presence:
// - Non-default values from override replace base
// - Default values don't replace (ambiguous!)
```

## Message Field Presence

Message fields always have presence:

```protobuf
message Order {
  Address shipping = 1;  // Has presence
}
```

=== "C++"

    ```cpp
    if (order.has_shipping()) {
      // Field was set
      const Address& addr = order.shipping();
    }

    // Setting creates the field
    Address* addr = order.mutable_shipping();
    addr->set_street("123 Main St");

    // Clear removes the field
    order.clear_shipping();
    ```

=== "Python"

    ```python
    if order.HasField('shipping'):
        # Field was set
        addr = order.shipping

    # Setting creates the field
    order.shipping.street = "123 Main St"

    # Clear removes the field
    order.ClearField('shipping')
    ```

## Oneof Presence

Oneof fields always track which field is set:

```protobuf
message Content {
  oneof data {
    string text = 1;
    bytes binary = 2;
    int32 count = 3;
  }
}
```

=== "C++"

    ```cpp
    switch (content.data_case()) {
      case Content::kText:
        std::cout << content.text();
        break;
      case Content::kBinary:
        // ...
        break;
      case Content::DATA_NOT_SET:
        std::cout << "No data";
        break;
    }
    ```

=== "Python"

    ```python
    which = content.WhichOneof('data')
    if which == 'text':
        print(content.text)
    elif which == 'binary':
        # ...
        pass
    elif which is None:
        print("No data")
    ```

## Proto2 vs Proto3 Comparison

### Proto2 Presence

All fields have presence in proto2:

```protobuf
syntax = "proto2";

message User {
  optional string name = 1;   // Has presence
  required int32 id = 2;      // Has presence (must be set)
  repeated string tags = 3;   // Repeated, no presence concept
}
```

### Proto3 Presence

Proto3 simplified presence semantics:

```protobuf
syntax = "proto3";

message User {
  string name = 1;            // No presence
  int32 id = 2;               // No presence
  repeated string tags = 3;   // Repeated
  optional string bio = 4;    // Has presence (explicit)
  Address addr = 5;           // Has presence (message)
}
```

## Default Values

### Proto3 Defaults (Fixed)

| Type | Default |
|------|---------|
| Numbers | 0 |
| Booleans | false |
| Strings | "" |
| Bytes | empty |
| Enums | First value (0) |
| Messages | null/not set |

### Proto2 Defaults (Configurable)

```protobuf
syntax = "proto2";

message Config {
  optional int32 timeout = 1 [default = 30];
  optional string name = 2 [default = "default"];
}
```

## Best Practices

### 1. Use `optional` for Nullable Semantics

```protobuf
// PATCH-style updates
message UpdateUserRequest {
  string user_id = 1;
  optional string name = 2;     // null = don't change
  optional string email = 3;    // null = don't change
}
```

### 2. Use Sentinel Values When No Presence

If you can't use `optional`, define sentinels:

```protobuf
message Config {
  int32 timeout = 1;  // -1 = not set, use default
}
```

### 3. Prefer Message Wrappers for Complex Cases

```protobuf
import "google/protobuf/wrappers.proto";

message Request {
  google.protobuf.Int32Value count = 1;  // Nullable int
  google.protobuf.StringValue name = 2;  // Nullable string
}
```

### 4. Document Presence Semantics

```protobuf
message UpdateRequest {
  // If set, updates the name. If not set, name is unchanged.
  optional string name = 1;

  // Always required. Cannot be unset.
  string id = 2;
}
```

## Migration Considerations

### Proto2 to Proto3

When migrating, fields lose presence tracking:

```protobuf
// Proto2
optional int32 count = 1;  // has_count() available

// Proto3 (migrated)
int32 count = 1;           // has_count() NOT available

// Proto3 (with presence)
optional int32 count = 1;  // has_count() available
```

### Adding `optional` to Existing Fields

Adding `optional` to an existing proto3 field is safe:

- Wire format unchanged
- Existing data still parses correctly
- New code gets `has_*` methods

## Further Reading

- [Field Presence Documentation](../field_presence.md) - Detailed presence rules
- [Proto3 Presence](../implementing_proto3_presence.md) - Implementation details
- [Application Note](https://protobuf.dev/programming-guides/field_presence/)
