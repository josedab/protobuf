# Generic/Template Messages in Protocol Buffers

This document describes the generic (parameterized) message type feature in Protocol Buffers.

## Overview

Generic messages allow you to define reusable message templates that can be instantiated with different types. This eliminates code duplication for common patterns like result wrappers, optional values, and collections.

## Syntax

### Defining Generic Messages

```protobuf
// Single type parameter
message Container<T> {
  T value = 1;
}

// Multiple type parameters
message Result<T, E> {
  oneof result {
    E error = 1;
    T success = 2;
  }
}

// Default type parameter
message Result<T, E = string> {
  oneof result {
    E error = 1;
    T success = 2;
  }
}

// Type constraint
message MessageWrapper<T: message> {
  T wrapped = 1;
}
```

### Using Generic Messages

```protobuf
message UserDashboard {
  // Instantiate with single type
  Container<User> user = 1;

  // Instantiate with multiple types
  Result<Order, ErrorCode> order_result = 2;

  // Use default for second parameter
  Result<Config> config_result = 3;
}
```

### In RPC Definitions

```protobuf
service UserService {
  rpc GetUser(GetUserRequest) returns (Result<User>);
  rpc ListUsers(ListRequest) returns (Result<Paginated<User>>);
  rpc CreateUser(CreateRequest) returns (Result<User, ValidationError>);
}
```

## Type Parameters

### Names

Type parameter names are typically single uppercase letters (`T`, `E`, `K`, `V`), but any valid identifier can be used:

```protobuf
message Container<ElementType> {
  ElementType element = 1;
}
```

### Constraints

You can constrain type parameters to require certain types:

```protobuf
// Must be a message type (not a primitive)
message Wrapper<T: message> {
  T value = 1;
}

// Must implement a specific interface
message Timestamped<T: HasTimestamp> {
  T value = 1;
}
```

### Default Values

Type parameters can have default values:

```protobuf
message Result<T, E = string> {
  oneof result {
    E error = 1;
    T success = 2;
  }
}

// Usage:
// Result<User> is equivalent to Result<User, string>
```

## Wire Format

Generic messages use **monomorphization** - each instantiation becomes a concrete message type. The wire format is identical to a handwritten message.

```protobuf
// This generic definition...
message Result<T> {
  oneof result {
    string error = 1;
    T success = 2;
  }
}

// ...when instantiated as Result<User>, becomes equivalent to:
message Result_User {
  oneof result {
    string error = 1;
    User success = 2;
  }
}
```

This means:
- No runtime overhead
- Full backward compatibility
- Existing clients see concrete types

## Generated Code

### C++

```cpp
// Template class is generated
template <typename T>
class Result : public Message {
 public:
  const std::string& error() const;
  void set_error(std::string value);

  const T& success() const;
  T* mutable_success();
  void set_allocated_success(T* value);
};

// Usage
Result<User> result;
result.mutable_success()->set_name("Alice");
```

### Java

```java
public class Result<T extends Message> extends GeneratedMessage {
  public String getError() { ... }
  public T getSuccess() { ... }

  public static class Builder<T extends Message>
      extends GeneratedMessage.Builder<Builder<T>> {
    public Builder<T> setError(String value) { ... }
    public Builder<T> setSuccess(T value) { ... }
    public Result<T> build() { ... }
  }
}

// Usage
Result<User> result = Result.<User>newBuilder()
    .setSuccess(user)
    .build();
```

### Python

```python
from typing import TypeVar, Generic

T = TypeVar('T', bound=Message)

class Result(Generic[T], Message):
    @property
    def error(self) -> str: ...

    @property
    def success(self) -> T: ...

# Usage
result: Result[User] = Result()
result.success = user
```

## Standard Library

Protocol Buffers provides a standard library of common generic types in `google/protobuf/generics.proto`:

| Type | Description | Example Usage |
|------|-------------|---------------|
| `Result<T, E>` | Success/error result | `Result<User>` |
| `Optional<T>` | Nullable value | `Optional<Config>` |
| `List<T>` | Collection | `List<Order>` |
| `Paginated<T>` | Paginated collection | `Paginated<User>` |
| `Either<L, R>` | Union of two types | `Either<string, int32>` |
| `Pair<K, V>` | Key-value pair | `Pair<string, User>` |

Import and use:

```protobuf
import "google/protobuf/generics.proto";

service UserService {
  rpc GetUser(Request) returns (google.protobuf.Result<User>);
}
```

## JSON Mapping

Generic messages map to JSON like any other message:

```json
{
  "success": {
    "name": "Alice",
    "email": "alice@example.com"
  }
}
```

With type discriminator (optional):

```json
{
  "@type": "Result<User>",
  "success": {
    "name": "Alice",
    "email": "alice@example.com"
  }
}
```

## Best Practices

### Do

- Use generics for commonly repeated patterns
- Provide default types when one type is commonly used
- Use meaningful type parameter names in complex cases
- Import the standard library instead of redefining common types

### Don't

- Overuse generics for simple cases
- Create deeply nested generics (more than 2-3 levels)
- Use generics where `Any` or `oneof` would be more appropriate

## Comparison with Alternatives

### vs. `google.protobuf.Any`

| Feature | Generics | Any |
|---------|----------|-----|
| Type safety | Compile-time | Runtime |
| Performance | No overhead | Type URL overhead |
| Reflection | Full | Limited |

Use generics when type safety is important. Use `Any` when types are truly dynamic.

### vs. Code Generation Scripts

| Feature | Generics | Scripts |
|---------|----------|---------|
| Standardization | Built-in | Custom |
| IDE support | Full | Varies |
| Maintenance | Low | High |

Prefer generics for their standardization and tooling support.

## Migration Guide

### From Duplicated Messages

Before:
```protobuf
message UserResult {
  oneof result {
    string error = 1;
    User success = 2;
  }
}

message OrderResult {
  oneof result {
    string error = 1;
    Order success = 2;
  }
}
```

After:
```protobuf
message Result<T> {
  oneof result {
    string error = 1;
    T success = 2;
  }
}

// Use as Result<User> and Result<Order>
```

### From `Any` Type

Before:
```protobuf
import "google/protobuf/any.proto";

message Container {
  google.protobuf.Any value = 1;
}
```

After:
```protobuf
message Container<T> {
  T value = 1;
}

// Use as Container<User> for type-safe access
```

## Limitations

1. **Nested depth**: Default limit of 3 levels for nested generics
2. **Cross-file**: Generics defined in one file can be used in others with proper imports
3. **Reflection**: Type parameters are available in descriptor metadata

## See Also

- [Language Guide](language-guide.md)
- [Style Guide](style-guide.md)
- [API Reference](api-reference.md)
