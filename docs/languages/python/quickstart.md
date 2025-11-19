# Python Quick Start

This tutorial walks you through creating a Python application using Protocol Buffers.

## Prerequisites

- Python 3.7 or later
- protobuf package installed (`pip install protobuf`)
- Protocol Buffers compiler (`protoc`)

## Step 1: Define the Schema

Create `addressbook.proto`:

```protobuf
syntax = "proto3";

package tutorial;

import "google/protobuf/timestamp.proto";

message Person {
  string name = 1;
  int32 id = 2;
  string email = 3;

  enum PhoneType {
    PHONE_TYPE_UNSPECIFIED = 0;
    PHONE_TYPE_MOBILE = 1;
    PHONE_TYPE_HOME = 2;
    PHONE_TYPE_WORK = 3;
  }

  message PhoneNumber {
    string number = 1;
    PhoneType type = 2;
  }

  repeated PhoneNumber phones = 4;
  google.protobuf.Timestamp last_updated = 5;
}

message AddressBook {
  repeated Person people = 1;
}
```

## Step 2: Generate Python Code

```bash
protoc --python_out=. addressbook.proto
```

This creates `addressbook_pb2.py`.

## Step 3: Write the Application

### Writing Messages

Create `add_person.py`:

```python
#!/usr/bin/env python3
"""Add a person to the address book."""

import sys
from pathlib import Path

import addressbook_pb2
from google.protobuf.timestamp_pb2 import Timestamp
from datetime import datetime


def prompt_for_person():
    """Prompt user for person information."""
    person = addressbook_pb2.Person()

    person.id = int(input("Enter person ID: "))
    person.name = input("Enter name: ")

    email = input("Enter email (blank for none): ")
    if email:
        person.email = email

    while True:
        number = input("Enter phone number (blank to finish): ")
        if not number:
            break

        phone = person.phones.add()
        phone.number = number

        phone_type = input("Phone type (mobile/home/work): ")
        if phone_type == "mobile":
            phone.type = addressbook_pb2.Person.PHONE_TYPE_MOBILE
        elif phone_type == "home":
            phone.type = addressbook_pb2.Person.PHONE_TYPE_HOME
        elif phone_type == "work":
            phone.type = addressbook_pb2.Person.PHONE_TYPE_WORK
        else:
            print("Unknown type, using default.")

    # Set current timestamp
    timestamp = Timestamp()
    timestamp.FromDatetime(datetime.now())
    person.last_updated.CopyFrom(timestamp)

    return person


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} ADDRESS_BOOK_FILE")
        sys.exit(-1)

    address_book = addressbook_pb2.AddressBook()
    filepath = Path(sys.argv[1])

    # Read existing address book if it exists
    if filepath.exists():
        with open(filepath, "rb") as f:
            address_book.ParseFromString(f.read())

    # Add a new person
    person = prompt_for_person()
    address_book.people.append(person)

    # Write the updated address book
    with open(filepath, "wb") as f:
        f.write(address_book.SerializeToString())

    print(f"Saved {len(address_book.people)} people to {filepath}")


if __name__ == "__main__":
    main()
```

### Reading Messages

Create `list_people.py`:

```python
#!/usr/bin/env python3
"""List people in the address book."""

import sys
from pathlib import Path

import addressbook_pb2


def list_people(address_book):
    """Display all people in the address book."""
    for person in address_book.people:
        print(f"Person ID: {person.id}")
        print(f"  Name: {person.name}")

        if person.email:
            print(f"  Email: {person.email}")

        for phone in person.phones:
            phone_type = addressbook_pb2.Person.PhoneType.Name(phone.type)
            print(f"  {phone_type}: {phone.number}")

        if person.HasField("last_updated"):
            dt = person.last_updated.ToDatetime()
            print(f"  Updated: {dt}")

        print()


def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} ADDRESS_BOOK_FILE")
        sys.exit(-1)

    filepath = Path(sys.argv[1])
    if not filepath.exists():
        print(f"File not found: {filepath}")
        sys.exit(-1)

    address_book = addressbook_pb2.AddressBook()
    with open(filepath, "rb") as f:
        address_book.ParseFromString(f.read())

    list_people(address_book)


if __name__ == "__main__":
    main()
```

## Step 4: Run the Application

```bash
# Generate proto
protoc --python_out=. addressbook.proto

# Add a person
python add_person.py addressbook.bin

# List people
python list_people.py addressbook.bin
```

## Key Concepts

### Creating Messages

```python
# Create empty message
person = addressbook_pb2.Person()

# Set fields
person.name = "Alice"
person.id = 123

# Alternative: merge from another message
other = addressbook_pb2.Person()
other.CopyFrom(person)
```

### Serialization

```python
# To bytes
data = person.SerializeToString()

# From bytes
person = addressbook_pb2.Person()
person.ParseFromString(data)

# To JSON
from google.protobuf.json_format import MessageToJson, Parse
json_string = MessageToJson(person)
person = Parse(json_string, addressbook_pb2.Person())

# To dict
from google.protobuf.json_format import MessageToDict
dict_obj = MessageToDict(person)
```

### Field Presence

```python
# Check if message field is set
if person.HasField("last_updated"):
    print(person.last_updated)

# For optional scalar fields (proto3 syntax)
if person.HasField("optional_field"):
    print("Field is set")

# Clear a field
person.ClearField("email")

# Check repeated field length
if len(person.phones) > 0:
    print(person.phones[0].number)
```

### Repeated Fields

```python
# Add elements
phone = person.phones.add()
phone.number = "555-1234"

# Or append
phone = addressbook_pb2.Person.PhoneNumber()
phone.number = "555-1234"
person.phones.append(phone)

# Extend with multiple
person.phones.extend([phone1, phone2])

# Iterate
for phone in person.phones:
    print(phone.number)

# Access by index
first_phone = person.phones[0]

# Length
count = len(person.phones)
```

## Next Steps

- [API Reference](api-reference.md) - Complete API documentation
- [Best Practices](best-practices.md) - Optimization tips
