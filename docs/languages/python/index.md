# Python Guide

The Python implementation of Protocol Buffers provides an easy-to-use, Pythonic API for working with protocol buffer messages.

## Overview

The Python protobuf library offers:

- **Pythonic API** - Natural Python attribute access
- **Full feature support** - All protobuf features available
- **Dynamic messages** - Create messages without compiled protos
- **Integration** - Works with gRPC and other Python libraries

## Getting Started

1. [Installation](installation.md) - Install protobuf for Python
2. [Quick Start](quickstart.md) - Create your first Python protobuf program
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
protoc --python_out=. person.proto
```

### Use the Generated Code

```python
import person_pb2

# Create a message
person = person_pb2.Person()
person.name = "Alice"
person.id = 123
person.email = "alice@example.com"

# Serialize to bytes
data = person.SerializeToString()

# Save to file
with open("person.bin", "wb") as f:
    f.write(data)

# Load from file
loaded = person_pb2.Person()
with open("person.bin", "rb") as f:
    loaded.ParseFromString(f.read())

print(f"Name: {loaded.name}")
print(f"ID: {loaded.id}")
```

## Key Features

### Pythonic Access

```python
# Set fields
person.name = "Alice"
person.id = 123

# Get fields
print(person.name)
print(person.id)

# Check presence (optional/message fields)
person.HasField("name")

# Clear field
person.ClearField("name")
```

### Repeated Fields

```python
# Add elements
person.phones.append(phone)
person.phones.extend([phone1, phone2])
phone = person.phones.add()  # Add and return new element

# Access elements
count = len(person.phones)
first = person.phones[0]

# Iterate
for phone in person.phones:
    print(phone.number)

# Slice
some_phones = person.phones[1:3]
```

### Nested Messages

```python
# Direct assignment creates new message
person.address.street = "123 Main St"
person.address.city = "Springfield"

# Copy from another message
person.address.CopyFrom(other_address)

# Check if set
if person.HasField("address"):
    print(person.address.city)
```

### Map Fields

```python
# Set values
message.counts["key1"] = 42
message.counts["key2"] = 100

# Get values
value = message.counts.get("key1", 0)

# Iterate
for key, value in message.counts.items():
    print(f"{key}: {value}")
```

### Oneof Fields

```python
# Set a oneof field
content.text = "hello"

# Check which is set
which = content.WhichOneof("data")
if which == "text":
    print(content.text)
elif which == "number":
    print(content.number)
```

## Installation

### pip

```bash
pip install protobuf
```

### conda

```bash
conda install -c conda-forge protobuf
```

## Type Hints (Python 3.7+)

Use generated `.pyi` files for type hints:

```bash
protoc --python_out=. --pyi_out=. message.proto
```

Then your IDE will provide autocomplete and type checking.

## Resources

- [Python API Reference](https://googleapis.dev/python/protobuf/latest/)
- [Python Tutorial](https://protobuf.dev/getting-started/pythontutorial/)
