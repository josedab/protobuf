# Zero-Copy Parsing for Protocol Buffers

This module provides zero-copy access to string and bytes fields in parsed
Protocol Buffer messages, significantly improving performance for large messages.

## Overview

The zero-copy optimization allows string and bytes fields to be accessed directly
from the source buffer without memory copies. This is particularly useful for:

- Messages with large string/bytes fields (>1KB)
- High-throughput systems processing many messages
- Memory-constrained environments
- RPC proxies that forward messages without full deserialization

## Performance

For a message with a 1MB string field:
- Traditional parsing: ~500µs (dominated by copy)
- Zero-copy access: ~50µs

## API Reference

### ParseBuffer

Manages buffer lifetime for zero-copy access.

```cpp
// Create from owned data
auto buffer = ParseBuffer::Create(std::move(data));

// Wrap external data (caller ensures lifetime)
auto buffer = ParseBuffer::Wrap(ptr, size);

// Memory-mapped file
auto buffer = MmapBuffer::Open("large_file.pb");

// Access data
absl::string_view view = buffer->Data();
buffer->Substr(offset, length);
```

### ZeroCopyString

String type that can reference buffer data without copying.

```cpp
// Create from buffer reference
ZeroCopyString str(buffer, offset, length);

// Zero-copy access
absl::string_view view = str.view();

// Materialize to owned string if needed
std::string owned = str.ToString();

// Check status
bool valid = str.IsValid();
bool is_zero_copy = str.IsZeroCopy();
```

### ZeroCopyParser

Parser that enables zero-copy access to fields.

```cpp
MyMessage msg;
auto result = ParseZeroCopy(std::move(data), &msg);

if (result.ok()) {
  // Access fields via result
  absl::string_view name = result->GetStringView(1);
}
```

## Example Usage

### Large Blob Processing

```cpp
#include "google/protobuf/zero_copy/zero_copy.h"

using namespace google::protobuf::zero_copy;

// Process images without copying
auto buffer = ParseBuffer::Create(std::move(raw_data));
ImageMessage msg;
auto result = ParseZeroCopy(buffer->Data().data(), buffer->Size(), &msg);

// Access image data directly (zero-copy)
absl::string_view image_data = result->GetStringView(image_field_number);
ProcessImage(image_data);
```

### RPC Proxy

```cpp
void ForwardRequest(const std::string& data, Connection* conn) {
  auto buffer = ParseBuffer::Wrap(data.data(), data.size());
  Request req;
  auto result = ParseZeroCopyFromArray(data.data(), data.size(), &req);

  if (result.ok()) {
    // Inspect routing field (small, copied)
    std::string route = req.route();

    // Forward original bytes for payload (zero-copy)
    conn->Send(result->GetStringView(payload_field_number));
  }
}
```

### Memory-Mapped Database

```cpp
// Direct access to on-disk data
auto buffer = MmapBuffer::Open("records.pb");
if (buffer) {
  RecordSet records;
  auto result = ParseZeroCopy(buffer, &records);

  // Iterate without loading into memory
  for (int i = 0; i < records.record_count(); i++) {
    absl::string_view data = result->GetStringView(record_data_field);
    ProcessRecord(data);
  }
}
```

## Limitations

1. **Buffer lifetime**: Zero-copy strings are only valid while the ParseBuffer
   exists and hasn't been invalidated. Always check `IsValid()` before access.

2. **Field tracking**: The current implementation requires knowing field numbers
   to access zero-copy data. Future versions will integrate with generated code.

3. **Mutation**: Modifying a message may invalidate zero-copy references.
   Always materialize strings before modifying the message.

## Building

Add to your BUILD target:

```python
deps = [
    "//src/google/protobuf/zero_copy:zero_copy",
]
```

## Thread Safety

- `ParseBuffer` can be safely shared between threads (read-only access)
- `ZeroCopyString` instances sharing a buffer can be accessed concurrently
- Invalidating a buffer affects all strings referencing it

## Future Work

- Code generator integration for automatic zero-copy accessors
- Arena integration for simplified lifetime management
- Java and Python implementations
