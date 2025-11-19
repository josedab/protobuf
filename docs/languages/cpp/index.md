# C++ Guide

The C++ implementation of Protocol Buffers provides high-performance serialization with fine-grained memory control.

## Overview

The C++ protobuf library offers:

- **Highest performance** - Optimized for speed and memory
- **Full feature support** - All protobuf features available
- **Memory control** - Arena allocation, custom allocators
- **Reflection** - Runtime metadata and dynamic messages

## Getting Started

1. [Installation](installation.md) - Install protoc and the C++ runtime
2. [Quick Start](quickstart.md) - Create your first C++ protobuf program
3. [API Reference](api-reference.md) - Detailed API documentation
4. [Best Practices](best-practices.md) - Optimization and patterns

## Quick Example

### Define a Message

```protobuf
// person.proto
syntax = "proto3";

message Person {
  string name = 1;
  int32 id = 2;
  string email = 3;
}
```

### Generate Code

```bash
protoc --cpp_out=. person.proto
```

This generates:
- `person.pb.h` - Header file
- `person.pb.cc` - Implementation

### Use the Generated Code

```cpp
#include "person.pb.h"
#include <iostream>
#include <fstream>

int main() {
  // Create a message
  Person person;
  person.set_name("Alice");
  person.set_id(123);
  person.set_email("alice@example.com");

  // Serialize to file
  std::ofstream output("person.bin", std::ios::binary);
  person.SerializeToOstream(&output);
  output.close();

  // Deserialize from file
  Person loaded;
  std::ifstream input("person.bin", std::ios::binary);
  loaded.ParseFromIstream(&input);

  std::cout << "Name: " << loaded.name() << std::endl;
  std::cout << "ID: " << loaded.id() << std::endl;

  return 0;
}
```

### Build

```bash
g++ -o example example.cc person.pb.cc -lprotobuf -pthread
```

## Key Features

### Arena Allocation

Reduce memory fragmentation and improve performance:

```cpp
#include <google/protobuf/arena.h>

google::protobuf::Arena arena;
Person* person = Arena::CreateMessage<Person>(&arena);
person->set_name("Alice");

// All messages freed when arena is destroyed
// No need to delete individual messages
```

### Serialization Options

```cpp
// To string
std::string output;
person.SerializeToString(&output);

// To stream
std::ofstream file("output.bin", std::ios::binary);
person.SerializeToOstream(&file);

// To byte array
size_t size = person.ByteSizeLong();
uint8_t* buffer = new uint8_t[size];
person.SerializeToArray(buffer, size);

// Deterministic (for caching/hashing)
google::protobuf::io::StringOutputStream stream(&output);
google::protobuf::io::CodedOutputStream coded(&stream);
coded.SetSerializationDeterministic(true);
person.SerializeToCodedStream(&coded);
```

### Parsing Options

```cpp
// From string
Person person;
person.ParseFromString(data);

// From stream
person.ParseFromIstream(&input);

// Partial parsing (unknown fields ok)
person.ParsePartialFromString(data);
```

## API Overview

### Generated Message API

```cpp
// Field accessors
person.name();                 // Get value
person.set_name("value");      // Set value
person.has_name();             // Check presence (optional only)
person.clear_name();           // Clear field

// Repeated fields
person.phones_size();          // Get count
person.phones(0);              // Get element
person.add_phones();           // Add element
person.mutable_phones();       // Get mutable list

// Nested messages
person.address();              // Get const reference
person.mutable_address();      // Get mutable pointer
person.has_address();          // Check if set
person.release_address();      // Release ownership

// Message operations
person.CopyFrom(other);        // Deep copy
person.MergeFrom(other);       // Merge fields
person.Clear();                // Clear all fields
person.IsInitialized();        // Check required fields
```

### Utility Functions

```cpp
// Size calculation
size_t size = person.ByteSizeLong();

// Comparison
bool equal = google::protobuf::util::MessageDifferencer::Equals(a, b);

// JSON conversion
std::string json;
google::protobuf::util::MessageToJsonString(person, &json);

// Text format
std::string text;
google::protobuf::TextFormat::PrintToString(person, &text);
```

## Build Systems

### CMake

```cmake
find_package(Protobuf REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS person.proto)

add_executable(myapp main.cc ${PROTO_SRCS})
target_link_libraries(myapp protobuf::libprotobuf)
```

### Bazel

```python
load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_proto_library")

proto_library(
    name = "person_proto",
    srcs = ["person.proto"],
)

cc_proto_library(
    name = "person_cc_proto",
    deps = [":person_proto"],
)
```

## Version Compatibility

| protobuf Version | C++ Standard | Compiler Requirements |
|-----------------|--------------|----------------------|
| 3.21+ | C++14 | GCC 7+, Clang 6+, MSVC 2019+ |
| 4.x | C++14 | GCC 7+, Clang 6+, MSVC 2019+ |

## Resources

- [C++ API Reference](https://protobuf.dev/reference/cpp/api-docs/)
- [Build Systems Guide](../../cpp_build_systems.md)
- [CMake Integration](../../cmake_protobuf_generate.md)
