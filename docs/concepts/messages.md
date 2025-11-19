# Messages and Fields

Messages are the fundamental building blocks of Protocol Buffers. They define the structure of your data.

## Defining Messages

A message is a collection of typed fields:

```protobuf
syntax = "proto3";

message SearchRequest {
  string query = 1;
  int32 page_number = 2;
  int32 results_per_page = 3;
}
```

Each field has:

- **Type** - The data type (`string`, `int32`, etc.)
- **Name** - A unique identifier (`query`, `page_number`)
- **Number** - A unique integer tag (1, 2, 3)

## Field Types

### Scalar Types

Basic types for storing simple values:

```protobuf
message Example {
  // Numeric types
  int32 count = 1;        // Signed 32-bit integer
  int64 big_count = 2;    // Signed 64-bit integer
  uint32 positive = 3;    // Unsigned 32-bit integer
  float ratio = 4;        // 32-bit floating point
  double precise = 5;     // 64-bit floating point

  // Boolean
  bool enabled = 6;

  // String and bytes
  string name = 7;        // UTF-8 string
  bytes data = 8;         // Arbitrary bytes
}
```

### Choosing Numeric Types

| Use Case | Recommended Type |
|----------|-----------------|
| General integers | `int32` or `int64` |
| Always positive | `uint32` or `uint64` |
| Often negative | `sint32` or `sint64` |
| Fixed size needed | `fixed32`, `fixed64`, `sfixed32`, `sfixed64` |
| Decimal numbers | `float` or `double` |

!!! tip "Performance tip"
    `sint32` and `sint64` use ZigZag encoding, which is more efficient for negative numbers.

### Enums

Enums define a set of named constants:

```protobuf
enum PhoneType {
  PHONE_TYPE_UNSPECIFIED = 0;  // Always have a zero value
  PHONE_TYPE_MOBILE = 1;
  PHONE_TYPE_HOME = 2;
  PHONE_TYPE_WORK = 3;
}

message Phone {
  string number = 1;
  PhoneType type = 2;
}
```

!!! warning "Enum requirements"
    - First value must be 0 (used as default)
    - Use `ENUM_NAME_VALUE_NAME` naming convention
    - Values must be unique (or use `allow_alias`)

### Nested Messages

Messages can contain other messages:

```protobuf
message Person {
  string name = 1;
  int32 id = 2;

  // Nested message definition
  message Address {
    string street = 1;
    string city = 2;
    string country = 3;
  }

  Address home_address = 3;
  Address work_address = 4;
}
```

You can also reference messages from other files:

```protobuf
import "other.proto";

message MyMessage {
  other.OtherMessage data = 1;
}
```

## Field Rules

### Singular Fields (Default)

In proto3, fields are singular by default (zero or one value):

```protobuf
message User {
  string name = 1;  // Singular field
}
```

### Optional Fields

Use `optional` to track field presence:

```protobuf
message User {
  optional string nickname = 1;  // Has presence tracking
}
```

This generates `has_nickname()` methods to check if set.

### Repeated Fields

Repeated fields hold a list of values:

```protobuf
message SearchResponse {
  repeated Result results = 1;  // List of results
}
```

In code:

=== "C++"

    ```cpp
    response.add_results();  // Add element
    response.results_size(); // Get count
    response.results(0);     // Access by index
    ```

=== "Java"

    ```java
    response.addResults(result);  // Add element
    response.getResultsCount();   // Get count
    response.getResults(0);       // Access by index
    ```

=== "Python"

    ```python
    response.results.append(result)  # Add element
    len(response.results)            # Get count
    response.results[0]              # Access by index
    ```

### Map Fields

Map fields store key-value pairs:

```protobuf
message Project {
  map<string, int32> permissions = 1;
}
```

Equivalent to:

```protobuf
message MapEntry {
  string key = 1;
  int32 value = 2;
}
repeated MapEntry permissions = 1;
```

!!! note "Map restrictions"
    - Keys must be integral or string types
    - Values can be any type except maps
    - Maps cannot be repeated

### Oneof Fields

Oneof fields share memory; only one can be set:

```protobuf
message Content {
  oneof payload {
    string text = 1;
    bytes binary = 2;
    int32 number = 3;
  }
}
```

Setting one field clears the others:

=== "C++"

    ```cpp
    content.set_text("hello");   // text is set
    content.set_number(42);      // text is cleared, number is set
    content.payload_case();      // Returns kNumber
    ```

=== "Python"

    ```python
    content.text = "hello"       # text is set
    content.number = 42          # text is cleared, number is set
    content.WhichOneof('payload')  # Returns 'number'
    ```

## Field Numbers

Field numbers identify fields in the binary format.

### Number Ranges

| Range | Bytes | Usage |
|-------|-------|-------|
| 1-15 | 1 | Frequently used fields |
| 16-2047 | 2 | Less frequent fields |
| 2048-536870911 | 3-5 | Rarely used fields |

!!! tip "Best practice"
    Reserve 1-15 for fields that are always present or frequently accessed.

### Reserved Numbers

Reserve numbers for deleted fields to prevent reuse:

```protobuf
message User {
  reserved 2, 15, 9 to 11;
  reserved "old_field", "legacy_name";

  string name = 1;
  string email = 3;
}
```

### Forbidden Numbers

- **0** - Invalid field number
- **19000-19999** - Reserved for Protocol Buffers implementation

## Default Values

In proto3, unset fields have default values:

| Type | Default Value |
|------|---------------|
| Numeric types | `0` |
| Booleans | `false` |
| Strings | `""` (empty string) |
| Bytes | `""` (empty bytes) |
| Enums | First defined value (0) |
| Messages | Language-specific (null or empty) |

!!! warning "Default value ambiguity"
    You cannot distinguish between "not set" and "set to default value" for most proto3 fields. Use `optional` if you need this distinction.

## Packages

Packages prevent naming conflicts:

```protobuf
package mycompany.myproject;

message User {
  string name = 1;
}
```

Reference from other files:

```protobuf
import "user.proto";

message Request {
  mycompany.myproject.User user = 1;
}
```

## Options

Options customize code generation:

```protobuf
// File-level options
option java_package = "com.example.proto";
option java_multiple_files = true;
option go_package = "github.com/example/proto";

message User {
  // Field-level option
  string email = 1 [deprecated = true];
}
```

Common options:

| Option | Description |
|--------|-------------|
| `deprecated` | Mark field as deprecated |
| `packed` | Use packed encoding (default for repeated scalars in proto3) |
| `json_name` | Custom JSON field name |

## Best Practices

### Naming Conventions

- **Messages**: PascalCase (`SearchRequest`)
- **Fields**: snake_case (`page_number`)
- **Enums**: SCREAMING_SNAKE_CASE (`PHONE_TYPE_MOBILE`)

### Schema Design

1. **Group related fields** - Use nested messages for clarity
2. **Use meaningful names** - Be descriptive, not cryptic
3. **Reserve field numbers** - Plan for evolution
4. **Document with comments** - Explain non-obvious fields

### Evolution Guidelines

1. **Never change field numbers** - They're part of the wire format
2. **Never reuse field numbers** - Use `reserved`
3. **Add fields with new numbers** - Safe to add
4. **Mark removed fields reserved** - Prevents accidental reuse

## Example: Complete Message

```protobuf
syntax = "proto3";

package ecommerce;

import "google/protobuf/timestamp.proto";

// Represents a customer order
message Order {
  // Reserve removed fields
  reserved 5, 10 to 12;
  reserved "legacy_status";

  // Core identification (use low numbers)
  string order_id = 1;
  string customer_id = 2;

  // Order details
  repeated LineItem items = 3;
  OrderStatus status = 4;

  // Timestamps
  google.protobuf.Timestamp created_at = 6;
  google.protobuf.Timestamp updated_at = 7;

  // Optional fields
  optional string notes = 8;

  // Nested types
  message LineItem {
    string product_id = 1;
    int32 quantity = 2;
    int64 price_cents = 3;
  }

  enum OrderStatus {
    ORDER_STATUS_UNSPECIFIED = 0;
    ORDER_STATUS_PENDING = 1;
    ORDER_STATUS_CONFIRMED = 2;
    ORDER_STATUS_SHIPPED = 3;
    ORDER_STATUS_DELIVERED = 4;
    ORDER_STATUS_CANCELLED = 5;
  }
}
```
