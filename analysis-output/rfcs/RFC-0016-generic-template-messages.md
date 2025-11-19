# RFC-0016: Generic/Template Messages

**Status:** Draft
**Author:** Codebase Analysis
**Created:** November 19, 2025
**Effort:** 10 weeks
**Category:** Long-term

---

## Summary

Introduce parameterized (generic) message types in Protocol Buffers, enabling type-safe reusable containers like `Result<T>`, `List<T>`, and `Paginated<T>` without code duplication or loss of type safety.

## Motivation

### Problem Statement

Common patterns require message duplication:

```protobuf
// Every result type needs separate definition
message UserResult {
  oneof result {
    string error = 1;
    User success = 2;
  }
}

message ConfigResult {
  oneof result {
    string error = 1;
    Config success = 2;
  }
}

message OrderResult {
  oneof result {
    string error = 1;
    Order success = 2;
  }
}

// Same for lists
message UserList {
  repeated User items = 1;
}

message OrderList {
  repeated Order items = 1;
}
```

### Issues

- **Code duplication** - Same pattern repeated
- **Maintenance burden** - Change in one place missed elsewhere
- **Inconsistency** - Slight variations between implementations
- **No type safety** - `Any` type loses compile-time checks

### Prevalence

This pattern appears constantly in:
- RPC response wrappers
- Pagination containers
- Optional/nullable wrappers
- Collection types
- Error handling

## Detailed Design

### Generic Syntax

```protobuf
// Define generic message
message Result<T> {
  oneof result {
    string error = 1;
    T success = 2;
  }
}

message List<T> {
  repeated T items = 1;
}

message Paginated<T> {
  repeated T items = 1;
  int32 total_count = 2;
  string next_page_token = 3;
}

message Optional<T> {
  oneof value {
    bool is_null = 1;
    T value = 2;
  }
}
```

### Type Parameters

#### Single Parameter
```protobuf
message Container<T> {
  T value = 1;
}
```

#### Multiple Parameters
```protobuf
message Result<T, E> {
  oneof result {
    E error = 1;
    T success = 2;
  }
}

message Map<K, V> {
  map<K, V> entries = 1;
}
```

#### Default Parameters
```protobuf
message Result<T, E = string> {
  oneof result {
    E error = 1;
    T success = 2;
  }
}

// Usage: Result<User> uses E = string
// Usage: Result<User, ErrorCode> overrides E
```

### Instantiation

```protobuf
service UserService {
  // Instantiate generics
  rpc GetUser(GetUserRequest) returns (Result<User>);
  rpc ListUsers(ListRequest) returns (Paginated<User>);
  rpc UpdateUser(User) returns (Result<User, ErrorCode>);
}

message Dashboard {
  // Use in fields
  Result<User> current_user = 1;
  Paginated<Order> recent_orders = 2;
  Optional<Config> custom_config = 3;
}
```

### Constraints

#### Type Bounds
```protobuf
// Constrain T to be a message (not scalar)
message Container<T: message> {
  T value = 1;
}

// Constrain to specific interface
message Timestamped<T: HasTimestamp> {
  T value = 1;
  google.protobuf.Timestamp timestamp = 2;
}
```

### Wire Format

**Monomorphization approach** - Each instantiation becomes a concrete message:

```protobuf
// Source
message Result<T> {
  oneof result {
    string error = 1;
    T success = 2;
  }
}

service UserService {
  rpc GetUser(Request) returns (Result<User>);
}

// Compiled (conceptually)
message Result_User {
  oneof result {
    string error = 1;
    User success = 2;
  }
}
```

Wire format is identical to handwritten messages - no runtime overhead.

### Generated Code

#### C++

```cpp
// Template class generated
template <typename T>
class Result : public Message {
 public:
  enum ResultCase {
    kError = 1,
    kSuccess = 2,
    RESULT_NOT_SET = 0,
  };

  // Error field
  const std::string& error() const;
  void set_error(std::string value);

  // Generic success field
  const T& success() const;
  T* mutable_success();
  void set_allocated_success(T* value);

  ResultCase result_case() const;
};

// Usage
Result<User> result;
result.mutable_success()->set_name("Alice");
```

#### Java

```java
// Generic class with type parameter
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

#### Python

```python
# Generic class using typing
from typing import TypeVar, Generic

T = TypeVar('T', bound=Message)

class Result(Generic[T], Message):
    def __init__(self):
        self._error: str = ""
        self._success: Optional[T] = None

    @property
    def error(self) -> str:
        return self._error

    @property
    def success(self) -> T:
        return self._success

# Usage
result: Result[User] = Result()
result.success = user
```

### JSON Mapping

```json
{
  "result": {
    "success": {
      "name": "Alice",
      "email": "alice@example.com"
    }
  }
}
```

Or with discriminator:

```json
{
  "@type": "Result<User>",
  "success": {
    "name": "Alice",
    "email": "alice@example.com"
  }
}
```

## Example Usage

### RPC Response Wrappers

```protobuf
// Define once
message Result<T, E = string> {
  oneof result {
    E error = 1;
    T success = 2;
  }
}

message Paginated<T> {
  repeated T items = 1;
  int32 total = 2;
  string next_token = 3;
}

// Use everywhere
service UserService {
  rpc GetUser(GetUserRequest) returns (Result<User>);
  rpc ListUsers(ListUsersRequest) returns (Result<Paginated<User>>);
  rpc CreateUser(CreateUserRequest) returns (Result<User, CreateError>);
  rpc DeleteUser(DeleteUserRequest) returns (Result<Empty>);
}

service OrderService {
  rpc GetOrder(GetOrderRequest) returns (Result<Order>);
  rpc ListOrders(ListOrdersRequest) returns (Result<Paginated<Order>>);
}
```

### Nested Generics

```protobuf
// Paginated results with errors
rpc ListUsers(Request) returns (Result<Paginated<User>>);

// Map of lists
message UserGroups {
  map<string, List<User>> groups = 1;
}
```

### Standard Library

Provide common generics:

```protobuf
// google/protobuf/generics.proto

message Result<T, E = string> { ... }
message Optional<T> { ... }
message List<T> { ... }
message Paginated<T> { ... }
message Either<L, R> { ... }
```

## Implementation Plan

### Phase 1: Parser and Descriptor (Weeks 1-3)
- [ ] Extend parser for generic syntax
- [ ] Add type parameters to descriptors
- [ ] Implement monomorphization

### Phase 2: C++ Code Generation (Weeks 4-5)
- [ ] Template class generation
- [ ] Instantiation at use sites
- [ ] Serialization support

### Phase 3: Java/Python Generation (Weeks 6-7)
- [ ] Java generic classes
- [ ] Python generic support
- [ ] Type parameter handling

### Phase 4: Advanced Features (Weeks 8-9)
- [ ] Type constraints
- [ ] Default parameters
- [ ] Nested generics

### Phase 5: Testing and Documentation (Week 10)
- [ ] Comprehensive tests
- [ ] Standard library
- [ ] Documentation

## Backwards Compatibility

### Fully Compatible
- New feature, opt-in
- Existing protos unchanged
- Wire format identical to handwritten

### Interoperability
- Generic and non-generic messages interoperable
- Old clients see concrete types

## Alternatives Considered

### Alternative 1: Any Type
- **Pro:** Already exists
- **Con:** Loses type safety, runtime overhead
- **Decision:** Generics preserve type safety

### Alternative 2: Code Generation Scripts
- **Pro:** Works today
- **Con:** Fragile, non-standard
- **Decision:** First-class support better

### Alternative 3: Runtime Generics
- **Pro:** Smaller generated code
- **Con:** Performance overhead, complexity
- **Decision:** Compile-time monomorphization

## Open Questions

1. **Recursion limit** - Allow `List<List<List<T>>>`?
   - Suggestion: Configurable depth limit

2. **Cross-file generics** - Define in one file, use in another?
   - Suggestion: Yes, with proper imports

3. **Code size** - Many instantiations = large binary?
   - Suggestion: Deduplication of identical instantiations

4. **Reflection** - How to reflect on generic types?
   - Suggestion: Expose type parameters in descriptor

## Success Criteria

- [ ] Generic syntax parsed and validated
- [ ] Monomorphization working
- [ ] C++, Java, Python code generation
- [ ] Standard library of common generics
- [ ] Documentation and examples

## Effort Estimation

| Task | Days |
|------|------|
| Parser extensions | 8 |
| Descriptor model | 5 |
| Monomorphization | 8 |
| C++ generator | 10 |
| Java generator | 8 |
| Python generator | 6 |
| Testing | 8 |
| Documentation | 5 |
| **Total** | **58** (10 weeks) |

---

## References

- [C++ Templates](https://en.cppreference.com/w/cpp/language/templates)
- [Java Generics](https://docs.oracle.com/javase/tutorial/java/generics/)
- [Rust Generics](https://doc.rust-lang.org/book/ch10-00-generics.html)
- [TypeScript Generics](https://www.typescriptlang.org/docs/handbook/2/generics.html)
