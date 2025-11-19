# Java Quick Start

This tutorial walks you through creating a Java application using Protocol Buffers.

## Prerequisites

- Java 8 or later
- Protocol Buffers compiler (`protoc`)
- Maven or Gradle

## Step 1: Create Project Structure

```
myproject/
├── pom.xml
├── src/
│   ├── main/
│   │   ├── java/
│   │   └── proto/
│   │       └── addressbook.proto
│   └── test/
```

## Step 2: Define the Schema

Create `src/main/proto/addressbook.proto`:

```protobuf
syntax = "proto3";

package tutorial;

option java_package = "com.example.tutorial";
option java_outer_classname = "AddressBookProtos";
option java_multiple_files = true;

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

## Step 3: Configure Build

### Maven (pom.xml)

```xml
<?xml version="1.0" encoding="UTF-8"?>
<project>
  <modelVersion>4.0.0</modelVersion>
  <groupId>com.example</groupId>
  <artifactId>addressbook</artifactId>
  <version>1.0-SNAPSHOT</version>

  <properties>
    <maven.compiler.source>11</maven.compiler.source>
    <maven.compiler.target>11</maven.compiler.target>
    <protobuf.version>3.25.1</protobuf.version>
  </properties>

  <dependencies>
    <dependency>
      <groupId>com.google.protobuf</groupId>
      <artifactId>protobuf-java</artifactId>
      <version>${protobuf.version}</version>
    </dependency>
  </dependencies>

  <build>
    <extensions>
      <extension>
        <groupId>kr.motd.maven</groupId>
        <artifactId>os-maven-plugin</artifactId>
        <version>1.7.1</version>
      </extension>
    </extensions>
    <plugins>
      <plugin>
        <groupId>org.xolstice.maven.plugins</groupId>
        <artifactId>protobuf-maven-plugin</artifactId>
        <version>0.6.1</version>
        <configuration>
          <protocArtifact>
            com.google.protobuf:protoc:${protobuf.version}:exe:${os.detected.classifier}
          </protocArtifact>
        </configuration>
        <executions>
          <execution>
            <goals>
              <goal>compile</goal>
            </goals>
          </execution>
        </executions>
      </plugin>
    </plugins>
  </build>
</project>
```

## Step 4: Write the Application

### Writing Messages

Create `src/main/java/com/example/AddPerson.java`:

```java
package com.example;

import com.example.tutorial.AddressBook;
import com.example.tutorial.Person;
import com.google.protobuf.Timestamp;

import java.io.*;
import java.time.Instant;
import java.util.Scanner;

public class AddPerson {
  public static void main(String[] args) throws Exception {
    if (args.length != 1) {
      System.err.println("Usage: AddPerson ADDRESS_BOOK_FILE");
      System.exit(-1);
    }

    AddressBook.Builder addressBook = AddressBook.newBuilder();

    // Read existing address book if it exists
    File file = new File(args[0]);
    if (file.exists()) {
      try (FileInputStream input = new FileInputStream(file)) {
        addressBook.mergeFrom(input);
      }
    }

    // Add a new person
    addressBook.addPeople(promptForPerson());

    // Write the updated address book
    try (FileOutputStream output = new FileOutputStream(file)) {
      addressBook.build().writeTo(output);
    }
  }

  static Person promptForPerson() {
    Scanner scanner = new Scanner(System.in);
    Person.Builder person = Person.newBuilder();

    System.out.print("Enter person ID: ");
    person.setId(scanner.nextInt());
    scanner.nextLine(); // consume newline

    System.out.print("Enter name: ");
    person.setName(scanner.nextLine());

    System.out.print("Enter email (blank for none): ");
    String email = scanner.nextLine();
    if (!email.isEmpty()) {
      person.setEmail(email);
    }

    while (true) {
      System.out.print("Enter phone number (blank to finish): ");
      String number = scanner.nextLine();
      if (number.isEmpty()) {
        break;
      }

      Person.PhoneNumber.Builder phone = Person.PhoneNumber.newBuilder();
      phone.setNumber(number);

      System.out.print("Phone type (mobile/home/work): ");
      String type = scanner.nextLine();
      switch (type) {
        case "mobile":
          phone.setType(Person.PhoneType.PHONE_TYPE_MOBILE);
          break;
        case "home":
          phone.setType(Person.PhoneType.PHONE_TYPE_HOME);
          break;
        case "work":
          phone.setType(Person.PhoneType.PHONE_TYPE_WORK);
          break;
        default:
          System.out.println("Unknown type, using default.");
      }

      person.addPhones(phone);
    }

    // Set timestamp
    Instant now = Instant.now();
    person.setLastUpdated(Timestamp.newBuilder()
        .setSeconds(now.getEpochSecond())
        .setNanos(now.getNano())
        .build());

    return person.build();
  }
}
```

### Reading Messages

Create `src/main/java/com/example/ListPeople.java`:

```java
package com.example;

import com.example.tutorial.AddressBook;
import com.example.tutorial.Person;

import java.io.FileInputStream;

public class ListPeople {
  public static void main(String[] args) throws Exception {
    if (args.length != 1) {
      System.err.println("Usage: ListPeople ADDRESS_BOOK_FILE");
      System.exit(-1);
    }

    AddressBook addressBook;
    try (FileInputStream input = new FileInputStream(args[0])) {
      addressBook = AddressBook.parseFrom(input);
    }

    for (Person person : addressBook.getPeopleList()) {
      System.out.println("Person ID: " + person.getId());
      System.out.println("  Name: " + person.getName());

      if (!person.getEmail().isEmpty()) {
        System.out.println("  Email: " + person.getEmail());
      }

      for (Person.PhoneNumber phone : person.getPhonesList()) {
        switch (phone.getType()) {
          case PHONE_TYPE_MOBILE:
            System.out.print("  Mobile: ");
            break;
          case PHONE_TYPE_HOME:
            System.out.print("  Home: ");
            break;
          case PHONE_TYPE_WORK:
            System.out.print("  Work: ");
            break;
          default:
            System.out.print("  Phone: ");
        }
        System.out.println(phone.getNumber());
      }

      if (person.hasLastUpdated()) {
        System.out.println("  Updated: " + person.getLastUpdated());
      }
    }
  }
}
```

## Step 5: Build and Run

```bash
# Build
mvn compile

# Run
mvn exec:java -Dexec.mainClass="com.example.AddPerson" -Dexec.args="addressbook.bin"
mvn exec:java -Dexec.mainClass="com.example.ListPeople" -Dexec.args="addressbook.bin"
```

## Key Concepts

### Builder Pattern

```java
// Create message
Person person = Person.newBuilder()
    .setName("Alice")
    .setId(123)
    .build();

// Modify existing
Person updated = person.toBuilder()
    .setEmail("new@email.com")
    .build();
```

### Serialization

```java
// To bytes
byte[] bytes = person.toByteArray();

// From bytes
Person loaded = Person.parseFrom(bytes);

// To stream
person.writeTo(outputStream);

// From stream
Person loaded = Person.parseFrom(inputStream);
```

### Field Access

```java
// Get values
String name = person.getName();
int id = person.getId();

// Check presence
if (person.hasLastUpdated()) { ... }

// Default values
person.getEmail();  // Returns "" if not set
person.getId();     // Returns 0 if not set
```

## Next Steps

- [API Reference](api-reference.md) - Complete API documentation
- [Best Practices](best-practices.md) - Performance tips
