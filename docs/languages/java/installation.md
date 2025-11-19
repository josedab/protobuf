# Java Installation

This guide covers adding Protocol Buffers to your Java project.

## Runtime Library

### Maven

Add to your `pom.xml`:

```xml
<dependencies>
  <dependency>
    <groupId>com.google.protobuf</groupId>
    <artifactId>protobuf-java</artifactId>
    <version>3.25.1</version>
  </dependency>
</dependencies>
```

### Gradle

Add to your `build.gradle`:

```groovy
dependencies {
  implementation 'com.google.protobuf:protobuf-java:3.25.1'
}
```

For Kotlin DSL (`build.gradle.kts`):

```kotlin
dependencies {
  implementation("com.google.protobuf:protobuf-java:3.25.1")
}
```

## Code Generation

### Maven Plugin

```xml
<build>
  <plugins>
    <plugin>
      <groupId>org.xolstice.maven.plugins</groupId>
      <artifactId>protobuf-maven-plugin</artifactId>
      <version>0.6.1</version>
      <configuration>
        <protocArtifact>
          com.google.protobuf:protoc:3.25.1:exe:${os.detected.classifier}
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

  <extensions>
    <extension>
      <groupId>kr.motd.maven</groupId>
      <artifactId>os-maven-plugin</artifactId>
      <version>1.7.1</version>
    </extension>
  </extensions>
</build>
```

Place `.proto` files in `src/main/proto/`.

### Gradle Plugin

```groovy
plugins {
  id 'java'
  id 'com.google.protobuf' version '0.9.4'
}

protobuf {
  protoc {
    artifact = 'com.google.protobuf:protoc:3.25.1'
  }
}

sourceSets {
  main {
    proto {
      srcDir 'src/main/proto'
    }
  }
}
```

Place `.proto` files in `src/main/proto/`.

## Android Projects

### Add Dependencies

```groovy
// build.gradle (app)
plugins {
  id 'com.android.application'
  id 'com.google.protobuf' version '0.9.4'
}

dependencies {
  implementation 'com.google.protobuf:protobuf-javalite:3.25.1'
}

protobuf {
  protoc {
    artifact = 'com.google.protobuf:protoc:3.25.1'
  }
  generateProtoTasks {
    all().each { task ->
      task.builtins {
        java {
          option "lite"
        }
      }
    }
  }
}
```

### Use Lite Runtime

```protobuf
option optimize_for = LITE_RUNTIME;
```

## Manual Installation

### Install protoc

See [Getting Started Installation](../../getting-started/installation.md).

### Generate Code Manually

```bash
protoc --java_out=src/main/java path/to/message.proto
```

## Proto File Options

Configure Java code generation:

```protobuf
syntax = "proto3";

// Package for generated Java classes
option java_package = "com.example.myproject";

// Outer class name (contains all messages)
option java_outer_classname = "MyProtos";

// Generate separate files per message
option java_multiple_files = true;
```

## Verify Installation

Create `test.proto`:

```protobuf
syntax = "proto3";
option java_package = "com.example";

message Test {
  string name = 1;
}
```

Generate and compile:

```bash
protoc --java_out=. test.proto
javac -cp protobuf-java-3.25.1.jar com/example/Test.java
```

## Troubleshooting

### Class Not Found

Ensure protobuf-java is in your classpath:

```bash
java -cp ".:protobuf-java-3.25.1.jar" com.example.Main
```

### Version Mismatch

Ensure protoc version matches library version:

```bash
protoc --version  # Should match library version
```

### Proto File Not Found

Check import paths:

```bash
protoc --proto_path=src/main/proto --java_out=src/main/java myfile.proto
```

## Next Steps

- [Quick Start](quickstart.md) - Create your first program
- [API Reference](api-reference.md) - Detailed API docs
