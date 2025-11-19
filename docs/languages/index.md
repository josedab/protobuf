# Language Guides

Protocol Buffers support multiple programming languages. Choose your language below for specific installation instructions, API references, and best practices.

## Officially Supported Languages

### Core Languages (Built-in)

These languages have runtime support included in the main protobuf repository:

<div class="grid cards" markdown>

-   :simple-cplusplus:{ .lg .middle } **C++**

    ---

    High-performance implementation with full feature support.

    [:octicons-arrow-right-24: C++ Guide](cpp/index.md)

-   :simple-java:{ .lg .middle } **Java**

    ---

    Full-featured implementation with builder pattern API.

    [:octicons-arrow-right-24: Java Guide](java/index.md)

-   :simple-python:{ .lg .middle } **Python**

    ---

    Easy-to-use implementation with native Python syntax.

    [:octicons-arrow-right-24: Python Guide](python/index.md)

</div>

### Additional Languages

| Language | Package | Documentation |
|----------|---------|---------------|
| C# | [Google.Protobuf](https://www.nuget.org/packages/Google.Protobuf/) | [C# README](https://github.com/protocolbuffers/protobuf/tree/main/csharp) |
| Objective-C | Built-in | [Objective-C README](https://github.com/protocolbuffers/protobuf/tree/main/objectivec) |
| PHP | [google/protobuf](https://packagist.org/packages/google/protobuf) | [PHP README](https://github.com/protocolbuffers/protobuf/tree/main/php) |
| Ruby | [google-protobuf](https://rubygems.org/gems/google-protobuf) | [Ruby README](https://github.com/protocolbuffers/protobuf/tree/main/ruby) |

### Community Languages

| Language | Package | Maintainer |
|----------|---------|------------|
| Go | [google.golang.org/protobuf](https://pkg.go.dev/google.golang.org/protobuf) | Google |
| Rust | [prost](https://github.com/tokio-rs/prost) | Community |
| Swift | [swift-protobuf](https://github.com/apple/swift-protobuf) | Apple |
| Dart | [protobuf](https://pub.dev/packages/protobuf) | Google |
| JavaScript/TypeScript | [protobuf.js](https://github.com/protobufjs/protobuf.js) | Community |

## Quick Comparison

| Feature | C++ | Java | Python |
|---------|-----|------|--------|
| Performance | Fastest | Fast | Moderate |
| Memory control | Full | GC | GC |
| Reflection | Yes | Yes | Yes |
| Lite runtime | Yes | Yes | No |
| gRPC support | Yes | Yes | Yes |

## Common Patterns

### Generating Code

```bash
# C++
protoc --cpp_out=. message.proto

# Java
protoc --java_out=. message.proto

# Python
protoc --python_out=. message.proto

# Multiple outputs
protoc --cpp_out=cpp/ --java_out=java/ --python_out=python/ message.proto
```

### Using Plugins

For some languages, you need additional plugins:

```bash
# Go (requires protoc-gen-go)
protoc --go_out=. message.proto

# gRPC (requires language-specific plugin)
protoc --cpp_out=. --grpc_out=. --plugin=protoc-gen-grpc=grpc_cpp_plugin message.proto
```

## Version Compatibility

| protoc Version | C++ Standard | Java Version | Python Version |
|----------------|--------------|--------------|----------------|
| 3.21+ | C++14 | Java 8+ | Python 3.7+ |
| 4.x | C++14 | Java 8+ | Python 3.8+ |

## Choosing a Language

Consider these factors:

- **Performance critical**: C++
- **Enterprise/Android**: Java
- **Rapid development**: Python
- **Web frontend**: JavaScript/TypeScript
- **Mobile iOS**: Swift or Objective-C
- **Mobile Android**: Java or Kotlin
- **Systems programming**: C++ or Rust
- **Microservices**: Go
