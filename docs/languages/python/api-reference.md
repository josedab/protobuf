# Python API Reference

Reference for the Protocol Buffers Python API.

## Message API

### Field Access

```python
# Get/set scalar fields
person.name = "Alice"
name = person.name

# Check if set (optional/message fields only)
if person.HasField("address"):
    print(person.address)

# Clear field
person.ClearField("name")

# List all set fields
fields = person.ListFields()
for descriptor, value in fields:
    print(f"{descriptor.name}: {value}")
```

### Repeated Fields

```python
# Add element
phone = person.phones.add()

# Append existing message
person.phones.append(phone)

# Extend with list
person.phones.extend([phone1, phone2])

# Insert at index
person.phones.insert(0, phone)

# Access by index
phone = person.phones[0]

# Slice
subset = person.phones[1:3]

# Length
count = len(person.phones)

# Iterate
for phone in person.phones:
    print(phone.number)

# Pop element
phone = person.phones.pop()

# Remove by value
person.phones.remove(phone)

# Clear all
del person.phones[:]
# or
person.ClearField("phones")
```

### Map Fields

```python
# Set value
message.attributes["key"] = 42

# Get value
value = message.attributes["key"]
value = message.attributes.get("key", default)

# Check key
if "key" in message.attributes:
    ...

# Iterate
for key, value in message.attributes.items():
    print(f"{key}: {value}")

# Keys and values
keys = message.attributes.keys()
values = message.attributes.values()

# Length
count = len(message.attributes)

# Delete
del message.attributes["key"]

# Clear all
message.ClearField("attributes")
```

### Nested Messages

```python
# Access creates if needed
person.address.street = "123 Main St"

# Copy from another message
person.address.CopyFrom(other_address)

# Merge (adds to existing)
person.address.MergeFrom(other_address)

# Clear
person.ClearField("address")
```

### Oneof Fields

```python
# Set field (clears other oneof fields)
content.text = "hello"
content.number = 42  # clears text

# Check which is set
which = content.WhichOneof("data")
if which == "text":
    print(content.text)
elif which is None:
    print("No field set")
```

### Enum Fields

```python
# Set enum
person.status = person_pb2.Person.STATUS_ACTIVE

# Get enum value
status = person.status

# Get enum name
name = person_pb2.Person.Status.Name(person.status)

# Get enum value from name
value = person_pb2.Person.Status.Value("STATUS_ACTIVE")

# List all enum values
for name, value in person_pb2.Person.Status.items():
    print(f"{name}: {value}")
```

## Serialization

### Binary Format

```python
# Serialize
data = message.SerializeToString()

# Parse
message = MyMessage()
message.ParseFromString(data)

# Partial (allows uninitialized)
data = message.SerializePartialToString()
message.MergeFromString(data)
```

### JSON Format

```python
from google.protobuf.json_format import (
    MessageToJson, Parse, MessageToDict, ParseDict
)

# To JSON string
json_string = MessageToJson(message)

# From JSON string
message = Parse(json_string, MyMessage())

# To Python dict
dict_obj = MessageToDict(message)

# From Python dict
message = ParseDict(dict_obj, MyMessage())

# Options
json_string = MessageToJson(
    message,
    including_default_value_fields=True,
    preserving_proto_field_names=True,
    indent=2,
    sort_keys=True,
    float_precision=6
)
```

### Text Format

```python
from google.protobuf.text_format import (
    MessageToString, Parse, Merge
)

# To text
text = MessageToString(message)

# From text
message = Parse(text, MyMessage())

# Merge into existing
Merge(text, message)
```

## Message Operations

### Copying

```python
# Deep copy
new_message = MyMessage()
new_message.CopyFrom(original)

# Merge (add to existing fields)
message.MergeFrom(other)
```

### Comparison

```python
# Equality
if message1 == message2:
    print("Equal")

# Not equal
if message1 != message2:
    print("Different")
```

### Size

```python
# Serialized size in bytes
size = message.ByteSize()
```

### Clear

```python
# Clear all fields
message.Clear()

# Clear specific field
message.ClearField("name")
```

### Validation

```python
# Check if initialized (proto2 required fields)
if message.IsInitialized():
    print("All required fields set")
else:
    print("Initialization errors:", message.FindInitializationErrors())
```

## Descriptors

```python
# Get message descriptor
descriptor = message.DESCRIPTOR

# Message name
name = descriptor.name
full_name = descriptor.full_name

# Fields
for field in descriptor.fields:
    print(f"{field.name}: {field.type}")

# Get field by name
field = descriptor.fields_by_name["name"]

# Get field by number
field = descriptor.fields_by_number[1]

# Enum types
for enum in descriptor.enum_types:
    print(enum.name)
```

## Dynamic Messages

```python
from google.protobuf import descriptor_pool, message_factory

# Get descriptor from pool
pool = descriptor_pool.DescriptorPool()
# ... add descriptors to pool

# Create message class
factory = message_factory.MessageFactory(pool)
message_class = factory.GetPrototype(descriptor)

# Create instance
message = message_class()
```

## Well-Known Types

### Any

```python
from google.protobuf.any_pb2 import Any

# Pack
any_msg = Any()
any_msg.Pack(person)

# Check type
if any_msg.Is(person_pb2.Person.DESCRIPTOR):
    # Unpack
    person = person_pb2.Person()
    any_msg.Unpack(person)

# Type URL
type_url = any_msg.type_url
```

### Timestamp

```python
from google.protobuf.timestamp_pb2 import Timestamp
from datetime import datetime

timestamp = Timestamp()

# From datetime
timestamp.FromDatetime(datetime.now())

# To datetime
dt = timestamp.ToDatetime()

# From seconds
timestamp.FromSeconds(1234567890)

# Current time
timestamp.GetCurrentTime()
```

### Duration

```python
from google.protobuf.duration_pb2 import Duration
from datetime import timedelta

duration = Duration()

# From timedelta
duration.FromTimedelta(timedelta(hours=1, minutes=30))

# To timedelta
td = duration.ToTimedelta()
```

### Wrappers

```python
from google.protobuf.wrappers_pb2 import Int32Value, StringValue

# Wrap value
wrapped = Int32Value(value=42)
wrapped = StringValue(value="hello")

# Access value
value = wrapped.value
```

## Error Handling

```python
from google.protobuf.message import DecodeError

try:
    message.ParseFromString(data)
except DecodeError as e:
    print(f"Failed to parse: {e}")
```

## See Also

- [Best Practices](best-practices.md)
- [Python API Docs](https://googleapis.dev/python/protobuf/latest/)
