# RFC-0002: Modular Generated Code

**Status:** Draft
**Author:** Codebase Analysis
**Created:** November 19, 2025
**Effort:** 2 weeks
**Category:** Strategic

---

## Summary

Split large generated files (particularly `descriptor.pb.h` at 22,696 lines) into smaller, modular compilation units to reduce compile times, improve binary size control, and enable better code organization.

## Motivation

### Problem Statement

The current code generation approach produces monolithic files that cause:

1. **Slow compilation** - `descriptor.pb.h` takes significant compile time
2. **Large includes** - Every file including descriptor.h gets 22K+ lines
3. **Binary bloat** - Hard to exclude unused descriptor code
4. **Poor parallelization** - Single large files can't be compiled in parallel

### Evidence from Analysis

| Generated File | Lines | Impact |
|----------------|-------|--------|
| `descriptor.pb.h` | 22,696 | Included everywhere |
| `descriptor.pb.cc` | 17,683 | Large compilation unit |
| Total descriptor | 40,379 | ~1MB of code |

### Impact on Users

- Increased CI times
- Slower local development
- Larger binaries
- Higher memory usage during compilation

## Detailed Design

### Modularization Strategy

Split generated code by message type:

```
Current:
  descriptor.pb.h (22,696 lines)
  descriptor.pb.cc (17,683 lines)

Proposed:
  descriptor/
  ├── descriptor.pb.h           # Main header (includes below)
  ├── file_descriptor.pb.h      # FileDescriptor message
  ├── descriptor.pb.h           # Descriptor message
  ├── field_descriptor.pb.h     # FieldDescriptor message
  ├── enum_descriptor.pb.h      # EnumDescriptor message
  ├── service_descriptor.pb.h   # ServiceDescriptor message
  └── [other_messages].pb.h
```

### Implementation Approach

#### 1. Generator Option

Add new generator option:

```bash
protoc --cpp_out=modular_output=true:. descriptor.proto
```

#### 2. File Split Logic

In `src/google/protobuf/compiler/cpp/file.cc`:

```cpp
bool FileGenerator::GenerateModular(GeneratorContext* context) {
  // Generate one header per top-level message
  for (const auto& message_gen : message_generators_) {
    std::string filename = absl::StrCat(
        StripProto(file_->name()), "/",
        message_gen->descriptor()->name(), ".pb.h");

    auto output = context->Open(filename);
    io::Printer p(output.get());

    GenerateMessageHeader(&p, message_gen.get());
  }

  // Generate umbrella header that includes all
  auto umbrella = context->Open(
      absl::StrCat(StripProto(file_->name()), ".pb.h"));
  io::Printer p(umbrella.get());

  for (const auto& message_gen : message_generators_) {
    p.Emit({{"name", message_gen->descriptor()->name()}},
           R"cpp(#include "$name$.pb.h")cpp");
  }

  return true;
}
```

#### 3. Forward Declarations

Generate forward declarations to minimize includes:

```cpp
// descriptor_fwd.pb.h
namespace google {
namespace protobuf {

class FileDescriptor;
class Descriptor;
class FieldDescriptor;
class EnumDescriptor;
class ServiceDescriptor;

}  // namespace protobuf
}  // namespace google
```

#### 4. Dependency Management

Track and generate minimal includes:

```cpp
// file_descriptor.pb.h
#include "google/protobuf/descriptor_fwd.pb.h"  // Forward decls
#include "google/protobuf/descriptor_options.pb.h"  // Used inline

// Only include what's needed
```

### Generated Structure Example

```cpp
// google/protobuf/descriptor/file_descriptor.pb.h

#pragma once

#include "google/protobuf/message.h"
#include "google/protobuf/descriptor_fwd.pb.h"

namespace google {
namespace protobuf {

class FileDescriptor : public Message {
 public:
  // ... FileDescriptor implementation
 private:
  // ... fields
};

}  // namespace protobuf
}  // namespace google
```

```cpp
// google/protobuf/descriptor.pb.h (umbrella)

#pragma once

// Include all descriptor messages
#include "google/protobuf/descriptor/file_descriptor.pb.h"
#include "google/protobuf/descriptor/descriptor.pb.h"
#include "google/protobuf/descriptor/field_descriptor.pb.h"
// ... etc
```

### Backward Compatibility Layer

Existing includes continue to work:

```cpp
// Existing code unchanged
#include "google/protobuf/descriptor.pb.h"

// New fine-grained includes available
#include "google/protobuf/descriptor/field_descriptor.pb.h"
```

## Example Usage

### Before (Current)

```cpp
// my_code.cc
#include "google/protobuf/descriptor.pb.h"  // 22K lines
// Only using FieldDescriptor but got everything
```

### After (With Modular)

```cpp
// my_code.cc
#include "google/protobuf/descriptor/field_descriptor.pb.h"  // ~1K lines
// Got only what we need
```

### Using the Generator Option

```bash
# Generate modular output
protoc --cpp_out=modular_output=true:generated/ myproto.proto

# Output structure:
# generated/
# ├── myproto.pb.h          # Umbrella header
# ├── myproto/
# │   ├── message_a.pb.h
# │   ├── message_b.pb.h
# │   └── message_c.pb.h
# └── myproto.pb.cc
```

## Implementation Plan

### Phase 1: Design and Prototype (Days 1-3)
- [ ] Design dependency analysis algorithm
- [ ] Prototype split logic for descriptor.proto
- [ ] Validate compile times improve

### Phase 2: Generator Implementation (Days 4-7)
- [ ] Implement `modular_output` option
- [ ] Add forward declaration generation
- [ ] Generate umbrella headers
- [ ] Handle nested messages

### Phase 3: Testing (Days 8-10)
- [ ] Update existing tests
- [ ] Add modular-specific tests
- [ ] Benchmark compile times
- [ ] Test backward compatibility

### Phase 4: Documentation and Rollout (Days 11-14)
- [ ] Document new option
- [ ] Update generator documentation
- [ ] Migrate protobuf internal usage
- [ ] Announce to users

## Backwards Compatibility

### Fully Compatible

- Existing `#include "foo.pb.h"` continues to work
- Default behavior unchanged
- Opt-in via generator option

### Migration Path

1. **Phase 1:** Release with `modular_output` option (optional)
2. **Phase 2:** Enable by default for new protos
3. **Phase 3:** Consider making default for all

## Alternatives Considered

### Alternative 1: Split by Namespace
- **Pro:** Logical grouping
- **Con:** Doesn't reduce individual file size much
- **Decision:** Rejected

### Alternative 2: Lazy Code Generation
- **Pro:** Generate only what's used
- **Con:** Requires build system integration
- **Decision:** Future consideration

### Alternative 3: Precompiled Headers
- **Pro:** Faster compilation
- **Con:** Toolchain-specific, doesn't reduce binary
- **Decision:** Orthogonal improvement

## Open Questions

1. **Default on/off** - Should modular be opt-in or opt-out?
   - Suggestion: Opt-in initially for stability

2. **Split granularity** - One file per message or per logical group?
   - Suggestion: One per top-level message, nested stay together

3. **Source file splits** - Split .cc files too?
   - Suggestion: Yes, for parallel compilation

## Success Criteria

- [ ] 50% reduction in descriptor.pb.h compile time
- [ ] Fine-grained includes working
- [ ] All existing tests pass
- [ ] No increase in total binary size
- [ ] Documentation complete

## Effort Estimation

| Task | Days |
|------|------|
| Design | 2 |
| Implementation | 5 |
| Testing | 3 |
| Documentation | 2 |
| Buffer | 2 |
| **Total** | **14** |

## Rollback Strategy

1. Remove `modular_output` option handling
2. Revert generator changes
3. Regenerate descriptor.pb.h/cc

The option is purely additive, so rollback is low-risk.

---

## References

- [Google C++ Style Guide - Headers](https://google.github.io/styleguide/cppguide.html#Header_Files)
- [Include What You Use](https://include-what-you-use.org/)
- [C++20 Modules](https://en.cppreference.com/w/cpp/language/modules) (future consideration)
