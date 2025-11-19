# Protocol Buffers Documentation

Protocol Buffers (protobuf) are Google's language-neutral, platform-neutral, extensible mechanism for serializing structured data.

## What are Protocol Buffers?

Protocol Buffers provide a language-agnostic way to define schemas for structured data. You define how you want your data to be structured once, then you can use special generated source code to easily write and read your structured data to and from a variety of data streams and using a variety of languages.

## Key Features

- **Language-neutral** - Generate code for C++, Java, Python, Go, C#, and many more
- **Platform-neutral** - Works across different systems and architectures
- **Extensible** - Add new fields without breaking existing code
- **Efficient** - Compact binary format for fast serialization
- **Type-safe** - Strong typing with schema validation

## Quick Example

Define a message in a `.proto` file:

```protobuf
syntax = "proto3";

message Person {
  string name = 1;
  int32 id = 2;
  string email = 3;
}
```

Generate code and use it:

=== "C++"

    ```cpp
    Person person;
    person.set_name("John Doe");
    person.set_id(1234);
    person.set_email("john@example.com");

    std::string output;
    person.SerializeToString(&output);
    ```

=== "Java"

    ```java
    Person person = Person.newBuilder()
        .setName("John Doe")
        .setId(1234)
        .setEmail("john@example.com")
        .build();

    byte[] output = person.toByteArray();
    ```

=== "Python"

    ```python
    person = Person()
    person.name = "John Doe"
    person.id = 1234
    person.email = "john@example.com"

    output = person.SerializeToString()
    ```

## Documentation Sections

<div class="grid cards" markdown>

-   :material-rocket-launch:{ .lg .middle } **Getting Started**

    ---

    Install Protocol Buffers and create your first message

    [:octicons-arrow-right-24: Get started](getting-started/index.md)

-   :material-book-open-variant:{ .lg .middle } **Core Concepts**

    ---

    Learn about messages, fields, wire format, and more

    [:octicons-arrow-right-24: Learn concepts](concepts/index.md)

-   :material-code-tags:{ .lg .middle } **Language Guides**

    ---

    Language-specific guides for C++, Java, Python, and more

    [:octicons-arrow-right-24: View guides](languages/index.md)

-   :material-view-grid:{ .lg .middle } **Architecture**

    ---

    Understand the codebase structure for contributors

    [:octicons-arrow-right-24: View architecture](architecture/index.md)

-   :material-api:{ .lg .middle } **API Reference**

    ---

    Detailed API documentation for all languages

    [:octicons-arrow-right-24: View API](api/index.md)

-   :material-handshake:{ .lg .middle } **Contributing**

    ---

    Guidelines for contributing to Protocol Buffers

    [:octicons-arrow-right-24: Contribute](contributing/index.md)

</div>

## Get Help

- **GitHub Issues**: [Report bugs or request features](https://github.com/protocolbuffers/protobuf/issues)
- **Stack Overflow**: [Ask questions](https://stackoverflow.com/questions/tagged/protocol-buffers)
- **Discussion Forum**: [Community discussions](https://github.com/protocolbuffers/protobuf/discussions)
