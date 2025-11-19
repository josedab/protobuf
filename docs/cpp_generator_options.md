# C++ Protocol Buffer Generator Options

This document describes the options available for the C++ protocol buffer
generator (protoc `--cpp_out`).

## Usage

Generator options are specified using the following syntax:

```bash
protoc --cpp_out=OPTION1=VALUE1,OPTION2=VALUE2:OUTPUT_DIR input.proto
```

For options without values, simply specify the option name:

```bash
protoc --cpp_out=modular_output:OUTPUT_DIR input.proto
```

## Available Options

### modular_output

**Type:** Boolean flag
**Default:** `false`

When enabled, generates modular output with one header file per top-level
message. This approach reduces compile times by allowing fine-grained includes.

**Generated File Structure:**

```
Input: myproto.proto (with messages A, B, C)

Output:
├── myproto.pb.h          # Umbrella header (includes all)
├── myproto_fwd.pb.h      # Forward declarations
├── myproto/
│   ├── message_0.pb.h    # Message A
│   ├── message_1.pb.h    # Message B
│   └── message_2.pb.h    # Message C
└── myproto.pb.cc         # Implementation
```

**Benefits:**

- **Reduced compile times:** Only include the messages you need
- **Better parallelization:** Multiple headers can be compiled in parallel
- **Smaller includes:** Avoid pulling in 22K+ lines for large descriptor files
- **Binary size control:** Easier to exclude unused code

**Usage Example:**

```bash
# Generate modular output
protoc --cpp_out=modular_output:generated/ myproto.proto

# In your code - include only what you need
#include "myproto/message_0.pb.h"  // Only Message A

# Or use the umbrella header for backward compatibility
#include "myproto.pb.h"  // All messages
```

**Backward Compatibility:**

Existing code that includes `myproto.pb.h` will continue to work unchanged.
The umbrella header includes all individual message headers.

### dllexport_decl

**Type:** String
**Default:** (empty)

Specifies a macro to use for DLL export declarations on Windows.

**Example:**

```bash
protoc --cpp_out=dllexport_decl=MY_EXPORT:. foo.proto
```

This generates classes like:

```cpp
class MY_EXPORT Foo {
  // ...
};
```

### proto_h

**Type:** Boolean flag
**Default:** `false`

Generates a separate `.proto.h` header with forward declarations.

### lite

**Type:** Boolean flag
**Default:** `false`

Enforces lite runtime for all generated code.

### speed

**Type:** Boolean flag
**Default:** `false`

Enforces speed-optimized generated code implementation.

### code_size

**Type:** Boolean flag
**Default:** `false`

Enforces code-size-optimized reflective implementation.

### lite_implicit_weak_fields

**Type:** Integer (optional value)
**Default:** `false`

Enables implicit weak fields in lite runtime mode. Can optionally specify
the number of `.cc` files to generate for better linker stripping.

**Example:**

```bash
protoc --cpp_out=lite_implicit_weak_fields=10:. foo.proto
```

### annotate_headers

**Type:** Boolean flag
**Default:** `false`

Enables annotation of generated headers for location tracking.

### annotation_pragma_name

**Type:** String
**Default:** (empty)

Specifies the pragma name for annotations.

### annotation_guard_name

**Type:** String
**Default:** (empty)

Specifies the guard name for annotations.

### safe_boundary_check

**Type:** Boolean flag
**Default:** `false`

Returns default values when accessing out-of-bounds repeated fields.
(Google-internal only)

### enforced_boundary_check

**Type:** Boolean flag
**Default:** `false`

Aborts when accessing out-of-bounds repeated fields.
(Google-internal only)

### experimental_strip_nonfunctional_codegen

**Type:** Boolean flag
**Default:** `false`

Strips non-functional generated code for cleaner output.

## Combining Options

Multiple options can be combined:

```bash
protoc --cpp_out=modular_output,lite,dllexport_decl=MY_EXPORT:. foo.proto
```

## See Also

- [Protocol Buffers C++ Tutorial](https://developers.google.com/protocol-buffers/docs/cpptutorial)
- [C++ Generated Code Guide](https://developers.google.com/protocol-buffers/docs/reference/cpp-generated)
