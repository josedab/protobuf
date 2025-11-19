# RFC-0009: Built-in Validation Constraints

**Status:** Draft
**Author:** Codebase Analysis
**Created:** November 19, 2025
**Effort:** 8 weeks
**Category:** Long-term

---

## Summary

Introduce native schema validation constraints directly in Protocol Buffers syntax, generating `Validate()` methods in all language runtimes to ensure data validity at the schema level rather than requiring separate validation logic in each service.

## Motivation

### Problem Statement

Protobuf schemas describe structure but not validity constraints:

1. **Duplicated validation** - Every service consuming a proto implements validation separately
2. **Inconsistency** - Different services validate the same message differently
3. **Runtime errors** - Invalid data discovered only at runtime
4. **External dependencies** - protoc-gen-validate shows demand but isn't standardized

### Evidence of Need

- protoc-gen-validate has 3,000+ GitHub stars
- Common validation patterns repeated across codebases:
  - Email format
  - String length
  - Numeric ranges
  - Required fields
  - Regex patterns

### Business Impact

- Bugs from inconsistent validation
- Security vulnerabilities from missing validation
- Development time wasted on repetitive code
- No single source of truth for data validity

## Detailed Design

### Syntax Extension

Add validation annotations to proto syntax:

```protobuf
import "google/protobuf/validate.proto";

message User {
  string email = 1 [
    (validate.string).email = true,
    (validate.string).min_len = 5,
    (validate.string).max_len = 255
  ];

  int32 age = 2 [
    (validate.int32).gte = 0,
    (validate.int32).lte = 150
  ];

  string username = 3 [
    (validate.string).pattern = "^[a-z0-9_]{3,20}$"
  ];

  repeated string tags = 4 [
    (validate.repeated).min_items = 1,
    (validate.repeated).max_items = 10,
    (validate.repeated).unique = true
  ];

  string phone = 5 [
    (validate.string).pattern = "^\\+?[1-9]\\d{1,14}$",
    (validate.string).ignore_empty = true
  ];
}
```

### Validation Types

#### String Validations
```protobuf
message StringRules {
  uint64 min_len = 1;           // Minimum length
  uint64 max_len = 2;           // Maximum length
  string pattern = 3;           // Regex pattern
  bool email = 4;               // Email format
  bool uri = 5;                 // URI format
  bool uuid = 6;                // UUID format
  bool ip = 7;                  // IP address
  bool hostname = 8;            // Hostname
  repeated string in = 9;       // Must be one of
  repeated string not_in = 10;  // Must not be one of
  bool ignore_empty = 11;       // Skip if empty
}
```

#### Numeric Validations
```protobuf
message Int32Rules {
  int32 const = 1;              // Must equal
  int32 lt = 2;                 // Less than
  int32 lte = 3;                // Less than or equal
  int32 gt = 4;                 // Greater than
  int32 gte = 5;                // Greater than or equal
  repeated int32 in = 6;        // Must be one of
  repeated int32 not_in = 7;    // Must not be one of
}
// Similar for int64, uint32, uint64, float, double
```

#### Message Validations
```protobuf
message MessageRules {
  bool required = 1;            // Must be set
  bool skip = 2;                // Don't validate nested
}
```

#### Repeated Field Validations
```protobuf
message RepeatedRules {
  uint64 min_items = 1;         // Minimum count
  uint64 max_items = 2;         // Maximum count
  bool unique = 3;              // All items unique
  FieldRules items = 4;         // Rules for each item
}
```

#### Map Validations
```protobuf
message MapRules {
  uint64 min_pairs = 1;         // Minimum entries
  uint64 max_pairs = 2;         // Maximum entries
  FieldRules keys = 3;          // Rules for keys
  FieldRules values = 4;        // Rules for values
}
```

### Generated Code

#### C++
```cpp
// Generated validation method
class User : public Message {
 public:
  absl::Status Validate() const {
    // Email validation
    if (!email_.empty()) {
      if (email_.size() < 5) {
        return absl::InvalidArgumentError(
            "field 'email' length must be >= 5");
      }
      if (email_.size() > 255) {
        return absl::InvalidArgumentError(
            "field 'email' length must be <= 255");
      }
      if (!IsValidEmail(email_)) {
        return absl::InvalidArgumentError(
            "field 'email' must be valid email format");
      }
    }

    // Age validation
    if (age_ < 0 || age_ > 150) {
      return absl::InvalidArgumentError(
          "field 'age' must be between 0 and 150");
    }

    // Username pattern validation
    static const std::regex pattern("^[a-z0-9_]{3,20}$");
    if (!std::regex_match(username_, pattern)) {
      return absl::InvalidArgumentError(
          "field 'username' must match pattern '^[a-z0-9_]{3,20}$'");
    }

    // Tags validation
    if (tags_.size() < 1) {
      return absl::InvalidArgumentError(
          "field 'tags' must have at least 1 item");
    }
    if (tags_.size() > 10) {
      return absl::InvalidArgumentError(
          "field 'tags' must have at most 10 items");
    }

    return absl::OkStatus();
  }
};
```

#### Java
```java
public final class User extends GeneratedMessage {
  public ValidationResult validate() {
    ValidationResult.Builder result = ValidationResult.newBuilder();

    // Email validation
    if (!email_.isEmpty()) {
      if (email_.length() < 5) {
        result.addError(ValidationError.newBuilder()
            .setField("email")
            .setMessage("length must be >= 5")
            .build());
      }
      if (!EmailValidator.isValid(email_)) {
        result.addError(ValidationError.newBuilder()
            .setField("email")
            .setMessage("must be valid email format")
            .build());
      }
    }

    // Age validation
    if (age_ < 0 || age_ > 150) {
      result.addError(ValidationError.newBuilder()
          .setField("age")
          .setMessage("must be between 0 and 150")
          .build());
    }

    return result.build();
  }

  public void validateOrThrow() throws ValidationException {
    ValidationResult result = validate();
    if (!result.isValid()) {
      throw new ValidationException(result);
    }
  }
}
```

#### Python
```python
class User(Message):
    def validate(self) -> ValidationResult:
        errors = []

        # Email validation
        if self.email:
            if len(self.email) < 5:
                errors.append(ValidationError(
                    field="email",
                    message="length must be >= 5"
                ))
            if not is_valid_email(self.email):
                errors.append(ValidationError(
                    field="email",
                    message="must be valid email format"
                ))

        # Age validation
        if not (0 <= self.age <= 150):
            errors.append(ValidationError(
                field="age",
                message="must be between 0 and 150"
            ))

        return ValidationResult(errors=errors)

    def validate_or_raise(self):
        result = self.validate()
        if not result.is_valid:
            raise ValidationException(result)
```

### Cross-Field Validation

Support validation across multiple fields:

```protobuf
message DateRange {
  google.protobuf.Timestamp start = 1;
  google.protobuf.Timestamp end = 2;

  option (validate.message) = {
    cel: "this.end > this.start"
    error: "end must be after start"
  };
}

message Password {
  string password = 1;
  string confirm = 2;

  option (validate.message) = {
    cel: "this.password == this.confirm"
    error: "passwords must match"
  };
}
```

### CEL Integration

Use Common Expression Language for complex validation:

```protobuf
message Order {
  repeated LineItem items = 1;
  int32 total = 2;

  option (validate.message) = {
    cel: "this.items.map(i, i.price * i.quantity).sum() == this.total"
    error: "total must equal sum of line items"
  };
}
```

## Example Usage

### Basic Validation

```cpp
User user;
user.set_email("invalid");
user.set_age(-5);
user.set_username("ab");  // Too short

absl::Status status = user.Validate();
if (!status.ok()) {
  LOG(ERROR) << "Validation failed: " << status.message();
  // Output: "field 'email' must be valid email format;
  //          field 'age' must be >= 0;
  //          field 'username' must match pattern..."
}
```

### RPC Integration

```cpp
grpc::Status UserService::CreateUser(
    ServerContext* context,
    const CreateUserRequest* request,
    CreateUserResponse* response) {

  // Validate request
  if (auto status = request->Validate(); !status.ok()) {
    return grpc::Status(grpc::INVALID_ARGUMENT, status.message());
  }

  // Process valid request
  return CreateUserInternal(request, response);
}
```

### Nested Message Validation

```protobuf
message Order {
  User customer = 1 [(validate.message).required = true];
  repeated LineItem items = 2 [
    (validate.repeated).min_items = 1
  ];
  Address shipping = 3;
}
```

```cpp
// Automatically validates nested messages
Order order;
absl::Status status = order.Validate();
// Validates: customer, customer's fields, each item, shipping
```

## Implementation Plan

### Phase 1: Core Infrastructure (Weeks 1-2)
- [ ] Define validate.proto with all rule types
- [ ] Implement validation rule parsing in protoc
- [ ] Create validation code generation framework

### Phase 2: C++ Implementation (Weeks 3-4)
- [ ] Generate C++ validation methods
- [ ] Implement string validators (email, uri, pattern)
- [ ] Implement numeric validators
- [ ] Add nested message validation

### Phase 3: Java/Python Implementation (Weeks 5-6)
- [ ] Generate Java validation methods
- [ ] Generate Python validation methods
- [ ] Ensure consistent error messages

### Phase 4: Advanced Features (Weeks 7-8)
- [ ] CEL integration for cross-field validation
- [ ] Custom validation hooks
- [ ] Performance optimization
- [ ] Documentation and examples

## Backwards Compatibility

### Fully Compatible
- Validation is opt-in via annotations
- Old code ignores validation options
- Generated code without validation still works
- Wire format unchanged

### Migration from protoc-gen-validate
- Provide conversion tool
- Similar syntax for easy migration
- Document differences

## Alternatives Considered

### Alternative 1: Keep protoc-gen-validate
- **Pro:** Already exists
- **Con:** External dependency, not standardized
- **Decision:** Standardize in core for consistency

### Alternative 2: JSON Schema Style Validation
- **Pro:** Familiar to web developers
- **Con:** Separate file, drift risk
- **Decision:** Keep validation with schema

### Alternative 3: Runtime-Only Validation
- **Pro:** No code generation changes
- **Con:** No schema benefits, larger runtime
- **Decision:** Schema-level provides more value

## Open Questions

1. **Cross-field validation** - How complex should CEL expressions be?
   - Suggestion: Limit to reasonable complexity, provide examples

2. **Custom validators** - Allow user-defined validation functions?
   - Suggestion: Yes, via hooks with clear interface

3. **Performance** - Regex compilation overhead?
   - Suggestion: Compile once, cache statically

4. **Error aggregation** - Return first error or all errors?
   - Suggestion: Option for both (fail-fast vs. collect-all)

## Success Criteria

- [ ] All validation types implemented in C++, Java, Python
- [ ] Zero runtime overhead if Validate() not called
- [ ] Clear error messages with field paths
- [ ] Migration tool for protoc-gen-validate
- [ ] Comprehensive documentation

## Effort Estimation

| Task | Days |
|------|------|
| validate.proto design | 3 |
| Protoc parser changes | 5 |
| C++ generator | 10 |
| Java generator | 8 |
| Python generator | 6 |
| CEL integration | 5 |
| Testing | 8 |
| Documentation | 5 |
| **Total** | **50** (8 weeks) |

---

## References

- [protoc-gen-validate](https://github.com/bufbuild/protoc-gen-validate)
- [Common Expression Language](https://github.com/google/cel-spec)
- [JSON Schema Validation](https://json-schema.org/draft/2020-12/json-schema-validation.html)
