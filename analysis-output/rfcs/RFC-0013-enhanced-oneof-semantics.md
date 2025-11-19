# RFC-0013: Enhanced Oneof Semantics

**Status:** Draft
**Author:** Codebase Analysis
**Created:** November 19, 2025
**Effort:** 5 weeks
**Category:** Long-term

---

## Summary

Extend Protocol Buffers oneof fields to support repeated fields within oneofs, nested oneofs, and explicit unset tracking, enabling cleaner expression of sum types and reducing the need for wrapper message workarounds.

## Motivation

### Problem Statement

Current oneof limitations force awkward workarounds:

1. **No repeated in oneof** - Common pattern requires wrapper
2. **No nested oneofs** - Can't express hierarchical choices
3. **Zero-value ambiguity** - Can't distinguish unset from zero
4. **No "at least one" constraint** - Can't enforce selection

### Common Workaround

```protobuf
// Want: "either a list of IDs OR a filter query"
// Can't do:
message Request {
  oneof selection {
    repeated int64 ids = 1;  // ERROR: can't repeat in oneof
    string filter = 2;
  }
}

// Must create wrapper:
message Request {
  oneof selection {
    IdList ids = 1;        // Extra message
    string filter = 2;
  }
}
message IdList {
  repeated int64 values = 1;
}
```

### Impact

- Code verbosity
- Additional allocations
- Unclear intent
- Maintenance burden

## Detailed Design

### 1. Repeated Fields in Oneof

Allow repeated fields as oneof members:

```protobuf
message Request {
  oneof selection {
    repeated int64 ids = 1;    // Now allowed!
    string filter_query = 2;
  }
}
```

#### Wire Format

Unchanged - repeated fields are already length-delimited. The oneof case tracks which field is set.

#### Generated Code (C++)

```cpp
class Request : public Message {
 public:
  enum SelectionCase {
    kIds = 1,
    kFilterQuery = 2,
    SELECTION_NOT_SET = 0,
  };

  SelectionCase selection_case() const;

  // Repeated field in oneof
  int ids_size() const;
  int64_t ids(int index) const;
  void add_ids(int64_t value);
  void clear_ids();
  const RepeatedField<int64_t>& ids() const;
  RepeatedField<int64_t>* mutable_ids();

  // Other member
  const std::string& filter_query() const;
  void set_filter_query(std::string value);

 private:
  SelectionCase selection_case_;
  union {
    RepeatedField<int64_t> ids_;
    std::string filter_query_;
  };
};
```

#### Semantics

- Setting any repeated field element switches oneof to that case
- `clear_ids()` clears values but keeps case as `kIds`
- Setting another field clears the repeated field

### 2. Nested Oneofs

Allow oneofs within oneofs for hierarchical choices:

```protobuf
message Shape {
  oneof shape_type {
    Circle circle = 1;
    oneof quadrilateral {
      Rectangle rectangle = 2;
      Square square = 3;
    }
    Triangle triangle = 4;
  }
}
```

#### Wire Format

Nested oneofs are flattened - only leaf fields have numbers.

#### Generated Code (C++)

```cpp
class Shape : public Message {
 public:
  enum ShapeTypeCase {
    kCircle = 1,
    kRectangle = 2,
    kSquare = 3,
    kTriangle = 4,
    SHAPE_TYPE_NOT_SET = 0,
  };

  enum QuadrilateralCase {
    kRectangle = 2,
    kSquare = 3,
    QUADRILATERAL_NOT_SET = 0,
  };

  ShapeTypeCase shape_type_case() const;
  QuadrilateralCase quadrilateral_case() const;

  bool is_quadrilateral() const {
    return shape_type_case() == kRectangle ||
           shape_type_case() == kSquare;
  }
};
```

### 3. Explicit Unset Tracking

Track whether a oneof was explicitly set vs. never touched:

```protobuf
message Config {
  oneof mode {
    option (oneof_options).track_unset = true;

    int32 timeout_ms = 1;
    bool use_default = 2;
  }
}
```

#### Generated Code

```cpp
class Config : public Message {
 public:
  enum ModeCase {
    kTimeoutMs = 1,
    kUseDefault = 2,
    MODE_NOT_SET = 0,
  };

  ModeCase mode_case() const;
  bool has_mode() const;     // True if any field set
  bool mode_is_unset() const; // True if never set (not just cleared)

  // Can distinguish:
  // - Never set: mode_case() == MODE_NOT_SET && mode_is_unset()
  // - Explicitly cleared: mode_case() == MODE_NOT_SET && !mode_is_unset()
  // - Set to value: mode_case() != MODE_NOT_SET
};
```

#### Wire Format

Add a presence bit for the oneof itself (not just individual fields).

### 4. Required Oneof

Enforce that at least one field must be set:

```protobuf
message Action {
  oneof action_type {
    option (oneof_options).required = true;

    CreateAction create = 1;
    UpdateAction update = 2;
    DeleteAction delete = 3;
  }
}
```

#### Validation

```cpp
absl::Status Action::Validate() const {
  if (action_type_case() == ACTION_TYPE_NOT_SET) {
    return absl::InvalidArgumentError(
        "oneof 'action_type' is required but not set");
  }
  return absl::OkStatus();
}
```

## Example Usage

### Repeated in Oneof

```cpp
Request request;

// Set repeated field
request.add_ids(1);
request.add_ids(2);
request.add_ids(3);
assert(request.selection_case() == Request::kIds);

// Switch to other field
request.set_filter_query("status:active");
assert(request.selection_case() == Request::kFilterQuery);
assert(request.ids_size() == 0);  // Repeated field cleared
```

### Nested Oneof

```cpp
Shape shape;

shape.mutable_rectangle()->set_width(10);
shape.mutable_rectangle()->set_height(20);

assert(shape.shape_type_case() == Shape::kRectangle);
assert(shape.is_quadrilateral() == true);
assert(shape.quadrilateral_case() == Shape::kRectangle);
```

### Explicit Unset

```cpp
Config config;

// Initially unset
assert(!config.has_mode());
assert(config.mode_is_unset());

// Set a value
config.set_timeout_ms(5000);
assert(config.has_mode());
assert(!config.mode_is_unset());

// Clear explicitly
config.clear_mode();
assert(!config.has_mode());
assert(!config.mode_is_unset());  // Was set, then cleared
```

### Required Oneof

```cpp
Action action;
// action_type not set

absl::Status status = action.Validate();
assert(!status.ok());
// "oneof 'action_type' is required but not set"

action.mutable_create()->set_name("new_item");
status = action.Validate();
assert(status.ok());
```

## Implementation Plan

### Week 1: Repeated in Oneof
- [ ] Update parser to allow repeated in oneof
- [ ] Update C++ code generator
- [ ] Handle memory management for union

### Week 2: Nested Oneof
- [ ] Update parser for nested oneof syntax
- [ ] Flatten nested oneofs in descriptor
- [ ] Generate helper methods

### Week 3: Unset Tracking
- [ ] Define oneof_options extension
- [ ] Add presence tracking
- [ ] Update wire format if needed

### Week 4: Java/Python Implementation
- [ ] Java code generator updates
- [ ] Python code generator updates
- [ ] Consistent semantics

### Week 5: Testing and Documentation
- [ ] Comprehensive tests
- [ ] Migration documentation
- [ ] Examples

## Backwards Compatibility

### Repeated in Oneof
- **Wire format:** Compatible (repeated already length-delimited)
- **API:** New methods, existing code unchanged

### Nested Oneof
- **Wire format:** Compatible (flattened)
- **API:** Additional helper methods

### Unset Tracking
- **Wire format:** May need presence bit
- **API:** New methods, opt-in feature

## Alternatives Considered

### Alternative 1: Wrapper Messages Only
- **Pro:** Simple, current approach
- **Con:** Verbose, extra allocations
- **Decision:** Allow direct repeated

### Alternative 2: Multiple Oneofs
- **Pro:** Already works
- **Con:** Can't enforce mutual exclusion
- **Decision:** Nested is clearer

### Alternative 3: Nullable Types
- **Pro:** Explicit presence
- **Con:** Different concept
- **Decision:** Oneof unset tracking fits better

## Open Questions

1. **Performance** - Overhead of unset tracking?
   - Suggestion: Single bit, minimal impact

2. **JSON mapping** - How to represent nested oneof?
   - Suggestion: Flat structure with case field

3. **Interaction with optional** - How does track_unset interact?
   - Suggestion: Orthogonal features

## Success Criteria

- [ ] Repeated in oneof working in C++, Java, Python
- [ ] Nested oneof generating helper methods
- [ ] Unset tracking opt-in feature working
- [ ] Required oneof validation
- [ ] All languages have consistent behavior

## Effort Estimation

| Task | Days |
|------|------|
| Repeated in oneof | 5 |
| Nested oneof | 5 |
| Unset tracking | 4 |
| Required oneof | 2 |
| Java/Python | 5 |
| Testing | 4 |
| Documentation | 3 |
| **Total** | **28** (5 weeks) |

---

## References

- [Protocol Buffers Oneof](https://developers.google.com/protocol-buffers/docs/proto3#oneof)
- [Sum Types in Programming](https://en.wikipedia.org/wiki/Tagged_union)
- [Rust Enums](https://doc.rust-lang.org/book/ch06-01-defining-an-enum.html)
