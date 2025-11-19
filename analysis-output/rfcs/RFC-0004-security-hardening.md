# RFC-0004: Security Hardening

**Status:** Draft
**Author:** Codebase Analysis
**Created:** November 19, 2025
**Effort:** 3 weeks
**Category:** Strategic

---

## Summary

Implement comprehensive security hardening measures for the Protocol Buffers parser and runtime, including fuzzing infrastructure improvements, resource limits, and security-focused API additions.

## Motivation

### Problem Statement

Protocol Buffers parsers process untrusted input in many contexts:

1. **Attack surface** - Parser handles untrusted binary data
2. **Resource exhaustion** - Deeply nested messages can exhaust stack
3. **Memory safety** - C++ code has potential memory issues
4. **DoS vectors** - Specially crafted inputs can be slow to parse

### Security Considerations

| Risk | Current Mitigation | Gap |
|------|-------------------|-----|
| Stack overflow | Recursion limits | Not configurable |
| Memory exhaustion | None standard | Need limits |
| Slow parsing | None | Need complexity limits |
| Malformed input | Validation | Could be stricter |

### Benefits

1. **Prevent crashes** - Robust against malformed input
2. **Limit resources** - Configurable memory/CPU limits
3. **Audit trail** - Security-focused logging
4. **Confidence** - Users can parse untrusted data safely

## Detailed Design

### 1. Configurable Resource Limits

Add a `ParseOptions` structure:

```cpp
// src/google/protobuf/parse_options.h

struct ParseOptions {
  // Maximum recursion depth (default: 100)
  int max_recursion_depth = 100;

  // Maximum message size in bytes (default: 64MB)
  size_t max_message_size = 64 * 1024 * 1024;

  // Maximum number of fields (default: 10000)
  int max_field_count = 10000;

  // Maximum string/bytes field size (default: 2GB)
  size_t max_string_size = 2ULL * 1024 * 1024 * 1024;

  // Timeout for parsing (default: no timeout)
  absl::Duration parse_timeout = absl::InfiniteDuration();

  // Strict mode - reject unknown fields
  bool strict_mode = false;

  // Enable security audit logging
  bool enable_audit_log = false;
};
```

### 2. API Integration

```cpp
// New API
bool Message::ParseFromString(const std::string& data,
                              const ParseOptions& options);

// Usage
ParseOptions options;
options.max_recursion_depth = 50;
options.max_message_size = 1024 * 1024;  // 1MB

if (!message.ParseFromString(untrusted_data, options)) {
  // Handle parse error
  LOG(WARNING) << "Parse failed: " << GetLastError();
}
```

### 3. Depth Tracking

Implement runtime depth checking:

```cpp
// src/google/protobuf/io/coded_stream.cc

class CodedInputStream {
 public:
  bool IncrementRecursionDepth() {
    if (++recursion_depth_ > recursion_limit_) {
      return false;  // Exceeded limit
    }
    return true;
  }

  void DecrementRecursionDepth() {
    --recursion_depth_;
  }

 private:
  int recursion_depth_ = 0;
  int recursion_limit_ = 100;  // Configurable
};
```

### 4. Memory Tracking

Track allocations during parsing:

```cpp
class MemoryTracker {
 public:
  bool Allocate(size_t bytes) {
    size_t new_total = allocated_ + bytes;
    if (new_total > limit_) {
      return false;  // Would exceed limit
    }
    allocated_ = new_total;
    return true;
  }

  void Deallocate(size_t bytes) {
    allocated_ -= bytes;
  }

 private:
  size_t allocated_ = 0;
  size_t limit_ = 64 * 1024 * 1024;  // Configurable
};
```

### 5. Fuzzing Infrastructure

Enhance existing fuzzing:

```cpp
// fuzz/parse_fuzzer.cc

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  // Configure restrictive limits for fuzzing
  ParseOptions options;
  options.max_recursion_depth = 32;
  options.max_message_size = 1024 * 1024;
  options.max_field_count = 1000;

  TestMessage message;
  message.ParseFromArray(data, size, options);

  return 0;
}
```

Add to CI:

```yaml
# .github/workflows/fuzz.yml
name: Continuous Fuzzing

on:
  schedule:
    - cron: '0 0 * * *'  # Daily

jobs:
  fuzz:
    runs-on: ubuntu-latest
    steps:
      - uses: google/oss-fuzz/infra/cifuzz@master
        with:
          fuzz-seconds: 600
          dry-run: false
```

### 6. Security Audit Logging

Optional detailed logging for security analysis:

```cpp
void LogSecurityEvent(const SecurityEvent& event) {
  if (!options_.enable_audit_log) return;

  AUDIT_LOG << "event=" << event.type
            << " message_type=" << event.message_type
            << " field=" << event.field_name
            << " size=" << event.size
            << " depth=" << event.depth;
}
```

### 7. Input Validation Improvements

Stricter validation in strict mode:

```cpp
bool ValidateField(const FieldDescriptor* field,
                   const std::string& value,
                   const ParseOptions& options) {
  if (!options.strict_mode) return true;

  // Validate UTF-8 for strings
  if (field->type() == FieldDescriptor::TYPE_STRING) {
    if (!IsValidUtf8(value)) {
      return false;
    }
  }

  // Validate enum values
  if (field->type() == FieldDescriptor::TYPE_ENUM) {
    if (field->enum_type()->FindValueByNumber(value) == nullptr) {
      return false;
    }
  }

  return true;
}
```

## Example Usage

### Basic Security Configuration

```cpp
// Parse untrusted input with security limits
ParseOptions opts;
opts.max_recursion_depth = 32;
opts.max_message_size = 1 * 1024 * 1024;  // 1MB
opts.strict_mode = true;

Message msg;
if (!msg.ParseFromString(untrusted_input, opts)) {
  LOG(ERROR) << "Rejected untrusted input";
  return Status::InvalidArgument("Parse failed");
}
```

### High-Security Environment

```cpp
// Maximum security configuration
ParseOptions opts;
opts.max_recursion_depth = 16;
opts.max_message_size = 100 * 1024;  // 100KB
opts.max_string_size = 10 * 1024;    // 10KB
opts.max_field_count = 100;
opts.strict_mode = true;
opts.enable_audit_log = true;

// Set parse timeout
opts.parse_timeout = absl::Seconds(1);
```

## Implementation Plan

### Week 1: Core Infrastructure
- [ ] Define ParseOptions structure
- [ ] Implement recursion depth tracking
- [ ] Implement memory tracking
- [ ] Add API overloads

### Week 2: Validation and Limits
- [ ] Add field count limits
- [ ] Implement string size limits
- [ ] Add timeout support
- [ ] Implement strict mode validations

### Week 3: Testing and Hardening
- [ ] Enhance fuzzing targets
- [ ] Add security-focused tests
- [ ] Audit log implementation
- [ ] Documentation

## Backwards Compatibility

### Fully Compatible
- Existing APIs continue to work with default limits
- New ParseOptions parameter is optional
- Default limits match current behavior

### Migration Path
Users can opt-in to stricter limits as needed.

## Alternatives Considered

### Alternative 1: Global Limits Only
- **Pro:** Simpler implementation
- **Con:** Not flexible for different use cases
- **Decision:** Rejected - need per-parse configuration

### Alternative 2: Separate Secure Parser
- **Pro:** Clear security boundary
- **Con:** Code duplication, maintenance burden
- **Decision:** Rejected - integrate into main parser

## Open Questions

1. **Default limits** - What should defaults be?
   - Suggestion: Conservative but not breaking existing usage

2. **Error reporting** - How much detail to expose?
   - Suggestion: Detailed internally, generic externally

3. **Performance impact** - Acceptable overhead for tracking?
   - Suggestion: <1% for common cases

## Success Criteria

- [ ] Zero new CVEs from parser/deserializer
- [ ] All existing tests pass with default limits
- [ ] Performance overhead <1%
- [ ] Fuzzer running continuously
- [ ] Security documentation complete

## Effort Estimation

| Task | Days |
|------|------|
| ParseOptions and API | 3 |
| Resource tracking | 4 |
| Strict validation | 3 |
| Fuzzing enhancement | 2 |
| Testing | 3 |
| Documentation | 2 |
| **Total** | **17** (3+ weeks) |

---

## References

- [OWASP Deserialization Cheatsheet](https://cheatsheetseries.owasp.org/cheatsheets/Deserialization_Cheat_Sheet.html)
- [Google OSS-Fuzz](https://github.com/google/oss-fuzz)
- [Protocol Buffers Security Advisories](https://github.com/protocolbuffers/protobuf/security/advisories)
