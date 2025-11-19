# Protocol Buffer Validation

This document describes the built-in validation constraints feature for Protocol Buffers, which allows you to define validation rules directly in your `.proto` files.

## Overview

Protocol Buffer validation constraints allow you to specify rules for valid data directly in your schema. These constraints generate `Validate()` methods in all target languages, ensuring consistent validation across your entire system.

### Key Benefits

- **Single source of truth** - Validation rules are defined once in the schema
- **Consistent validation** - All services validate the same way
- **Generated code** - No manual validation code to write
- **Cross-language support** - Works in C++, Java, Python, and more

## Quick Start

### 1. Import the validation proto

```protobuf
import "google/protobuf/validate.proto";
```

### 2. Add validation rules to your fields

```protobuf
message User {
  string email = 1 [
    (google.protobuf.validate) = {
      string: {
        email: true
        max_len: 255
      }
    }
  ];

  int32 age = 2 [
    (google.protobuf.validate) = {
      int32: {
        gte: 0
        lte: 150
      }
    }
  ];
}
```

### 3. Use generated validation methods

**C++:**
```cpp
User user;
user.set_email("invalid");
user.set_age(-5);

absl::Status status = user.Validate();
if (!status.ok()) {
  std::cerr << "Validation failed: " << status.message() << std::endl;
}
```

**Java:**
```java
User user = User.newBuilder()
    .setEmail("invalid")
    .setAge(-5)
    .build();

ValidationResult result = user.validate();
if (!result.getIsValid()) {
  for (ValidationError error : result.getErrorsList()) {
    System.err.println(error.getField() + ": " + error.getMessage());
  }
}
```

**Python:**
```python
user = User()
user.email = "invalid"
user.age = -5

result = user.validate()
if not result.is_valid:
    for error in result.errors:
        print(f"{error.field}: {error.message}")
```

## Validation Rules

### String Rules

```protobuf
message StringRules {
  uint64 min_len = 1;           // Minimum length (Unicode code points)
  uint64 max_len = 2;           // Maximum length
  uint64 min_bytes = 3;         // Minimum bytes
  uint64 max_bytes = 4;         // Maximum bytes
  string pattern = 5;           // Regex pattern
  string prefix = 6;            // Must have prefix
  string suffix = 7;            // Must have suffix
  string contains = 8;          // Must contain substring
  string not_contains = 9;      // Must not contain substring
  repeated string in = 10;      // Must be one of
  repeated string not_in = 11;  // Must not be one of

  // Format validations
  bool email = 12;              // Valid email format
  bool hostname = 13;           // Valid hostname (RFC 1123)
  bool ip = 14;                 // Valid IP address (v4 or v6)
  bool ipv4 = 15;               // Valid IPv4 address
  bool ipv6 = 16;               // Valid IPv6 address
  bool uri = 17;                // Valid URI (RFC 3986)
  bool uri_ref = 18;            // Valid URI reference
  bool uuid = 19;               // Valid UUID (RFC 4122)

  bool ignore_empty = 20;       // Skip if empty
}
```

**Example:**
```protobuf
message Contact {
  string email = 1 [
    (google.protobuf.validate) = {
      string: {
        email: true
        min_len: 5
        max_len: 255
      }
    }
  ];

  string username = 2 [
    (google.protobuf.validate) = {
      string: {
        pattern: "^[a-z0-9_]{3,20}$"
      }
    }
  ];

  string phone = 3 [
    (google.protobuf.validate) = {
      string: {
        pattern: "^\\+?[1-9]\\d{1,14}$"
        ignore_empty: true
      }
    }
  ];
}
```

### Numeric Rules

All numeric types (int32, int64, uint32, uint64, sint32, sint64, fixed32, fixed64, sfixed32, sfixed64, float, double) support these rules:

```protobuf
message Int32Rules {
  int32 const = 1;              // Must equal this value
  int32 lt = 2;                 // Less than
  int32 lte = 3;                // Less than or equal
  int32 gt = 4;                 // Greater than
  int32 gte = 5;                // Greater than or equal
  repeated int32 in = 6;        // Must be one of
  repeated int32 not_in = 7;    // Must not be one of
  bool ignore_empty = 8;        // Skip if default value
}
```

**Example:**
```protobuf
message Product {
  int32 quantity = 1 [
    (google.protobuf.validate) = {
      int32: {
        gte: 0
        lte: 1000
      }
    }
  ];

  double price = 2 [
    (google.protobuf.validate) = {
      double: {
        gt: 0.0
      }
    }
  ];

  int32 priority = 3 [
    (google.protobuf.validate) = {
      int32: {
        in: [1, 2, 3, 4, 5]
      }
    }
  ];
}
```

### Message Rules

```protobuf
message MessageRules {
  bool required = 1;            // Must be set (not null)
  bool skip = 2;                // Don't validate nested fields
}
```

**Example:**
```protobuf
message Order {
  User customer = 1 [
    (google.protobuf.validate) = {
      message: {
        required: true
      }
    }
  ];

  // This nested message won't be validated
  Metadata metadata = 2 [
    (google.protobuf.validate) = {
      message: {
        skip: true
      }
    }
  ];
}
```

### Repeated Field Rules

```protobuf
message RepeatedRules {
  uint64 min_items = 1;         // Minimum count
  uint64 max_items = 2;         // Maximum count
  bool unique = 3;              // All items must be unique
  FieldRules items = 4;         // Rules for each item
  bool ignore_empty = 5;        // Skip if empty
}
```

**Example:**
```protobuf
message Article {
  repeated string tags = 1 [
    (google.protobuf.validate) = {
      repeated: {
        min_items: 1
        max_items: 10
        unique: true
        items: {
          string: {
            min_len: 1
            max_len: 50
          }
        }
      }
    }
  ];

  repeated string emails = 2 [
    (google.protobuf.validate) = {
      repeated: {
        max_items: 5
        items: {
          string: {
            email: true
          }
        }
      }
    }
  ];
}
```

### Map Field Rules

```protobuf
message MapRules {
  uint64 min_pairs = 1;         // Minimum entries
  uint64 max_pairs = 2;         // Maximum entries
  bool no_sparse = 3;           // No null values
  FieldRules keys = 4;          // Rules for keys
  FieldRules values = 5;        // Rules for values
  bool ignore_empty = 6;        // Skip if empty
}
```

**Example:**
```protobuf
message Config {
  map<string, string> settings = 1 [
    (google.protobuf.validate) = {
      map: {
        min_pairs: 1
        max_pairs: 100
        keys: {
          string: {
            min_len: 1
            max_len: 50
            pattern: "^[a-z_]+$"
          }
        }
        values: {
          string: {
            max_len: 1000
          }
        }
      }
    }
  ];
}
```

### Enum Rules

```protobuf
message EnumRules {
  int32 const = 1;              // Must equal this value
  bool defined_only = 2;        // Must be a defined enum value
  repeated int32 in = 3;        // Must be one of these values
  repeated int32 not_in = 4;    // Must not be one of these values
}
```

**Example:**
```protobuf
enum Status {
  UNKNOWN = 0;
  ACTIVE = 1;
  INACTIVE = 2;
}

message Entity {
  Status status = 1 [
    (google.protobuf.validate) = {
      enum: {
        defined_only: true
        not_in: [0]  // UNKNOWN not allowed
      }
    }
  ];
}
```

### Bytes Rules

```protobuf
message BytesRules {
  bytes const = 1;              // Must equal this value
  uint64 min_len = 2;           // Minimum length
  uint64 max_len = 3;           // Maximum length
  string pattern = 4;           // Regex pattern
  bytes prefix = 5;             // Must have prefix
  bytes suffix = 6;             // Must have suffix
  bytes contains = 7;           // Must contain sequence
  repeated bytes in = 8;        // Must be one of
  repeated bytes not_in = 9;    // Must not be one of

  // Format validations
  bool ip = 10;                 // Valid IP address bytes
  bool ipv4 = 11;               // Valid IPv4 bytes (4 bytes)
  bool ipv6 = 12;               // Valid IPv6 bytes (16 bytes)

  bool ignore_empty = 13;       // Skip if empty
}
```

### Bool Rules

```protobuf
message BoolRules {
  bool const = 1;               // Must equal this value
}
```

**Example:**
```protobuf
message Agreement {
  bool accepted_terms = 1 [
    (google.protobuf.validate) = {
      bool: {
        const: true
      }
    }
  ];
}
```

## RPC Integration

### gRPC Service Example

```cpp
grpc::Status UserService::CreateUser(
    ServerContext* context,
    const CreateUserRequest* request,
    CreateUserResponse* response) {

  // Validate request
  if (auto status = request->Validate(); !status.ok()) {
    return grpc::Status(grpc::INVALID_ARGUMENT,
                        std::string(status.message()));
  }

  // Process valid request
  return CreateUserInternal(request, response);
}
```

### Java Spring Example

```java
@RestController
public class UserController {
    @PostMapping("/users")
    public ResponseEntity<?> createUser(@RequestBody User user) {
        try {
            user.validateOrThrow();
            // Process valid user
            return ResponseEntity.ok(userService.create(user));
        } catch (ValidationException e) {
            return ResponseEntity.badRequest()
                .body(e.getErrors());
        }
    }
}
```

## File-Level Options

You can set validation options at the file level:

```protobuf
import "google/protobuf/validate.proto";

option (google.protobuf.disable_validation) = false;  // Enable (default)
option (google.protobuf.fail_fast) = true;  // Stop on first error
```

## Error Handling

### Fail-Fast Mode

By default, `Validate()` stops on the first error (fail-fast):

```cpp
absl::Status status = message.Validate();
// Returns first error encountered
```

### Collect All Errors

Use `ValidateAll()` to collect all errors:

```cpp
ValidationResult result = message.ValidateAll();
for (const auto& error : result.errors()) {
  std::cerr << error.field() << ": " << error.message() << std::endl;
}
```

## Best Practices

1. **Validate at service boundaries** - Always validate incoming requests
2. **Use specific formats** - Prefer `email: true` over generic patterns
3. **Set reasonable limits** - Always set max_len/max_items to prevent DoS
4. **Document constraints** - The schema serves as documentation
5. **Test edge cases** - Ensure validation handles boundary conditions

## Migration from protoc-gen-validate

If you're migrating from protoc-gen-validate, the syntax is similar:

**Before (protoc-gen-validate):**
```protobuf
import "validate/validate.proto";

string email = 1 [(validate.rules).string.email = true];
```

**After (built-in validation):**
```protobuf
import "google/protobuf/validate.proto";

string email = 1 [
  (google.protobuf.validate) = {
    string: {
      email: true
    }
  }
];
```

## Performance Considerations

- Validation code is generated at compile time
- Regex patterns are compiled once and cached
- Zero overhead if `Validate()` is not called
- Inline validation for simple checks

## Troubleshooting

### Common Issues

1. **"field is required" error** - Ensure required message fields are set
2. **Pattern doesn't match** - Check regex syntax; use raw strings
3. **Nested validation fails** - Check nested message validation rules

### Debugging

Enable verbose validation output:

```cpp
#define PROTOBUF_VALIDATION_DEBUG 1
```

## API Reference

### C++ API

```cpp
// Fail-fast validation
absl::Status Validate() const;

// Collect all errors
ValidationResult ValidateAll() const;
```

### Java API

```java
// Returns ValidationResult
ValidationResult validate();

// Throws ValidationException if invalid
void validateOrThrow() throws ValidationException;
```

### Python API

```python
# Returns ValidationResult
def validate(self) -> ValidationResult

# Raises ValidationException if invalid
def validate_or_raise(self)
```

## Extension Numbers

The validation extension uses the following extension numbers:

- `50000` - FieldOptions validate extension
- `50001` - MessageOptions message_validate extension
- `50002` - FileOptions disable_validation extension
- `50003` - FileOptions fail_fast extension

## See Also

- [Protocol Buffers Language Guide](https://developers.google.com/protocol-buffers/docs/proto3)
- [Custom Options](https://developers.google.com/protocol-buffers/docs/proto#customoptions)
- [Extension Registry](options.md)
