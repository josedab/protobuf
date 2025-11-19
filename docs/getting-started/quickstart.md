# Quick Start

This tutorial walks you through creating your first Protocol Buffer message, generating code, and using it in your application.

## Overview

In this tutorial, you will:

1. Define a message schema in a `.proto` file
2. Generate code for your language
3. Use the generated code to serialize and deserialize data

## Step 1: Define Your Schema

Create a file named `addressbook.proto`:

```protobuf
syntax = "proto3";

package tutorial;

// Represents a person in the address book
message Person {
  string name = 1;
  int32 id = 2;
  string email = 3;

  // Phone number with type
  message PhoneNumber {
    string number = 1;
    PhoneType type = 2;
  }

  enum PhoneType {
    PHONE_TYPE_UNSPECIFIED = 0;
    PHONE_TYPE_MOBILE = 1;
    PHONE_TYPE_HOME = 2;
    PHONE_TYPE_WORK = 3;
  }

  repeated PhoneNumber phones = 4;
}

// The address book containing multiple people
message AddressBook {
  repeated Person people = 1;
}
```

### Understanding the Schema

- `syntax = "proto3"` - Uses proto3 syntax
- `package tutorial` - Defines the namespace
- `message Person` - Defines a message type
- Field types: `string`, `int32`, `repeated` (list)
- Field numbers (1, 2, 3...) identify fields in the binary format
- Nested messages and enums are supported

## Step 2: Generate Code

Run the Protocol Buffer compiler for your language:

=== "C++"

    ```bash
    protoc --cpp_out=. addressbook.proto
    ```

    This generates:

    - `addressbook.pb.h` - Header file with class declarations
    - `addressbook.pb.cc` - Implementation file

=== "Java"

    ```bash
    protoc --java_out=. addressbook.proto
    ```

    This generates:

    - `tutorial/AddressBookProtos.java` - Contains all message classes

=== "Python"

    ```bash
    protoc --python_out=. addressbook.proto
    ```

    This generates:

    - `addressbook_pb2.py` - Python module with message classes

## Step 3: Use the Generated Code

### Writing a Message

=== "C++"

    ```cpp
    #include "addressbook.pb.h"
    #include <fstream>
    #include <iostream>

    int main() {
      // Create a Person
      tutorial::Person person;
      person.set_name("John Doe");
      person.set_id(1234);
      person.set_email("john.doe@example.com");

      // Add a phone number
      tutorial::Person::PhoneNumber* phone = person.add_phones();
      phone->set_number("555-1234");
      phone->set_type(tutorial::Person::PHONE_TYPE_MOBILE);

      // Serialize to a file
      std::fstream output("person.bin",
          std::ios::out | std::ios::binary);
      person.SerializeToOstream(&output);

      std::cout << "Saved person: " << person.name() << std::endl;
      return 0;
    }
    ```

    Compile with:

    ```bash
    g++ -o write_person write_person.cc addressbook.pb.cc \
        -lprotobuf -pthread
    ```

=== "Java"

    ```java
    import tutorial.AddressBookProtos.Person;
    import java.io.FileOutputStream;

    public class WritePerson {
      public static void main(String[] args) throws Exception {
        // Create a Person using the builder
        Person person = Person.newBuilder()
            .setName("John Doe")
            .setId(1234)
            .setEmail("john.doe@example.com")
            .addPhones(Person.PhoneNumber.newBuilder()
                .setNumber("555-1234")
                .setType(Person.PhoneType.PHONE_TYPE_MOBILE))
            .build();

        // Serialize to a file
        try (FileOutputStream output =
            new FileOutputStream("person.bin")) {
          person.writeTo(output);
        }

        System.out.println("Saved person: " + person.getName());
      }
    }
    ```

=== "Python"

    ```python
    import addressbook_pb2

    # Create a Person
    person = addressbook_pb2.Person()
    person.name = "John Doe"
    person.id = 1234
    person.email = "john.doe@example.com"

    # Add a phone number
    phone = person.phones.add()
    phone.number = "555-1234"
    phone.type = addressbook_pb2.Person.PHONE_TYPE_MOBILE

    # Serialize to a file
    with open("person.bin", "wb") as f:
        f.write(person.SerializeToString())

    print(f"Saved person: {person.name}")
    ```

### Reading a Message

=== "C++"

    ```cpp
    #include "addressbook.pb.h"
    #include <fstream>
    #include <iostream>

    int main() {
      tutorial::Person person;

      // Read from file
      std::fstream input("person.bin",
          std::ios::in | std::ios::binary);
      person.ParseFromIstream(&input);

      // Access fields
      std::cout << "Name: " << person.name() << std::endl;
      std::cout << "ID: " << person.id() << std::endl;
      std::cout << "Email: " << person.email() << std::endl;

      for (const auto& phone : person.phones()) {
        std::cout << "Phone: " << phone.number() << std::endl;
      }

      return 0;
    }
    ```

=== "Java"

    ```java
    import tutorial.AddressBookProtos.Person;
    import java.io.FileInputStream;

    public class ReadPerson {
      public static void main(String[] args) throws Exception {
        // Read from file
        Person person;
        try (FileInputStream input =
            new FileInputStream("person.bin")) {
          person = Person.parseFrom(input);
        }

        // Access fields
        System.out.println("Name: " + person.getName());
        System.out.println("ID: " + person.getId());
        System.out.println("Email: " + person.getEmail());

        for (Person.PhoneNumber phone : person.getPhonesList()) {
          System.out.println("Phone: " + phone.getNumber());
        }
      }
    }
    ```

=== "Python"

    ```python
    import addressbook_pb2

    # Read from file
    person = addressbook_pb2.Person()
    with open("person.bin", "rb") as f:
        person.ParseFromString(f.read())

    # Access fields
    print(f"Name: {person.name}")
    print(f"ID: {person.id}")
    print(f"Email: {person.email}")

    for phone in person.phones:
        print(f"Phone: {phone.number}")
    ```

## Complete Example

Here's a complete working example that creates an address book:

=== "Python"

    ```python
    #!/usr/bin/env python3
    """Complete address book example."""

    import addressbook_pb2

    def create_address_book():
        """Create and populate an address book."""
        address_book = addressbook_pb2.AddressBook()

        # Add first person
        person1 = address_book.people.add()
        person1.name = "Alice Smith"
        person1.id = 1
        person1.email = "alice@example.com"

        phone = person1.phones.add()
        phone.number = "555-0100"
        phone.type = addressbook_pb2.Person.PHONE_TYPE_WORK

        # Add second person
        person2 = address_book.people.add()
        person2.name = "Bob Jones"
        person2.id = 2
        person2.email = "bob@example.com"

        return address_book

    def save_address_book(address_book, filename):
        """Save address book to file."""
        with open(filename, "wb") as f:
            f.write(address_book.SerializeToString())
        print(f"Saved {len(address_book.people)} people to {filename}")

    def load_address_book(filename):
        """Load address book from file."""
        address_book = addressbook_pb2.AddressBook()
        with open(filename, "rb") as f:
            address_book.ParseFromString(f.read())
        return address_book

    def print_address_book(address_book):
        """Print all people in the address book."""
        for person in address_book.people:
            print(f"\nPerson ID: {person.id}")
            print(f"  Name: {person.name}")
            if person.email:
                print(f"  Email: {person.email}")
            for phone in person.phones:
                type_name = addressbook_pb2.Person.PhoneType.Name(phone.type)
                print(f"  Phone ({type_name}): {phone.number}")

    if __name__ == "__main__":
        # Create and save
        book = create_address_book()
        save_address_book(book, "addressbook.bin")

        # Load and display
        loaded_book = load_address_book("addressbook.bin")
        print_address_book(loaded_book)
    ```

## Key Concepts

### Field Numbers

Field numbers (1, 2, 3...) are critical:

- They identify fields in the binary format
- Never change them for existing fields
- Numbers 1-15 use 1 byte (use for frequent fields)
- Numbers 16-2047 use 2 bytes

### Wire Types

Protocol Buffers use efficient binary encoding:

| Wire Type | Used For |
|-----------|----------|
| 0 | int32, int64, uint32, uint64, bool, enum |
| 1 | fixed64, sfixed64, double |
| 2 | string, bytes, embedded messages, repeated |
| 5 | fixed32, sfixed32, float |

### Default Values

Unset fields have default values:

- Numbers: `0`
- Booleans: `false`
- Strings: `""`
- Enums: First value (usually 0)
- Messages: Language-dependent (null or empty)

## Troubleshooting

### Common Errors

**"Package not found"**

Ensure the generated files are in your import path:

```python
import sys
sys.path.append('/path/to/generated/files')
```

**"Missing required fields"** (proto2)

Proto2 required fields must be set before serialization.

**"Unknown field" warnings**

The message was serialized with fields unknown to your code version. This is normal during schema evolution.

## Next Steps

You've learned the basics! Continue with:

- [Next Steps](next-steps.md) - Learn about advanced topics
- [Core Concepts](../concepts/index.md) - Deeper understanding
- [Language Guides](../languages/index.md) - Language-specific details
