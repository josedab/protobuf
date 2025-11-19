# Code Style

Follow these style guidelines when contributing to Protocol Buffers.

## General Principles

- **Consistency** - Match surrounding code style
- **Clarity** - Write readable, understandable code
- **Simplicity** - Prefer simple solutions

## C++ Style

We follow [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html) with some exceptions.

### Naming

```cpp
// Classes: PascalCase
class MessageFactory {
  // Methods: PascalCase
  void CreateMessage();

  // Member variables: snake_case with trailing underscore
  int message_count_;

  // Constants: kPascalCase
  static const int kMaxSize = 100;
};

// Functions: PascalCase
void ProcessMessage(const Message& msg);

// Variables: snake_case
int message_count = 0;

// Namespaces: lowercase
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google
```

### Formatting

```cpp
// Braces on same line
if (condition) {
  // ...
} else {
  // ...
}

// Two-space indentation
void Function() {
  if (condition) {
    DoSomething();
  }
}

// Line length: 80 characters
```

### Headers

```cpp
// Include guards
#ifndef GOOGLE_PROTOBUF_MESSAGE_H__
#define GOOGLE_PROTOBUF_MESSAGE_H__

// Includes in order:
// 1. Related header
// 2. C system headers
// 3. C++ standard library
// 4. Other libraries
// 5. Project headers

#include <memory>
#include <string>

#include "google/protobuf/message_lite.h"

#endif  // GOOGLE_PROTOBUF_MESSAGE_H__
```

### Comments

```cpp
// Single-line comments with two spaces before //
int value;  // Comment here

// Doxygen for public API
/// Brief description.
///
/// Detailed description.
///
/// @param input The input value.
/// @return The result.
int Calculate(int input);
```

### Tools

Format with clang-format:

```bash
clang-format -i file.cc
```

## Java Style

Follow [Google Java Style Guide](https://google.github.io/styleguide/javaguide.html).

### Naming

```java
// Classes: PascalCase
public class MessageFactory {
  // Methods: camelCase
  public void createMessage() {}

  // Fields: camelCase
  private int messageCount;

  // Constants: SCREAMING_SNAKE_CASE
  public static final int MAX_SIZE = 100;
}
```

### Formatting

```java
// Two-space indentation
public void method() {
  if (condition) {
    doSomething();
  }
}

// 100 character line length
```

## Python Style

Follow [Google Python Style Guide](https://google.github.io/styleguide/pyguide.html) and [PEP 8](https://pep8.org/).

### Naming

```python
# Modules: snake_case
import my_module

# Classes: PascalCase
class MessageFactory:
    # Methods: snake_case
    def create_message(self):
        pass

    # Constants: SCREAMING_SNAKE_CASE
    MAX_SIZE = 100
```

### Formatting

```python
# Four-space indentation
def function():
    if condition:
        do_something()

# 80 character line length
```

### Docstrings

```python
def function(arg1: int, arg2: str) -> bool:
    """Brief description.

    Longer description if needed.

    Args:
        arg1: Description of arg1.
        arg2: Description of arg2.

    Returns:
        Description of return value.

    Raises:
        ValueError: If arg1 is negative.
    """
    pass
```

## Proto Style

Follow [Protocol Buffers Style Guide](https://protobuf.dev/programming-guides/style/).

### Naming

```protobuf
// Messages: PascalCase
message SearchRequest {
  // Fields: snake_case
  string query_string = 1;
  int32 page_number = 2;
}

// Enums: PascalCase
enum Status {
  // Values: SCREAMING_SNAKE_CASE with prefix
  STATUS_UNSPECIFIED = 0;
  STATUS_ACTIVE = 1;
}

// Services: PascalCase
service SearchService {
  // RPCs: PascalCase
  rpc Search(SearchRequest) returns (SearchResponse);
}
```

### Formatting

```protobuf
// Two-space indentation
message Example {
  string name = 1;
  int32 id = 2;

  message Nested {
    string value = 1;
  }
}

// Group related fields
message Order {
  // Identification
  string order_id = 1;
  string customer_id = 2;

  // Details
  repeated LineItem items = 3;
  int64 total_cents = 4;
}
```

## Commit Messages

```
component: Brief description (50 chars max)

More detailed explanation. Wrap at 72 characters.

- Use bullet points for lists
- Explain what and why, not how

Fixes #123
```

Components:
- `cpp` - C++ runtime
- `java` - Java runtime
- `python` - Python runtime
- `compiler` - protoc compiler
- `docs` - Documentation
- `build` - Build system

## Code Review

Expect feedback on:
- Style compliance
- Code correctness
- Test coverage
- Documentation

Respond constructively to all feedback.

## See Also

- [Testing Guide](testing.md)
- [Pull Request Process](pull-requests.md)
