# C++ API Reference

This page provides a reference for the Protocol Buffers C++ API.

## Generated Message API

For each message in your `.proto` file, the compiler generates a C++ class with these methods.

### Field Accessors

#### Singular Fields

```cpp
// For: string name = 1;
const std::string& name() const;           // Get value
void set_name(const std::string& value);   // Set from string
void set_name(std::string&& value);        // Set from rvalue
void set_name(const char* value);          // Set from C string
void set_name(const char* value, size_t size);
std::string* mutable_name();               // Get mutable pointer
void clear_name();                         // Clear to default

// For optional fields:
bool has_name() const;                     // Check presence

// For: int32 count = 2;
int32_t count() const;
void set_count(int32_t value);
void clear_count();
```

#### Repeated Fields

```cpp
// For: repeated string items = 1;
int items_size() const;                    // Number of elements
const std::string& items(int index) const; // Get element
std::string* mutable_items(int index);     // Get mutable element
void set_items(int index, const std::string& value);
std::string* add_items();                  // Add and return new element
void add_items(const std::string& value);  // Add with value
void clear_items();                        // Remove all elements

// Range-based iteration
const RepeatedPtrField<std::string>& items() const;
RepeatedPtrField<std::string>* mutable_items();
```

#### Nested Messages

```cpp
// For: Address address = 1;
bool has_address() const;                  // Check if set
const Address& address() const;            // Get const reference
Address* mutable_address();                // Get mutable pointer (creates if needed)
void clear_address();                      // Clear the field
Address* release_address();                // Release ownership
void set_allocated_address(Address* addr); // Take ownership
```

#### Enum Fields

```cpp
// For: Status status = 1;
Status status() const;
void set_status(Status value);
void clear_status();
```

#### Map Fields

```cpp
// For: map<string, int32> counts = 1;
const Map<std::string, int32_t>& counts() const;
Map<std::string, int32_t>* mutable_counts();
int counts_size() const;
void clear_counts();
```

#### Oneof Fields

```cpp
// For: oneof choice { string text = 1; int32 number = 2; }
ChoiceCase choice_case() const;
void clear_choice();

// Each field has normal accessors
bool has_text() const;
const std::string& text() const;
void set_text(const std::string& value);

bool has_number() const;
int32_t number() const;
void set_number(int32_t value);
```

### Message Operations

```cpp
// Copy and assignment
void CopyFrom(const Message& other);       // Deep copy
void MergeFrom(const Message& other);      // Merge fields
Message& operator=(const Message& other);

// Clearing
void Clear();                              // Clear all fields

// Comparison
bool operator==(const Message& other) const;
bool operator!=(const Message& other) const;

// Validation
bool IsInitialized() const;                // Required fields set?

// Size
size_t ByteSizeLong() const;               // Serialized size
int GetCachedSize() const;                 // Cached size
```

### Serialization

```cpp
// To string
bool SerializeToString(std::string* output) const;
bool SerializePartialToString(std::string* output) const;
std::string SerializeAsString() const;

// To byte array
bool SerializeToArray(void* data, int size) const;

// To stream
bool SerializeToOstream(std::ostream* output) const;

// To coded stream
bool SerializeToCodedStream(io::CodedOutputStream* output) const;
```

### Parsing

```cpp
// From string
bool ParseFromString(const std::string& data);
bool ParsePartialFromString(const std::string& data);

// From byte array
bool ParseFromArray(const void* data, int size);

// From stream
bool ParseFromIstream(std::istream* input);

// From coded stream
bool ParseFromCodedStream(io::CodedInputStream* input);

// From zero-copy stream
bool ParseFromZeroCopyStream(io::ZeroCopyInputStream* input);
```

### Reflection

```cpp
// Get descriptors
const Descriptor* GetDescriptor() const;
const Reflection* GetReflection() const;

// Static descriptor access
static const Descriptor* descriptor();
```

## Utility Classes

### Arena

Memory arena for efficient allocation:

```cpp
#include <google/protobuf/arena.h>

google::protobuf::Arena arena;

// Create message on arena
MyMessage* msg = Arena::CreateMessage<MyMessage>(&arena);

// All arena messages freed when arena is destroyed
// Never delete arena-allocated messages
```

Arena options:

```cpp
ArenaOptions options;
options.initial_block_size = 1024;
options.max_block_size = 8192;
Arena arena(options);
```

### CodedInputStream/CodedOutputStream

Low-level serialization:

```cpp
#include <google/protobuf/io/coded_stream.h>

// Writing
std::string output;
io::StringOutputStream string_stream(&output);
io::CodedOutputStream coded_stream(&string_stream);

coded_stream.WriteVarint32(42);
coded_stream.WriteString("hello");

// Reading
io::ArrayInputStream array_stream(data, size);
io::CodedInputStream coded_stream(&array_stream);

uint32_t value;
coded_stream.ReadVarint32(&value);
```

### MessageDifferencer

Compare messages:

```cpp
#include <google/protobuf/util/message_differencer.h>

using google::protobuf::util::MessageDifferencer;

// Simple equality
bool equal = MessageDifferencer::Equals(msg1, msg2);

// Detailed comparison
MessageDifferencer diff;
diff.ReportDifferencesToString(&differences);
bool equal = diff.Compare(msg1, msg2);
```

### JSON Conversion

```cpp
#include <google/protobuf/util/json_util.h>

using google::protobuf::util::JsonStringToMessage;
using google::protobuf::util::MessageToJsonString;

// To JSON
std::string json;
MessageToJsonString(message, &json);

// From JSON
MyMessage message;
JsonStringToMessage(json, &message);

// With options
JsonPrintOptions options;
options.add_whitespace = true;
options.preserve_proto_field_names = true;
MessageToJsonString(message, &json, options);
```

### TextFormat

Human-readable text format:

```cpp
#include <google/protobuf/text_format.h>

// To text
std::string text;
TextFormat::PrintToString(message, &text);

// From text
MyMessage message;
TextFormat::ParseFromString(text, &message);
```

## Common Patterns

### Error Handling

```cpp
if (!message.ParseFromString(data)) {
  // Handle parse error
  LOG(ERROR) << "Failed to parse message";
  return false;
}

if (!message.IsInitialized()) {
  // Required fields missing
  LOG(ERROR) << "Message not initialized: "
             << message.InitializationErrorString();
  return false;
}
```

### Thread Safety

Messages are not thread-safe for writes:

```cpp
// Bad: concurrent modification
std::thread t1([&]() { message.set_name("a"); });
std::thread t2([&]() { message.set_name("b"); });

// Good: lock or use separate messages
std::mutex mutex;
std::thread t1([&]() {
  std::lock_guard<std::mutex> lock(mutex);
  message.set_name("a");
});
```

### Memory Management

```cpp
// Avoid copies with move semantics
std::string data = message.SerializeAsString();
OtherMessage other;
other.set_data(std::move(data));

// Release ownership instead of copying
NestedMessage* nested = message.release_nested();
other_message.set_allocated_nested(nested);
```

## See Also

- [Best Practices](best-practices.md)
- [Full C++ API Documentation](https://protobuf.dev/reference/cpp/api-docs/)
