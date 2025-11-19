# Enhanced Oneof Semantics (RFC-0013)

This document describes the enhanced oneof semantics added to Protocol Buffers,
based on RFC-0013.

## Overview

Enhanced oneof semantics extend Protocol Buffers oneof fields to support:

1. **Repeated fields in oneofs** - Use repeated fields directly in oneofs
2. **Nested oneofs** - Create hierarchical choice structures
3. **Unset tracking** - Distinguish between never-set and explicitly-cleared
4. **Required oneofs** - Enforce that at least one field must be set

## Features

### 1. Repeated Fields in Oneofs

Previously, repeated fields could not be placed directly in oneofs, requiring
wrapper messages. Now you can write:

```protobuf
message Request {
  oneof selection {
    repeated int64 ids = 1;    // Now allowed!
    string filter_query = 2;
  }
}
```

#### Semantics

- Setting any element of the repeated field switches the oneof to that case
- `clear_ids()` clears the values but keeps the case as `kIds`
- Setting another field in the oneof clears the repeated field

#### Example Usage (C++)

```cpp
Request request;

// Set repeated field
request.add_ids(1);
request.add_ids(2);
request.add_ids(3);
assert(request.selection_case() == Request::kIds);

// Switch to other field - clears the repeated field
request.set_filter_query("status:active");
assert(request.selection_case() == Request::kFilterQuery);
assert(request.ids_size() == 0);  // Repeated field was cleared
```

### 2. Nested Oneofs

Nested oneofs allow hierarchical choice structures:

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

#### Generated Helper Methods

The code generator produces helper methods for checking nested membership:

```cpp
Shape shape;

shape.mutable_rectangle()->set_width(10);

assert(shape.shape_type_case() == Shape::kRectangle);
assert(shape.is_quadrilateral() == true);  // Helper method
assert(shape.quadrilateral_case() == Shape::kRectangle);
```

### 3. Unset Tracking

Track whether a oneof was explicitly set vs never touched:

```protobuf
message Config {
  oneof mode {
    option (track_unset) = true;

    int32 timeout_ms = 1;
    bool use_default = 2;
  }
}
```

#### Generated Methods

- `has_mode()` - Returns true if any field is set
- `mode_is_unset()` - Returns true if never set (not just cleared)

#### Example Usage

```cpp
Config config;

// Initially unset
assert(!config.has_mode());
assert(config.mode_is_unset());  // Never touched

// Set a value
config.set_timeout_ms(5000);
assert(config.has_mode());
assert(!config.mode_is_unset());

// Clear explicitly
config.clear_mode();
assert(!config.has_mode());
assert(!config.mode_is_unset());  // Was set, then cleared
```

### 4. Required Oneofs

Enforce that at least one field must be set:

```protobuf
message Action {
  oneof action_type {
    option (required) = true;

    CreateAction create = 1;
    UpdateAction update = 2;
    DeleteAction delete = 3;
  }
}
```

#### Validation

```cpp
Action action;
// action_type not set

absl::Status status = action.Validate();
assert(!status.ok());
// Error: "oneof 'action_type' is required but not set"

action.mutable_create()->set_name("new_item");
status = action.Validate();
assert(status.ok());  // Now valid
```

## Wire Format Compatibility

- **Repeated in oneof**: Wire format is unchanged (repeated fields are already
  length-delimited)
- **Nested oneofs**: Flattened on the wire (only leaf fields have numbers)
- **Unset tracking**: May use a presence bit for the oneof itself
- **Required oneofs**: No wire format changes (validation-only)

## OneofOptions Proto

The new options are defined in `descriptor.proto`:

```protobuf
message OneofOptions {
  optional FeatureSet features = 1;

  // Track whether the oneof was explicitly set vs never touched
  optional bool track_unset = 2 [default = false];

  // Require that at least one field must be set
  optional bool required = 3 [default = false];

  repeated UninterpretedOption uninterpreted_option = 999;
  extensions 1000 to max;
}
```

## Implementation Status

### Completed

- Parser updates to allow repeated fields in oneofs
- Parser updates for nested oneof syntax
- Descriptor validation updates
- OneofOptions extensions (track_unset, required)
- Helper functions in C++ code generator
- Test framework

### In Progress

- Full C++ code generation for repeated fields in unions
- Generated helper methods for nested oneofs
- Validation methods for required oneofs
- Java and Python code generators

### Future Work

- Complete code generator support for all features
- JSON mapping for nested oneofs
- Performance optimization for unset tracking

## Migration Guide

### From Wrapper Messages

If you previously used wrapper messages for repeated fields in oneofs:

```protobuf
// Old pattern
message Request {
  oneof selection {
    IdList ids = 1;
    string filter = 2;
  }
}
message IdList {
  repeated int64 values = 1;
}

// New pattern
message Request {
  oneof selection {
    repeated int64 ids = 1;
    string filter = 2;
  }
}
```

### Accessing Repeated Oneof Fields

```cpp
// Old pattern
for (int i = 0; i < request.ids().values_size(); ++i) {
  process(request.ids().values(i));
}

// New pattern
for (int i = 0; i < request.ids_size(); ++i) {
  process(request.ids(i));
}
```

## References

- [RFC-0013: Enhanced Oneof Semantics](analysis-output/rfcs/RFC-0013-enhanced-oneof-semantics.md)
- [Protocol Buffers Oneof Documentation](https://developers.google.com/protocol-buffers/docs/proto3#oneof)
