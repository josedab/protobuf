# Java Guide

The Java implementation of Protocol Buffers provides a full-featured, type-safe API with excellent tooling support.

## Overview

The Java protobuf library offers:

- **Builder pattern** - Immutable messages with builders
- **Type safety** - Compile-time type checking
- **Full reflection** - Runtime metadata and dynamic messages
- **Lite runtime** - Smaller footprint for Android

## Getting Started

1. [Installation](installation.md) - Add protobuf to your project
2. [Quick Start](quickstart.md) - Create your first Java protobuf program
3. [API Reference](api-reference.md) - Detailed API documentation
4. [Best Practices](best-practices.md) - Optimization and patterns

## Quick Example

### Define a Message

```protobuf
// person.proto
syntax = "proto3";

option java_package = "com.example.proto";
option java_outer_classname = "PersonProto";

message Person {
  string name = 1;
  int32 id = 2;
  string email = 3;
}
```

### Generate Code

```bash
protoc --java_out=src/main/java person.proto
```

### Use the Generated Code

```java
import com.example.proto.PersonProto.Person;
import java.io.*;

public class Main {
  public static void main(String[] args) throws Exception {
    // Create a message using builder
    Person person = Person.newBuilder()
        .setName("Alice")
        .setId(123)
        .setEmail("alice@example.com")
        .build();

    // Serialize to file
    try (FileOutputStream output = new FileOutputStream("person.bin")) {
      person.writeTo(output);
    }

    // Deserialize from file
    Person loaded;
    try (FileInputStream input = new FileInputStream("person.bin")) {
      loaded = Person.parseFrom(input);
    }

    System.out.println("Name: " + loaded.getName());
    System.out.println("ID: " + loaded.getId());
  }
}
```

## Key Features

### Immutable Messages

Messages are immutable once built:

```java
// Create with builder
Person person = Person.newBuilder()
    .setName("Alice")
    .setId(123)
    .build();

// Modify by creating a new builder from existing
Person updated = person.toBuilder()
    .setEmail("newemail@example.com")
    .build();
```

### Repeated Fields

```java
// Add elements
AddressBook book = AddressBook.newBuilder()
    .addPeople(person1)
    .addPeople(person2)
    .addAllPeople(personList)
    .build();

// Access elements
int count = book.getPeopleCount();
Person first = book.getPeople(0);
List<Person> all = book.getPeopleList();
```

### Oneof Fields

```java
// Check which field is set
Content.DataCase dataCase = content.getDataCase();
switch (dataCase) {
  case TEXT:
    String text = content.getText();
    break;
  case BINARY:
    ByteString binary = content.getBinary();
    break;
  case DATA_NOT_SET:
    // No value set
    break;
}
```

## Build Systems

### Maven

```xml
<dependency>
  <groupId>com.google.protobuf</groupId>
  <artifactId>protobuf-java</artifactId>
  <version>3.25.1</version>
</dependency>

<!-- For protoc plugin -->
<plugin>
  <groupId>org.xolstice.maven.plugins</groupId>
  <artifactId>protobuf-maven-plugin</artifactId>
  <version>0.6.1</version>
</plugin>
```

### Gradle

```groovy
plugins {
  id 'com.google.protobuf' version '0.9.4'
}

dependencies {
  implementation 'com.google.protobuf:protobuf-java:3.25.1'
}

protobuf {
  protoc {
    artifact = 'com.google.protobuf:protoc:3.25.1'
  }
}
```

### Bazel

```python
java_proto_library(
    name = "person_java_proto",
    deps = [":person_proto"],
)
```

## Lite Runtime

For Android and resource-constrained environments:

```protobuf
option optimize_for = LITE_RUNTIME;
```

```xml
<dependency>
  <groupId>com.google.protobuf</groupId>
  <artifactId>protobuf-javalite</artifactId>
  <version>3.25.1</version>
</dependency>
```

## Resources

- [Java API Reference](https://protobuf.dev/reference/java/api-docs/)
- [Java Tutorial](https://protobuf.dev/getting-started/javatutorial/)
