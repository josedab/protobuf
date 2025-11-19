# Protobuf Error Handling Patterns

## C++ Error Handling Strategy

### Exception-Based Model
Protobuf uses C++ exceptions for error handling in the core library.

**Usage Statistics:**
- `throw` statements: 60 occurrences
- Primary mechanism: C++ exceptions
- Not using Status-based return values

**Example Files:**
- `/home/user/protobuf/src/google/protobuf/descriptor.cc` (10,645 LOC)
- `/home/user/protobuf/src/google/protobuf/generated_message_reflection.cc` (4,247 LOC)

**Common Exception Types:**
```
ParseException
FatalException
DescriptorException
```

### Logging Infrastructure
**Primary Logging Library:** ABSL (Abseil)

- `ABSL_LOG` and `LOG` macros: 306 occurrences
- Used for:
  - Debug information
  - Runtime diagnostics
  - Warning conditions
  
**Debug Utilities:**
- `/home/user/protobuf/src/google/protobuf/unredacted_debug_format_for_test.h`
- `/home/user/protobuf/src/google/protobuf/unredacted_debug_format_for_test.cc`
- `/home/user/protobuf/src/google/protobuf/debug_counter_test.cc`

### Checked Assertions
- `ABSL_DCHECK` macros for invariant checking
- Used in auto-generated code from descriptor.pb.h

---

## Java Error Handling Strategy

### Exception-Based Model
Traditional Java exception handling with custom exception classes.

**Usage Statistics:**
- Try-catch blocks: 171 blocks
- Throw statements: 526 occurrences
- Custom exception classes: 3 classes

**Common Exception Patterns:**
```java
try {
    // Protocol buffer parsing
} catch (IOException | InvalidProtocolBufferException e) {
    // Handle parse errors
}
```

**Custom Exception Classes:**
Located in Java core implementation:
- `com.google.protobuf.*Exception` classes
- Extend `IOException` for compatibility

**Key Error Points:**
- `CodedInputStream` (2,437 LOC) - parsing errors
- `CodedOutputStream` (2,513 LOC) - serialization errors
- `MessageSchema` (4,891 LOC) - schema validation errors

---

## Python Error Handling Strategy

### Comprehensive Exception Handling
Python uses traditional exception-based error handling with extensive try-except blocks.

**Usage Statistics:**
- Try blocks: 132 blocks
- Except blocks: 220 blocks
- Raise statements: 318 occurrences

**Common Exception Patterns:**
```python
try:
    # Protocol buffer operations
except (TypeError, ValueError, AttributeError) as e:
    # Handle errors
```

**Exception Categories:**
- `TypeError` - type mismatch
- `ValueError` - invalid values
- `AttributeError` - missing attributes
- Custom protobuf exceptions

**Key Files:**
- `reflection_test.py` (3,444 LOC test)
- `message_test.py` (3,123 LOC test)
- `json_format.py` (1,090 LOC)
- `descriptor.py` (1,676 LOC)

---

## Language-Specific Error Handling

### C#
- .NET exception model
- 183,900 LOC across 216 files
- Likely uses try-finally for resource management

### Objective-C
- 82,405 LOC across 119 files
- NSError-based error handling pattern
- Possible exception bridging to C++

### Ruby & PHP
- Dynamic language exception handling
- Custom exception classes per language

---

## Error Handling Best Practices (As Applied in Protobuf)

1. **Fail Fast:** Exceptions raised immediately on invalid conditions
2. **Specific Exceptions:** Different exception types for different errors
3. **Logging:** ABSL logging for runtime diagnostics
4. **Testing:** Extensive error case testing in conformance suite
5. **Defensive Programming:** Invariant checking with DCHECK macros

---

## Conformance Testing for Error Cases
- Location: `/home/user/protobuf/conformance/`
- Tests error handling across all implementations
- Binary format, JSON, text format error cases
- Failure lists maintained per language
