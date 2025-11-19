# RFC-0008: Zero-Copy Optimization

**Status:** Draft
**Author:** Codebase Analysis
**Created:** November 19, 2025
**Effort:** 3 weeks
**Category:** Strategic

---

## Summary

Implement zero-copy parsing and serialization paths for Protocol Buffers, allowing large string and bytes fields to be accessed directly from the source buffer without memory copies, significantly improving performance for large messages.

## Motivation

### Problem Statement

Current parsing copies all data:

1. **Memory overhead** - 1MB message → 2MB+ memory (buffer + message)
2. **CPU overhead** - Copying large strings is expensive
3. **Latency** - Copy time adds to request latency
4. **GC pressure** - More allocations in Java/Python

### Performance Impact

For a message with 1MB string field:
- Current: ~500µs parse time (dominated by copy)
- Zero-copy: ~50µs parse time

### Use Cases

1. **Image/blob storage** - Proto with large bytes fields
2. **Log aggregation** - Messages with large string payloads
3. **RPC proxies** - Forward without deserializing
4. **Memory-mapped files** - Direct access to on-disk data

## Detailed Design

### Zero-Copy String View

```cpp
// New zero-copy string type
class ZeroCopyString {
 public:
  // Get string view (no copy)
  absl::string_view view() const;

  // Materialize to owned string (copies)
  std::string ToString() const;

  // Check if data is still valid
  bool IsValid() const;

 private:
  const char* data_;
  size_t size_;
  std::shared_ptr<const Buffer> buffer_;
};
```

### API Changes

```cpp
// Generated code changes
class Person : public Message {
 public:
  // Existing (copies)
  const std::string& name() const;

  // New zero-copy accessor
  absl::string_view name_view() const;

  // For bytes fields
  absl::Span<const uint8_t> avatar_bytes() const;
};
```

### Implementation Approach

#### 1. Buffer Lifetime Management

```cpp
// Shared buffer with reference counting
class ParseBuffer {
 public:
  static std::shared_ptr<ParseBuffer> Create(std::string data);
  static std::shared_ptr<ParseBuffer> Wrap(const char* data, size_t size);

  absl::string_view Data() const { return data_; }

 private:
  std::string owned_data_;
  absl::string_view data_;
};
```

#### 2. Zero-Copy Parsing

```cpp
// Parser that preserves buffer reference
class ZeroCopyParser {
 public:
  template <typename Message>
  bool Parse(std::shared_ptr<ParseBuffer> buffer, Message* message) {
    message->SetParseBuffer(buffer);
    return ParseInternal(buffer->Data(), message);
  }
};

// Generated message stores buffer reference
class Person : public Message {
 private:
  std::shared_ptr<ParseBuffer> parse_buffer_;
  size_t name_offset_;
  size_t name_size_;

 public:
  absl::string_view name_view() const {
    return parse_buffer_->Data().substr(name_offset_, name_size_);
  }
};
```

#### 3. Lazy Materialization

```cpp
// String field with lazy copy
class LazyString {
 public:
  // Fast path - return view if buffer valid
  absl::string_view view() const {
    if (buffer_ && buffer_->IsValid()) {
      return absl::string_view(buffer_->Data() + offset_, size_);
    }
    return materialized_;
  }

  // Materialize to owned string
  const std::string& str() {
    if (materialized_.empty() && buffer_) {
      materialized_ = std::string(view());
    }
    return materialized_;
  }

 private:
  std::shared_ptr<ParseBuffer> buffer_;
  size_t offset_;
  size_t size_;
  std::string materialized_;  // Lazily populated
};
```

### Generator Changes

Add option to generate zero-copy accessors:

```bash
protoc --cpp_out=zero_copy_strings=true:. person.proto
```

Generated code:

```cpp
// person.pb.h
class Person : public Message {
 public:
  // Standard accessor (always works)
  const std::string& name() const;

  // Zero-copy accessor (requires buffer)
  absl::string_view name_view() const;

  // Check if zero-copy is available
  bool has_zero_copy_name() const;

  // Set from string view (avoids copy on serialize)
  void set_name(absl::string_view value);
};
```

### Memory-Mapped File Support

```cpp
// Parse directly from memory-mapped file
class MmapBuffer : public ParseBuffer {
 public:
  static std::shared_ptr<MmapBuffer> Open(const std::string& path);

 private:
  int fd_;
  void* mapped_data_;
  size_t size_;
};

// Usage
auto buffer = MmapBuffer::Open("large_data.pb");
Person person;
ZeroCopyParser parser;
parser.Parse(buffer, &person);

// Access string directly from mmap
absl::string_view name = person.name_view();  // No copy!
```

### Java Implementation

```java
// ByteBuffer-backed strings
public class ZeroCopyMessage extends GeneratedMessage {
    private ByteBuffer buffer;

    // Zero-copy accessor
    public ByteBuffer getPayloadBuffer() {
        return buffer.slice(payloadOffset, payloadLength);
    }

    // Standard accessor (copies)
    public ByteString getPayload() {
        return ByteString.copyFrom(getPayloadBuffer());
    }
}
```

### Python Implementation

```python
# memoryview-based access
class ZeroCopyMessage:
    def __init__(self):
        self._buffer = None
        self._payload_slice = None

    @property
    def payload_view(self) -> memoryview:
        """Get zero-copy view of payload."""
        return self._buffer[self._payload_slice]

    @property
    def payload(self) -> bytes:
        """Get copy of payload."""
        return bytes(self.payload_view)
```

## Example Usage

### Large Blob Processing

```cpp
// Process images without copying
auto buffer = ParseBuffer::Create(std::move(raw_data));
ImageMessage msg;
ZeroCopyParser().Parse(buffer, &msg);

// Access image data directly (zero-copy)
absl::Span<const uint8_t> image_data = msg.image_bytes();
ProcessImage(image_data);

// Modify metadata only
msg.mutable_metadata()->set_processed(true);

// Serialize - image bytes not copied again
std::string output = msg.SerializeAsString();
```

### RPC Proxy

```cpp
// Forward message without full deserialization
void ForwardRequest(const std::string& data, Connection* conn) {
  auto buffer = ParseBuffer::Wrap(data.data(), data.size());
  Request req;
  ZeroCopyParser().Parse(buffer, &req);

  // Inspect routing field (small, copied)
  std::string route = req.route();

  // Forward original bytes for payload (zero-copy)
  conn->Send(req.payload_view());
}
```

### Memory-Mapped Database

```cpp
// Direct access to on-disk data
auto db = MmapBuffer::Open("records.pb");
RecordSet records;
ZeroCopyParser().Parse(db, &records);

// Iterate without loading into memory
for (int i = 0; i < records.record_count(); i++) {
  absl::string_view data = records.record(i).data_view();
  ProcessRecord(data);
}
```

## Implementation Plan

### Week 1: Core Infrastructure
- [ ] Implement ParseBuffer class
- [ ] Implement ZeroCopyString
- [ ] Add buffer lifetime management

### Week 2: Parser Integration
- [ ] Modify parser to track offsets
- [ ] Implement zero-copy string fields
- [ ] Add bytes field support
- [ ] Update code generator

### Week 3: Testing & Polish
- [ ] Performance benchmarks
- [ ] Memory safety tests
- [ ] Java/Python implementation
- [ ] Documentation

## Backwards Compatibility

### Fully Compatible
- Existing API unchanged
- Zero-copy accessors are additions
- Default behavior preserved

### Opt-In
- Requires `zero_copy_strings` generator option
- Must use `ZeroCopyParser`
- Standard accessors always work

## Alternatives Considered

### Alternative 1: Always Zero-Copy
- **Pro:** Maximum performance
- **Con:** Complex lifetime management
- **Decision:** Opt-in safer

### Alternative 2: Cord Integration Only
- **Pro:** Simpler implementation
- **Con:** Abseil dependency everywhere
- **Decision:** Support both

### Alternative 3: Arena-Only Zero-Copy
- **Pro:** Lifetime is clear
- **Con:** Requires arena usage
- **Decision:** Support arena and non-arena

## Open Questions

1. **Lifetime safety** - How to prevent dangling references?
   - Suggestion: Reference counting + IsValid() check

2. **Mutation handling** - What happens when message is modified?
   - Suggestion: Materialize on mutation

3. **Cross-language** - Unified API across languages?
   - Suggestion: Language-idiomatic with shared concepts

## Success Criteria

- [ ] 10x speedup for 1MB+ string fields
- [ ] <5% overhead for small messages
- [ ] No memory safety issues
- [ ] Working in C++, Java, Python
- [ ] Clear documentation

## Effort Estimation

| Task | Days |
|------|------|
| Core infrastructure | 4 |
| Parser integration | 5 |
| Generator changes | 3 |
| Testing | 3 |
| Documentation | 2 |
| **Total** | **17** (3 weeks) |

---

## References

- [Abseil Cord](https://abseil.io/docs/cpp/guides/strings#cord)
- [Cap'n Proto Zero-Copy](https://capnproto.org/)
- [FlatBuffers](https://google.github.io/flatbuffers/)
