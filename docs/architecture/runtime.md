# Runtime Libraries

Runtime libraries provide the functionality to work with protocol buffer messages at runtime.

## Overview

Each language has a runtime library that provides:

- Serialization and deserialization
- Message API (getters, setters)
- Reflection and descriptors
- Utility functions (JSON, text format)

## C++ Runtime

### Core Components

**Location**: `src/google/protobuf/`

| Component | Files | Description |
|-----------|-------|-------------|
| Message base | `message.h`, `message_lite.h` | Base classes for messages |
| Serialization | `wire_format.cc`, `coded_stream.h` | Binary encoding/decoding |
| Descriptors | `descriptor.h`, `descriptor.cc` | Runtime type information |
| Reflection | `generated_message_reflection.h` | Dynamic field access |
| Arena | `arena.h`, `arena.cc` | Memory management |

### Message Hierarchy

```
MessageLite        // Minimal API, no reflection
    │
    └── Message    // Full API with reflection
```

### Key Classes

**Message** (`message.h`):

```cpp
class Message : public MessageLite {
  // Reflection access
  const Reflection* GetReflection() const;
  const Descriptor* GetDescriptor() const;

  // Text format
  std::string DebugString() const;
  std::string ShortDebugString() const;
};
```

**Reflection** (`message.h`):

```cpp
class Reflection {
  // Field access
  int32 GetInt32(const Message& message, const FieldDescriptor* field) const;
  void SetInt32(Message* message, const FieldDescriptor* field, int32 value) const;

  // Repeated fields
  int FieldSize(const Message& message, const FieldDescriptor* field) const;
  // ...
};
```

**Arena** (`arena.h`):

```cpp
class Arena {
  template<typename T>
  static T* CreateMessage(Arena* arena);

  // Memory is freed when Arena is destroyed
  ~Arena();
};
```

### Serialization Layer

**Coded streams** (`coded_stream.h`):

```cpp
class CodedInputStream {
  bool ReadVarint32(uint32* value);
  bool ReadString(std::string* buffer, int size);
  // ...
};

class CodedOutputStream {
  void WriteVarint32(uint32 value);
  void WriteString(const std::string& value);
  // ...
};
```

**Wire format** (`wire_format.h`):

```cpp
class WireFormat {
  static bool ParseAndMergePartial(io::CodedInputStream* input, Message* message);
  static void SerializeWithCachedSizes(const Message& message, io::CodedOutputStream* output);
};
```

## Java Runtime

### Core Components

**Location**: `java/core/src/main/java/com/google/protobuf/`

| Component | Files | Description |
|-----------|-------|-------------|
| Message base | `Message.java`, `MessageLite.java` | Message interfaces |
| Builder | `Message.java` (Builder interface) | Builder pattern |
| Serialization | `CodedInputStream.java`, `CodedOutputStream.java` | Binary I/O |
| Descriptors | `Descriptors.java` | Type information |
| ByteString | `ByteString.java` | Immutable bytes |

### Class Hierarchy

```
MessageLite
    │
    └── Message
         │
         └── GeneratedMessage
```

### Key Interfaces

**Message** (`Message.java`):

```java
public interface Message extends MessageLite {
  Descriptor getDescriptorForType();
  Map<FieldDescriptor, Object> getAllFields();
  boolean hasField(FieldDescriptor field);
  Object getField(FieldDescriptor field);
}
```

**Builder** (`Message.java`):

```java
interface Builder extends MessageLite.Builder {
  Builder setField(FieldDescriptor field, Object value);
  Builder clearField(FieldDescriptor field);
  Message build();
}
```

### Serialization

**CodedInputStream** (`CodedInputStream.java`):

```java
public abstract class CodedInputStream {
  public abstract int readTag();
  public abstract int readInt32();
  public abstract String readString();
  // ...
}
```

## Python Runtime

### Core Components

**Location**: `python/google/protobuf/`

| Component | Files | Description |
|-----------|-------|-------------|
| Message base | `message.py` | Message class |
| Descriptors | `descriptor.py` | Type information |
| Reflection | `reflection.py` | Dynamic message creation |
| Serialization | `encoder.py`, `decoder.py` | Binary encoding |
| JSON | `json_format.py` | JSON conversion |

### Implementation Options

Python has two implementations:

1. **Pure Python** - `python_message.py`
2. **C++ Extension** - `pyext/` (faster)

Check implementation:

```python
from google.protobuf.internal import api_implementation
print(api_implementation.Type())  # 'cpp' or 'python'
```

### Key Components

**Message metaclass**:

Creates message classes dynamically from descriptors.

```python
# Generated code uses metaclass
class Person(message.Message, metaclass=reflection.GeneratedProtocolMessageType):
    DESCRIPTOR = _PERSON  # From generated descriptor
```

**Descriptor classes** (`descriptor.py`):

```python
class Descriptor:
    name: str
    full_name: str
    fields: List[FieldDescriptor]
    # ...

class FieldDescriptor:
    name: str
    number: int
    type: int
    # ...
```

## Well-Known Types

Commonly used types provided by protobuf:

**Location**: `src/google/protobuf/` (protos), runtime directories (implementations)

| Type | Proto File | Description |
|------|------------|-------------|
| Any | `any.proto` | Arbitrary message |
| Timestamp | `timestamp.proto` | Point in time |
| Duration | `duration.proto` | Time span |
| Struct | `struct.proto` | Dynamic JSON-like |
| Wrappers | `wrappers.proto` | Nullable primitives |
| Empty | `empty.proto` | Empty message |

### Usage Example

```cpp
#include <google/protobuf/timestamp.pb.h>
#include <google/protobuf/util/time_util.h>

google::protobuf::Timestamp timestamp;
timestamp = TimeUtil::SecondsToTimestamp(time(nullptr));
```

## I/O System

### Zero-Copy Streams

**Location**: `src/google/protobuf/io/`

```cpp
class ZeroCopyInputStream {
  virtual bool Next(const void** data, int* size) = 0;
  virtual void BackUp(int count) = 0;
};

class ZeroCopyOutputStream {
  virtual bool Next(void** data, int* size) = 0;
  virtual void BackUp(int count) = 0;
};
```

Implementations:
- `ArrayInputStream` / `ArrayOutputStream`
- `FileInputStream` / `FileOutputStream`
- `StringOutputStream`

## Utilities

### Text Format

**Location**: `src/google/protobuf/text_format.h`

```cpp
class TextFormat {
  static bool PrintToString(const Message& message, std::string* output);
  static bool ParseFromString(const std::string& input, Message* output);
};
```

### JSON Format

**Location**: `src/google/protobuf/util/json_util.h`

```cpp
namespace util {
Status MessageToJsonString(const Message& message, std::string* output);
Status JsonStringToMessage(const std::string& input, Message* message);
}
```

### Message Differencing

**Location**: `src/google/protobuf/util/message_differencer.h`

```cpp
class MessageDifferencer {
  static bool Equals(const Message& message1, const Message& message2);
  bool Compare(const Message& message1, const Message& message2);
};
```

## Thread Safety

### C++

- Messages are **not** thread-safe for concurrent modification
- Read-only access is safe
- Use `Arena` for efficient batch processing

### Java

- Message instances are **immutable** and thread-safe
- Builders are **not** thread-safe

### Python

- Messages are **not** thread-safe
- Use locks for concurrent access

## See Also

- [Compiler Pipeline](compiler.md)
- [Key Files Reference](key-files.md)
- [C++ Best Practices](../languages/cpp/best-practices.md)
