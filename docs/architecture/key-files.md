# Key Files Reference

Quick reference to important source files in the Protocol Buffers repository.

## Compiler

| Purpose | File |
|---------|------|
| Compiler entry point | `src/google/protobuf/compiler/main.cc` |
| CLI implementation | `src/google/protobuf/compiler/command_line_interface.cc` |
| Proto parser | `src/google/protobuf/compiler/parser.cc` |
| Plugin protocol | `src/google/protobuf/compiler/plugin.proto` |

### Code Generators

| Language | Entry Point | Directory |
|----------|-------------|-----------|
| C++ | `src/google/protobuf/compiler/cpp/generator.cc` | `compiler/cpp/` |
| Java | `src/google/protobuf/compiler/java/generator.cc` | `compiler/java/` |
| Python | `src/google/protobuf/compiler/python/generator.cc` | `compiler/python/` |
| C# | `src/google/protobuf/compiler/csharp/csharp_generator.cc` | `compiler/csharp/` |
| Objective-C | `src/google/protobuf/compiler/objectivec/objectivec_generator.cc` | `compiler/objectivec/` |
| PHP | `src/google/protobuf/compiler/php/php_generator.cc` | `compiler/php/` |
| Ruby | `src/google/protobuf/compiler/ruby/ruby_generator.cc` | `compiler/ruby/` |

## C++ Runtime

### Core Classes

| Purpose | File |
|---------|------|
| Message base class | `src/google/protobuf/message.h` |
| Lite message base | `src/google/protobuf/message_lite.h` |
| Descriptor classes | `src/google/protobuf/descriptor.h` |
| Descriptor implementation | `src/google/protobuf/descriptor.cc` |
| Reflection | `src/google/protobuf/generated_message_reflection.h` |
| Arena allocation | `src/google/protobuf/arena.h` |

### Serialization

| Purpose | File |
|---------|------|
| Coded streams | `src/google/protobuf/io/coded_stream.h` |
| Wire format | `src/google/protobuf/wire_format.h` |
| Wire format lite | `src/google/protobuf/wire_format_lite.h` |

### I/O

| Purpose | File |
|---------|------|
| Zero-copy streams | `src/google/protobuf/io/zero_copy_stream.h` |
| File streams | `src/google/protobuf/io/zero_copy_stream_impl.h` |
| String streams | `src/google/protobuf/io/zero_copy_stream_impl_lite.h` |

### Utilities

| Purpose | File |
|---------|------|
| Text format | `src/google/protobuf/text_format.h` |
| JSON utilities | `src/google/protobuf/util/json_util.h` |
| Message differencer | `src/google/protobuf/util/message_differencer.h` |
| Time utilities | `src/google/protobuf/util/time_util.h` |

## Java Runtime

### Core Classes

| Purpose | File |
|---------|------|
| Message interface | `java/core/src/main/java/com/google/protobuf/Message.java` |
| MessageLite interface | `java/core/src/main/java/com/google/protobuf/MessageLite.java` |
| Generated message base | `java/core/src/main/java/com/google/protobuf/GeneratedMessage.java` |
| Descriptors | `java/core/src/main/java/com/google/protobuf/Descriptors.java` |

### Serialization

| Purpose | File |
|---------|------|
| Input stream | `java/core/src/main/java/com/google/protobuf/CodedInputStream.java` |
| Output stream | `java/core/src/main/java/com/google/protobuf/CodedOutputStream.java` |
| ByteString | `java/core/src/main/java/com/google/protobuf/ByteString.java` |

### Utilities

| Purpose | File |
|---------|------|
| JSON format | `java/util/src/main/java/com/google/protobuf/util/JsonFormat.java` |
| Text format | `java/core/src/main/java/com/google/protobuf/TextFormat.java` |

## Python Runtime

### Core Modules

| Purpose | File |
|---------|------|
| Message base | `python/google/protobuf/message.py` |
| Descriptors | `python/google/protobuf/descriptor.py` |
| Reflection | `python/google/protobuf/reflection.py` |
| Symbol database | `python/google/protobuf/symbol_database.py` |

### Serialization

| Purpose | File |
|---------|------|
| Encoder | `python/google/protobuf/internal/encoder.py` |
| Decoder | `python/google/protobuf/internal/decoder.py` |
| Wire format | `python/google/protobuf/internal/wire_format.py` |

### Utilities

| Purpose | File |
|---------|------|
| JSON format | `python/google/protobuf/json_format.py` |
| Text format | `python/google/protobuf/text_format.py` |

### C++ Extension

| Purpose | File |
|---------|------|
| Python API implementation | `python/google/protobuf/pyext/message.cc` |
| Descriptor pool | `python/google/protobuf/pyext/descriptor_pool.cc` |

## Well-Known Types

### Proto Definitions

| Type | File |
|------|------|
| Any | `src/google/protobuf/any.proto` |
| Duration | `src/google/protobuf/duration.proto` |
| Timestamp | `src/google/protobuf/timestamp.proto` |
| Struct | `src/google/protobuf/struct.proto` |
| Wrappers | `src/google/protobuf/wrappers.proto` |
| Empty | `src/google/protobuf/empty.proto` |
| Field mask | `src/google/protobuf/field_mask.proto` |

### Descriptor Proto

| File | Description |
|------|-------------|
| `src/google/protobuf/descriptor.proto` | Self-describing protocol buffer format |

## Build Files

| Purpose | File |
|---------|------|
| CMake | `CMakeLists.txt` |
| Bazel | `BUILD.bazel`, various `BUILD` files |

## Tests

### C++ Tests

| Purpose | File |
|---------|------|
| Message tests | `src/google/protobuf/message_unittest.cc` |
| Descriptor tests | `src/google/protobuf/descriptor_unittest.cc` |
| Wire format tests | `src/google/protobuf/wire_format_unittest.cc` |

### Java Tests

| Purpose | Directory |
|---------|-----------|
| Core tests | `java/core/src/test/java/com/google/protobuf/` |

### Python Tests

| Purpose | Directory |
|---------|-----------|
| Python tests | `python/google/protobuf/internal/*_test.py` |

## Documentation

| Purpose | File |
|---------|------|
| C++ style guide | `docs/upb/style-guide.md` |
| Design docs | `docs/design/` |

## See Also

- [Architecture Overview](overview.md)
- [Compiler Pipeline](compiler.md)
- [Runtime Libraries](runtime.md)
