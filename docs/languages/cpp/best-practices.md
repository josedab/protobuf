# C++ Best Practices

This guide covers optimization techniques and best practices for using Protocol Buffers in C++.

## Performance Optimization

### Arena Allocation

Arenas dramatically improve performance by reducing allocations:

```cpp
#include <google/protobuf/arena.h>

void ProcessMessages() {
  google::protobuf::Arena arena;

  // All messages allocated from arena
  for (int i = 0; i < 1000; i++) {
    MyMessage* msg = Arena::CreateMessage<MyMessage>(&arena);
    msg->set_name("value");
    ProcessMessage(*msg);
  }
  // All memory freed when arena goes out of scope
}
```

**Benefits:**

- Reduced allocation overhead
- Better cache locality
- Automatic cleanup
- No memory fragmentation

**When to use:**

- Processing many messages
- Short-lived message batches
- Performance-critical code

**Caveats:**

- Cannot use `delete` on arena messages
- Cannot transfer ownership out of arena
- Arena messages cannot outlive the arena

### Reusing Messages

Reuse message objects instead of creating new ones:

```cpp
MyMessage message;

while (has_data()) {
  message.Clear();  // Reuse buffers
  message.ParseFromString(GetData());
  Process(message);
}
```

### Serialization Optimization

#### Pre-calculate Size

```cpp
// Avoid multiple size calculations
size_t size = message.ByteSizeLong();
std::string output;
output.resize(size);
message.SerializeToArray(output.data(), size);
```

#### Use Zero-Copy Streams

```cpp
#include <google/protobuf/io/zero_copy_stream_impl.h>

// For file output
int fd = open("output.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
io::FileOutputStream file_stream(fd);
message.SerializeToZeroCopyStream(&file_stream);
```

#### Deterministic Serialization

For caching or hashing:

```cpp
std::string output;
io::StringOutputStream string_stream(&output);
io::CodedOutputStream coded_stream(&string_stream);
coded_stream.SetSerializationDeterministic(true);
message.SerializeToCodedStream(&coded_stream);
```

### Parsing Optimization

#### Lazy Parsing

Configure fields to parse on access:

```cpp
// In .proto file
message LargeMessage {
  bytes large_blob = 1 [lazy = true];
}
```

#### Limit Recursion and Size

```cpp
io::CodedInputStream coded_input(&input);
coded_input.SetRecursionLimit(50);
coded_input.SetTotalBytesLimit(64 * 1024 * 1024);  // 64 MB
message.ParseFromCodedStream(&coded_input);
```

## Memory Management

### Avoid Unnecessary Copies

```cpp
// Bad: creates copy
std::string value = message.name();

// Good: use const reference
const std::string& value = message.name();

// Move when possible
std::string data = message.SerializeAsString();
other.set_data(std::move(data));
```

### Transfer Ownership

```cpp
// Transfer nested message
NestedMessage* nested = source.release_nested();
dest.set_allocated_nested(nested);

// With repeated fields
for (auto* item : *source.mutable_items()) {
  dest.mutable_items()->AddAllocated(item);
}
source.mutable_items()->ExtractSubrange(0, source.items_size(), nullptr);
```

### String Optimization

```cpp
// Use mutable string for in-place modification
std::string* name = message.mutable_name();
name->reserve(100);
name->append("prefix_");
name->append(value);

// Avoid creating temporary strings
message.set_name(string_view.data(), string_view.size());
```

## Code Organization

### Separate Proto Dependencies

```cpp
// message_handler.h
class MessageHandler {
 public:
  // Forward declare to avoid including proto headers
  void Handle(const MyMessage& message);
};

// message_handler.cc
#include "my_message.pb.h"
void MessageHandler::Handle(const MyMessage& message) {
  // Implementation
}
```

### Use Lite Runtime for Mobile/Embedded

```protobuf
option optimize_for = LITE_RUNTIME;
```

Reduces binary size by removing:
- Reflection
- Descriptors
- Text format support

## Error Handling

### Always Check Parse Results

```cpp
if (!message.ParseFromString(data)) {
  LOG(ERROR) << "Failed to parse message";
  return Status::InvalidArgument("Parse failed");
}
```

### Validate Messages

```cpp
// Check required fields (proto2) or custom validation
if (!message.IsInitialized()) {
  LOG(ERROR) << "Missing required fields: "
             << message.InitializationErrorString();
  return false;
}

// Custom validation
if (message.timeout_ms() < 0) {
  LOG(ERROR) << "Invalid timeout value";
  return false;
}
```

### Handle Unknown Fields

```cpp
// Check for unknown fields (possible version mismatch)
if (message.GetReflection()
        ->GetUnknownFields(message)
        .field_count() > 0) {
  LOG(WARNING) << "Message contains unknown fields";
}
```

## Thread Safety

### Read-Only Access is Safe

```cpp
// Safe: concurrent reads
const MyMessage& shared_msg = GetSharedMessage();
std::thread t1([&]() { Process(shared_msg.name()); });
std::thread t2([&]() { Process(shared_msg.id()); });
```

### Writes Need Synchronization

```cpp
class ThreadSafeMessage {
 public:
  void SetName(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    message_.set_name(name);
  }

  std::string GetName() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return message_.name();
  }

 private:
  mutable std::mutex mutex_;
  MyMessage message_;
};
```

## Common Pitfalls

### Don't Store Pointers from Getters

```cpp
// Bad: pointer may be invalidated
const NestedMessage* nested = &message.nested();
message.clear_nested();
nested->field();  // Undefined behavior!

// Good: copy or check before clear
NestedMessage copy = message.nested();
```

### Don't Modify During Iteration

```cpp
// Bad: modifying during iteration
for (auto& item : *message.mutable_items()) {
  if (should_remove(item)) {
    message.mutable_items()->RemoveLast();  // Undefined!
  }
}

// Good: collect indices, then remove
std::vector<int> to_remove;
for (int i = 0; i < message.items_size(); i++) {
  if (should_remove(message.items(i))) {
    to_remove.push_back(i);
  }
}
for (int i = to_remove.size() - 1; i >= 0; i--) {
  message.mutable_items()->DeleteSubrange(to_remove[i], 1);
}
```

### Initialize Library Properly

```cpp
int main() {
  // Optional but recommended at startup
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  // ... application code ...

  // Clean up before exit
  google::protobuf::ShutdownProtobufLibrary();
  return 0;
}
```

## Schema Design Tips

### Field Number Allocation

```protobuf
message MyMessage {
  // Use 1-15 for frequently accessed fields (1 byte tag)
  string name = 1;
  int32 id = 2;

  // Use 16+ for less frequent fields (2 byte tag)
  string description = 16;
  repeated string tags = 17;
}
```

### Use Enums for Constants

```protobuf
enum Priority {
  PRIORITY_UNSPECIFIED = 0;
  PRIORITY_LOW = 1;
  PRIORITY_MEDIUM = 2;
  PRIORITY_HIGH = 3;
}
```

### Reserve Deleted Fields

```protobuf
message MyMessage {
  reserved 4, 8 to 10;
  reserved "old_field";

  string name = 1;
  // field 4 was 'deprecated_field'
}
```

## Benchmarking

Profile your protobuf code:

```cpp
#include <chrono>

void BenchmarkSerialization(const MyMessage& msg, int iterations) {
  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < iterations; i++) {
    std::string output;
    msg.SerializeToString(&output);
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
      end - start);

  std::cout << "Serialization: "
            << duration.count() / iterations << " µs/op" << std::endl;
}
```

## See Also

- [API Reference](api-reference.md)
- [Arena Allocation Guide](https://protobuf.dev/reference/cpp/arenas/)
- [Performance Tips](https://protobuf.dev/programming-guides/performance/)
