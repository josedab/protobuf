# Python Best Practices

Best practices for using Protocol Buffers in Python applications.

## Performance

### Use C++ Extension

The C++ extension is much faster:

```python
# Check which implementation is being used
from google.protobuf.internal import api_implementation
print(api_implementation.Type())  # Should be 'cpp'
```

### Reuse Messages

```python
# Good: reuse message object
message = MyMessage()
for data in data_list:
    message.Clear()
    message.ParseFromString(data)
    process(message)

# Bad: create new message each time
for data in data_list:
    message = MyMessage()
    message.ParseFromString(data)
    process(message)
```

### Avoid Frequent Serialization

```python
# Bad: serialize multiple times
cache.put(key1, message.SerializeToString())
cache.put(key2, message.SerializeToString())

# Good: serialize once
data = message.SerializeToString()
cache.put(key1, data)
cache.put(key2, data)
```

### Pre-calculate Size

```python
# When you need size for allocation
size = message.ByteSize()
buffer = bytearray(size)
```

## Memory Efficiency

### Clear Large Messages

```python
# Release memory when done
message.Clear()
```

### Use Iterators for Large Repeated Fields

```python
# Good: iterate without copying
for item in message.large_list:
    process(item)

# Bad: creates a copy
items = list(message.large_list)
```

### Stream Large Files

```python
# Good: stream parsing for large files
from google.protobuf.internal.decoder import _DecodeVarint32

def read_delimited_messages(filename, message_type):
    with open(filename, 'rb') as f:
        buf = f.read()
        position = 0
        while position < len(buf):
            msg_len, new_pos = _DecodeVarint32(buf, position)
            position = new_pos
            msg_buf = buf[position:position + msg_len]
            position += msg_len

            message = message_type()
            message.ParseFromString(msg_buf)
            yield message
```

## Code Organization

### Organize Proto Imports

```python
# Good: clear organization
import myproject.proto.common_pb2 as common
import myproject.proto.api_pb2 as api

person = api.Person()
person.status = common.Status.ACTIVE
```

### Create Factory Functions

```python
def create_person(name: str, id: int, email: str = "") -> Person:
    """Create a Person message with common defaults."""
    person = person_pb2.Person()
    person.name = name
    person.id = id
    if email:
        person.email = email
    return person
```

### Wrap Protos in Domain Classes

```python
class User:
    """Domain object wrapping Person proto."""

    def __init__(self, proto: person_pb2.Person = None):
        self._proto = proto or person_pb2.Person()

    @property
    def name(self) -> str:
        return self._proto.name

    @name.setter
    def name(self, value: str):
        self._proto.name = value

    @property
    def email(self) -> Optional[str]:
        return self._proto.email or None

    def to_proto(self) -> person_pb2.Person:
        return self._proto

    @classmethod
    def from_proto(cls, proto: person_pb2.Person) -> 'User':
        return cls(proto)
```

## Type Hints

### Generate .pyi Files

```bash
protoc --python_out=. --pyi_out=. message.proto
```

### Use Type Annotations

```python
from typing import List, Optional

def process_people(address_book: addressbook_pb2.AddressBook) -> List[str]:
    """Extract names from address book."""
    return [person.name for person in address_book.people]

def find_person(
    address_book: addressbook_pb2.AddressBook,
    id: int
) -> Optional[addressbook_pb2.Person]:
    """Find person by ID."""
    for person in address_book.people:
        if person.id == id:
            return person
    return None
```

## Error Handling

### Handle Parse Errors

```python
from google.protobuf.message import DecodeError

def safe_parse(data: bytes, message_type):
    """Safely parse protobuf data."""
    try:
        message = message_type()
        message.ParseFromString(data)
        return message
    except DecodeError as e:
        logger.error(f"Failed to parse {message_type.DESCRIPTOR.name}: {e}")
        return None
```

### Validate Messages

```python
def validate_person(person: person_pb2.Person) -> List[str]:
    """Validate person message."""
    errors = []

    if not person.name:
        errors.append("Name is required")

    if person.id <= 0:
        errors.append("ID must be positive")

    if person.email and "@" not in person.email:
        errors.append("Invalid email format")

    return errors
```

## JSON Interoperability

### Configure JSON Output

```python
from google.protobuf.json_format import MessageToJson

# Include defaults and use proto field names
json_str = MessageToJson(
    message,
    including_default_value_fields=True,
    preserving_proto_field_names=True
)
```

### Handle Unknown Fields in JSON

```python
from google.protobuf.json_format import Parse, ParseError

def parse_json_safe(json_str: str, message_type):
    """Parse JSON with unknown field handling."""
    try:
        return Parse(
            json_str,
            message_type(),
            ignore_unknown_fields=True
        )
    except ParseError as e:
        logger.error(f"JSON parse error: {e}")
        return None
```

## Testing

### Use MessageToDict for Assertions

```python
from google.protobuf.json_format import MessageToDict

def test_person_creation():
    person = create_person("Alice", 123)

    expected = {
        "name": "Alice",
        "id": 123
    }

    assert MessageToDict(person) == expected
```

### Create Test Fixtures

```python
import pytest

@pytest.fixture
def sample_person():
    person = person_pb2.Person()
    person.name = "Test User"
    person.id = 1
    person.email = "test@example.com"
    return person

def test_person_serialization(sample_person):
    data = sample_person.SerializeToString()
    loaded = person_pb2.Person()
    loaded.ParseFromString(data)
    assert loaded == sample_person
```

## Common Pitfalls

### Don't Compare with None

```python
# Bad: message fields are never None
if person.address is None:
    ...

# Good: use HasField
if not person.HasField("address"):
    ...

# Good: for scalar fields, check value
if not person.email:
    ...
```

### Avoid Modifying During Iteration

```python
# Bad: modifying while iterating
for i, phone in enumerate(person.phones):
    if should_remove(phone):
        del person.phones[i]  # Undefined behavior!

# Good: collect indices first
to_remove = []
for i, phone in enumerate(person.phones):
    if should_remove(phone):
        to_remove.append(i)

for i in reversed(to_remove):
    del person.phones[i]
```

### Handle Unknown Enum Values

```python
# Proto3 enums can have unknown values
try:
    name = person_pb2.Person.PhoneType.Name(phone.type)
except ValueError:
    name = f"UNKNOWN({phone.type})"
```

## See Also

- [API Reference](api-reference.md)
- [Performance Guide](https://protobuf.dev/programming-guides/performance/)
