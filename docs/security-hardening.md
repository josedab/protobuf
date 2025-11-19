# Security Hardening for Protocol Buffers

This document describes the security hardening features available for parsing untrusted Protocol Buffer input.

## Overview

When parsing Protocol Buffers from untrusted sources, it's important to configure appropriate security limits to prevent:

- **Stack overflow** from deeply nested messages
- **Memory exhaustion** from very large messages
- **DoS attacks** from malformed input
- **Resource exhaustion** from messages with many fields

## Quick Start

### For Untrusted Input

```cpp
#include "google/protobuf/secure_parsing.h"

// Parse untrusted input with safe defaults
MyMessage message;
if (google::protobuf::ParseUntrusted(&message, untrusted_data)) {
  // Success - message is safe to use
} else {
  // Parse failed - input was malformed or exceeded limits
}
```

### For Maximum Security

```cpp
#include "google/protobuf/secure_parsing.h"

// Parse with maximum security
MyMessage message;
if (google::protobuf::ParseHighSecurity(&message, untrusted_data)) {
  // Success
}
```

### With Custom Options

```cpp
#include "google/protobuf/parse_options.h"
#include "google/protobuf/secure_parsing.h"

google::protobuf::ParseOptions options;
options.max_recursion_depth = 50;
options.max_message_size = 1 * 1024 * 1024;  // 1MB
options.max_string_size = 10 * 1024;  // 10KB
options.strict_mode = true;

MyMessage message;
auto result = google::protobuf::SecureParseFromString(&message, data, options);
if (result.success) {
  // Success
} else {
  LOG(ERROR) << "Parse failed: " << result.error_message;
}
```

## ParseOptions

The `ParseOptions` struct allows configuring security limits:

| Option | Default | Description |
|--------|---------|-------------|
| `max_recursion_depth` | 100 | Maximum nesting depth for messages |
| `max_message_size` | 64MB | Maximum total message size |
| `max_field_count` | 10000 | Maximum number of fields |
| `max_string_size` | 2GB | Maximum size of string/bytes fields |
| `parse_timeout` | infinite | Maximum time to spend parsing |
| `strict_mode` | false | Reject unknown fields |
| `validate_utf8` | false | Validate UTF-8 in strings |
| `enable_audit_log` | false | Enable security event logging |

## Preset Configurations

Three preset configurations are available:

### Trusted (Default)

```cpp
ParseOptions::Trusted()
```

Uses default limits - same as current protobuf behavior. Suitable for parsing trusted internal data.

### Untrusted

```cpp
ParseOptions::Untrusted()
```

Uses restrictive limits suitable for parsing external/untrusted data:
- Max recursion: 32
- Max message size: 1MB
- Max fields: 1000
- Max string size: 100KB
- Strict mode: enabled
- UTF-8 validation: enabled

### High Security

```cpp
ParseOptions::HighSecurity()
```

Maximum security configuration:
- Max recursion: 16
- Max message size: 100KB
- Max fields: 100
- Max string size: 10KB
- Parse timeout: 1 second
- Strict mode: enabled
- Audit logging: enabled

## Security Audit Logging

Enable audit logging to track security events:

```cpp
#include "google/protobuf/parse_options.h"
#include "google/protobuf/secure_parsing.h"

void MyAuditCallback(const google::protobuf::SecurityEvent& event) {
  LOG(INFO) << "Security event: " << static_cast<int>(event.type)
            << " message=" << event.message_type
            << " size=" << event.size
            << " depth=" << event.depth;
}

ParseOptions options;
options.enable_audit_log = true;
options.audit_callback = MyAuditCallback;
```

### Security Event Types

- `kParseStarted` - Parsing began
- `kParseCompleted` - Parsing succeeded
- `kParseFailed` - Parsing failed
- `kRecursionLimitExceeded` - Recursion depth exceeded
- `kMessageSizeLimitExceeded` - Message too large
- `kFieldCountLimitExceeded` - Too many fields
- `kStringSizeLimitExceeded` - String field too large
- `kTimeoutExceeded` - Parse timeout exceeded
- `kUnknownFieldRejected` - Unknown field in strict mode
- `kInvalidUtf8Rejected` - Invalid UTF-8 in string

## API Reference

### SecureParseFromString

```cpp
SecureParseResult SecureParseFromString(
    MessageLite* message,
    absl::string_view data,
    const ParseOptions& options);
```

Parse from string with detailed result including metrics.

### SecureParseFromArray

```cpp
SecureParseResult SecureParseFromArray(
    MessageLite* message,
    const void* data,
    int size,
    const ParseOptions& options);
```

Parse from byte array with detailed result.

### ParseFromStringWithOptions

```cpp
bool ParseFromStringWithOptions(
    MessageLite* message,
    absl::string_view data,
    const ParseOptions& options);
```

Simple bool return for compatibility.

### ParseUntrusted / ParseHighSecurity

```cpp
bool ParseUntrusted(MessageLite* message, absl::string_view data);
bool ParseHighSecurity(MessageLite* message, absl::string_view data);
```

Convenience functions with preset options.

## SecureParseResult

The `SecureParseResult` struct provides detailed information:

```cpp
struct SecureParseResult {
  bool success;              // Whether parsing succeeded
  std::string error_message; // Error description if failed
  int max_depth_reached;     // Maximum recursion depth seen
  int field_count;           // Number of fields parsed
  size_t bytes_parsed;       // Bytes successfully parsed
};
```

## Best Practices

1. **Always use security limits for external data** - Use `ParseUntrusted()` or custom `ParseOptions` for any data from external sources.

2. **Set appropriate limits for your use case** - Consider your message schema when setting limits. Don't use unnecessarily large limits.

3. **Enable audit logging in production** - Security events can help detect attacks and debug issues.

4. **Use timeouts for network-received data** - Set `parse_timeout` to prevent slow parsing attacks.

5. **Validate business logic separately** - Security parsing validates format, not business rules. Always validate your data semantically after parsing.

## Migration Guide

### From Standard Parsing

Replace:
```cpp
message.ParseFromString(data);
```

With:
```cpp
#include "google/protobuf/secure_parsing.h"

google::protobuf::ParseUntrusted(&message, data);
```

### Gradual Migration

For gradual migration, start with default options and progressively tighten:

```cpp
// Step 1: Add wrapper with default options
ParseOptions options = ParseOptions::Trusted();
ParseFromStringWithOptions(&message, data, options);

// Step 2: Reduce limits
options.max_recursion_depth = 50;
options.max_message_size = 10 * 1024 * 1024;

// Step 3: Enable strict mode
options.strict_mode = true;

// Step 4: Use untrusted preset
options = ParseOptions::Untrusted();
```

## Performance Considerations

The security parsing adds minimal overhead:
- Size checking: O(1)
- Recursion tracking: O(depth)
- Memory tracking: O(allocations)

Expected overhead is <1% for typical messages.

## Testing

Security parsing includes comprehensive tests in `secure_parsing_test.cc`. Run with:

```bash
bazel test //src/google/protobuf:secure_parsing_test
```

## Fuzzing

A fuzzing target is provided in `fuzz/secure_parse_fuzzer.cc`. This target tests parsing with restrictive limits to find potential issues.

## See Also

- [ParseOptions Reference](../src/google/protobuf/parse_options.h)
- [SecureParsing API](../src/google/protobuf/secure_parsing.h)
- [RFC-0004: Security Hardening](analysis-output/rfcs/RFC-0004-security-hardening.md)
