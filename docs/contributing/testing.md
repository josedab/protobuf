# Testing Guide

This guide covers writing and running tests for Protocol Buffers.

## Running Tests

### C++ Tests (CMake)

```bash
# Build with tests
cmake -B build -Dprotobuf_BUILD_TESTS=ON
cmake --build build --parallel

# Run all tests
cd build && ctest --output-on-failure

# Run specific test
./tests/protobuf-test --gtest_filter="*MessageTest*"

# Run with verbose output
ctest -V
```

### C++ Tests (Bazel)

```bash
# Run all tests
bazel test //...

# Run specific test
bazel test //src/google/protobuf:message_test
```

### Java Tests

```bash
cd java
mvn test

# Run specific test
mvn test -Dtest=MessageTest
```

### Python Tests

```bash
cd python
python -m pytest

# Run specific test
python -m pytest google/protobuf/internal/message_test.py

# With coverage
python -m pytest --cov=google.protobuf
```

## Writing Tests

### C++ Tests

Use Google Test framework:

```cpp
#include <gtest/gtest.h>
#include "google/protobuf/message.h"

namespace google {
namespace protobuf {
namespace {

TEST(MessageTest, SerializeAndParse) {
  MyMessage original;
  original.set_name("test");

  std::string serialized;
  ASSERT_TRUE(original.SerializeToString(&serialized));

  MyMessage parsed;
  ASSERT_TRUE(parsed.ParseFromString(serialized));

  EXPECT_EQ(original.name(), parsed.name());
}

TEST(MessageTest, DefaultValues) {
  MyMessage message;
  EXPECT_EQ("", message.name());
  EXPECT_EQ(0, message.id());
}

class MessageFixtureTest : public testing::Test {
 protected:
  void SetUp() override {
    message_.set_name("fixture");
  }

  MyMessage message_;
};

TEST_F(MessageFixtureTest, UsesFixture) {
  EXPECT_EQ("fixture", message_.name());
}

}  // namespace
}  // namespace protobuf
}  // namespace google
```

### Java Tests

Use JUnit:

```java
import static org.junit.Assert.*;
import org.junit.Test;
import org.junit.Before;

public class MessageTest {

  private MyMessage message;

  @Before
  public void setUp() {
    message = MyMessage.newBuilder()
        .setName("test")
        .build();
  }

  @Test
  public void testSerializeAndParse() throws Exception {
    byte[] serialized = message.toByteArray();
    MyMessage parsed = MyMessage.parseFrom(serialized);

    assertEquals(message.getName(), parsed.getName());
  }

  @Test
  public void testDefaultValues() {
    MyMessage empty = MyMessage.getDefaultInstance();

    assertEquals("", empty.getName());
    assertEquals(0, empty.getId());
  }
}
```

### Python Tests

Use pytest or unittest:

```python
import unittest
from google.protobuf import message_test_pb2

class MessageTest(unittest.TestCase):

    def test_serialize_and_parse(self):
        original = message_test_pb2.MyMessage()
        original.name = "test"

        serialized = original.SerializeToString()
        parsed = message_test_pb2.MyMessage()
        parsed.ParseFromString(serialized)

        self.assertEqual(original.name, parsed.name)

    def test_default_values(self):
        message = message_test_pb2.MyMessage()

        self.assertEqual("", message.name)
        self.assertEqual(0, message.id)


if __name__ == '__main__':
    unittest.main()
```

## Test Categories

### Unit Tests

Test individual components in isolation:

```cpp
TEST(EncoderTest, WriteVarint) {
  std::string output;
  io::StringOutputStream stream(&output);
  io::CodedOutputStream coded(&stream);

  coded.WriteVarint32(150);
  coded.Trim();

  EXPECT_EQ("\x96\x01", output);
}
```

### Integration Tests

Test components working together:

```cpp
TEST(MessageIntegrationTest, RoundTrip) {
  // Create complex message
  AddressBook book;
  Person* person = book.add_people();
  person->set_name("Alice");

  // Serialize
  std::string data;
  book.SerializeToString(&data);

  // Parse
  AddressBook parsed;
  parsed.ParseFromString(data);

  // Verify
  ASSERT_EQ(1, parsed.people_size());
  EXPECT_EQ("Alice", parsed.people(0).name());
}
```

### Performance Tests

Measure performance characteristics:

```cpp
TEST(MessagePerformanceTest, SerializationSpeed) {
  MyMessage message;
  // ... populate message

  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < 10000; i++) {
    std::string output;
    message.SerializeToString(&output);
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
      end - start);

  // Log but don't fail on timing
  std::cout << "Serialization: " << duration.count() << "ms" << std::endl;
}
```

## Test Protos

Create test proto files in `src/google/protobuf/`:

```protobuf
// unittest.proto
syntax = "proto3";

package protobuf_unittest;

message TestAllTypes {
  int32 optional_int32 = 1;
  string optional_string = 14;
  repeated int32 repeated_int32 = 31;
  // ...
}
```

## Test Coverage

### Check Coverage

```bash
# C++ with gcov
cmake -B build -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_CXX_FLAGS="--coverage"
cmake --build build
cd build && ctest
gcov src/google/protobuf/*.cc

# Python with coverage
cd python
pip install pytest-cov
pytest --cov=google.protobuf --cov-report=html
```

### Coverage Goals

- Aim for >80% line coverage
- Cover all public APIs
- Include edge cases

## Best Practices

### Test Structure

```cpp
TEST(Category, WhatIsBeingTested_ExpectedBehavior) {
  // Arrange
  MyMessage message;
  message.set_name("test");

  // Act
  std::string result;
  bool success = message.SerializeToString(&result);

  // Assert
  ASSERT_TRUE(success);
  EXPECT_FALSE(result.empty());
}
```

### Test Independence

Tests should not depend on each other:

```cpp
// Good: each test creates its own data
TEST(MessageTest, Test1) {
  MyMessage msg;
  // ...
}

TEST(MessageTest, Test2) {
  MyMessage msg;
  // ...
}
```

### Test Naming

Use descriptive names:

```cpp
// Good
TEST(MessageTest, ParseFromString_ReturnsTrue_WhenValidInput)
TEST(MessageTest, ParseFromString_ReturnsFalse_WhenInvalidInput)

// Bad
TEST(MessageTest, Test1)
TEST(MessageTest, TestParsing)
```

## Troubleshooting

### Flaky Tests

1. Avoid timing dependencies
2. Don't rely on execution order
3. Clean up resources properly

### Debugging Tests

```bash
# Run with gdb
gdb ./tests/protobuf-test
(gdb) run --gtest_filter="*FailingTest*"

# Verbose output
./tests/protobuf-test --gtest_filter="*Test*" --gtest_print_time=1
```

## See Also

- [Development Setup](setup.md)
- [Code Style](style.md)
- [Pull Request Process](pull-requests.md)
