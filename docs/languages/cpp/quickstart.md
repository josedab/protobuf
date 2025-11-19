# C++ Quick Start

This tutorial walks you through creating a complete C++ application using Protocol Buffers.

## Prerequisites

- Protocol Buffers compiler (`protoc`) installed
- C++ compiler (GCC, Clang, or MSVC)
- protobuf library installed

See [Installation](installation.md) if you need to set these up.

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

## Step 2: Generate C++ Code

```bash
protoc --cpp_out=. addressbook.proto
```

This creates:
- `addressbook.pb.h`
- `addressbook.pb.cc`

## Step 3: Write the Application

### Writing Messages

Create `write_addressbook.cc`:

```cpp
#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

#include "addressbook.pb.h"
#include <google/protobuf/util/time_util.h>

using google::protobuf::util::TimeUtil;

// Prompts user for person information and adds to AddressBook
void PromptForAddress(tutorial::Person* person) {
  std::cout << "Enter person ID number: ";
  int id;
  std::cin >> id;
  person->set_id(id);
  std::cin.ignore(256, '\n');

  std::cout << "Enter name: ";
  std::getline(std::cin, *person->mutable_name());

  std::cout << "Enter email address (blank for none): ";
  std::string email;
  std::getline(std::cin, email);
  if (!email.empty()) {
    person->set_email(email);
  }

  while (true) {
    std::cout << "Enter a phone number (or leave blank to finish): ";
    std::string number;
    std::getline(std::cin, number);
    if (number.empty()) {
      break;
    }

    tutorial::Person::PhoneNumber* phone = person->add_phones();
    phone->set_number(number);

    std::cout << "Is this a mobile, home, or work phone? ";
    std::string type;
    std::getline(std::cin, type);
    if (type == "mobile") {
      phone->set_type(tutorial::Person::PHONE_TYPE_MOBILE);
    } else if (type == "home") {
      phone->set_type(tutorial::Person::PHONE_TYPE_HOME);
    } else if (type == "work") {
      phone->set_type(tutorial::Person::PHONE_TYPE_WORK);
    } else {
      std::cout << "Unknown type. Using default." << std::endl;
    }
  }

  *person->mutable_last_updated() = TimeUtil::SecondsToTimestamp(time(NULL));
}

int main(int argc, char* argv[]) {
  // Verify version compatibility
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " ADDRESS_BOOK_FILE" << std::endl;
    return -1;
  }

  tutorial::AddressBook address_book;

  // Read existing address book if it exists
  {
    std::fstream input(argv[1], std::ios::in | std::ios::binary);
    if (input) {
      if (!address_book.ParseFromIstream(&input)) {
        std::cerr << "Failed to parse address book." << std::endl;
        return -1;
      }
    }
  }

  // Add a new person
  PromptForAddress(address_book.add_people());

  // Write the updated address book
  {
    std::fstream output(argv[1],
        std::ios::out | std::ios::trunc | std::ios::binary);
    if (!address_book.SerializeToOstream(&output)) {
      std::cerr << "Failed to write address book." << std::endl;
      return -1;
    }
  }

  // Clean up protobuf library
  google::protobuf::ShutdownProtobufLibrary();

  return 0;
}
```

### Reading Messages

Create `read_addressbook.cc`:

```cpp
#include <iostream>
#include <fstream>
#include <string>

#include "addressbook.pb.h"
#include <google/protobuf/util/time_util.h>

using google::protobuf::util::TimeUtil;

void ListPeople(const tutorial::AddressBook& address_book) {
  for (int i = 0; i < address_book.people_size(); i++) {
    const tutorial::Person& person = address_book.people(i);

    std::cout << "Person ID: " << person.id() << std::endl;
    std::cout << "  Name: " << person.name() << std::endl;

    if (!person.email().empty()) {
      std::cout << "  Email: " << person.email() << std::endl;
    }

    for (int j = 0; j < person.phones_size(); j++) {
      const tutorial::Person::PhoneNumber& phone = person.phones(j);

      switch (phone.type()) {
        case tutorial::Person::PHONE_TYPE_MOBILE:
          std::cout << "  Mobile phone: ";
          break;
        case tutorial::Person::PHONE_TYPE_HOME:
          std::cout << "  Home phone: ";
          break;
        case tutorial::Person::PHONE_TYPE_WORK:
          std::cout << "  Work phone: ";
          break;
        default:
          std::cout << "  Unknown phone: ";
          break;
      }
      std::cout << phone.number() << std::endl;
    }

    if (person.has_last_updated()) {
      std::cout << "  Updated: "
                << TimeUtil::ToString(person.last_updated())
                << std::endl;
    }
  }
}

int main(int argc, char* argv[]) {
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " ADDRESS_BOOK_FILE" << std::endl;
    return -1;
  }

  tutorial::AddressBook address_book;

  {
    std::fstream input(argv[1], std::ios::in | std::ios::binary);
    if (!input) {
      std::cerr << argv[1] << ": File not found." << std::endl;
      return -1;
    }
    if (!address_book.ParseFromIstream(&input)) {
      std::cerr << "Failed to parse address book." << std::endl;
      return -1;
    }
  }

  ListPeople(address_book);

  google::protobuf::ShutdownProtobufLibrary();

  return 0;
}
```

## Step 4: Build and Run

### Manual Build

```bash
# Compile
g++ -std=c++14 -o write_addressbook write_addressbook.cc addressbook.pb.cc \
    -lprotobuf -pthread

g++ -std=c++14 -o read_addressbook read_addressbook.cc addressbook.pb.cc \
    -lprotobuf -pthread

# Run
./write_addressbook addressbook.bin
./read_addressbook addressbook.bin
```

### CMake Build

Create `CMakeLists.txt`:

```cmake
cmake_minimum_required(VERSION 3.16)
project(addressbook)

set(CMAKE_CXX_STANDARD 14)

find_package(Protobuf REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS addressbook.proto)

add_executable(write_addressbook write_addressbook.cc ${PROTO_SRCS})
target_link_libraries(write_addressbook protobuf::libprotobuf)
target_include_directories(write_addressbook PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_executable(read_addressbook read_addressbook.cc ${PROTO_SRCS})
target_link_libraries(read_addressbook protobuf::libprotobuf)
target_include_directories(read_addressbook PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
```

Build:

```bash
cmake -B build
cmake --build build

./build/write_addressbook addressbook.bin
./build/read_addressbook addressbook.bin
```

## Key Concepts Demonstrated

### Setting Fields

```cpp
person.set_name("Alice");           // Set string
person.set_id(123);                 // Set int
*person.mutable_name() = "Alice";   // Mutable access
```

### Repeated Fields

```cpp
// Add elements
Person::PhoneNumber* phone = person.add_phones();
phone->set_number("555-1234");

// Access elements
int count = person.phones_size();
const auto& phone = person.phones(0);

// Iterate
for (const auto& phone : person.phones()) {
  std::cout << phone.number() << std::endl;
}
```

### Nested Messages

```cpp
// Get mutable pointer (creates if needed)
Person::PhoneNumber* phone = person.mutable_phones()->Add();

// Check if set
if (person.has_last_updated()) {
  // ...
}
```

### Serialization

```cpp
// To string
std::string data;
person.SerializeToString(&data);

// From string
Person person;
person.ParseFromString(data);

// To stream
person.SerializeToOstream(&output);

// From stream
person.ParseFromIstream(&input);
```

## Next Steps

- [API Reference](api-reference.md) - Complete API documentation
- [Best Practices](best-practices.md) - Performance and optimization
- [Arena Allocation](best-practices.md#arena-allocation) - Memory optimization
