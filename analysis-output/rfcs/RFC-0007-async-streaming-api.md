# RFC-0007: Async/Streaming API

**Status:** Draft
**Author:** Codebase Analysis
**Created:** November 19, 2025
**Effort:** 8 weeks
**Category:** Long-term

---

## Summary

Add first-class support for asynchronous parsing/serialization and streaming message processing to the Protocol Buffers runtime, enabling efficient handling of large messages and integration with modern async frameworks.

## Motivation

### Problem Statement

Current API is synchronous and blocking:

1. **Large messages block** - Parsing 100MB message blocks thread
2. **No streaming** - Must have entire message in memory
3. **Poor async integration** - Doesn't work with async/await
4. **Memory pressure** - Can't process messages incrementally

### Modern Requirements

- Async/await is standard in modern code
- Streaming data processing is common
- Memory-constrained environments
- High-throughput systems

### Use Cases

1. **Large file processing** - Parse 1GB+ proto files incrementally
2. **Network streaming** - Process messages as bytes arrive
3. **Async services** - Non-blocking gRPC handlers
4. **Memory-limited** - Parse without loading entire message

## Detailed Design

### Core Concepts

```
AsyncParser
├── parseAsync(stream) → Future<Message>
├── parseStream(stream) → Stream<Message>
└── parseIncremental(bytes) → Optional<Message>

AsyncSerializer
├── serializeAsync(message) → Future<bytes>
├── serializeStream(messages) → Stream<bytes>
└── serializeIncremental(message) → Stream<bytes>
```

### C++ Implementation

#### Async Parsing

```cpp
// New async API
#include "google/protobuf/async.h"

class AsyncParser {
 public:
  // Parse with future result
  template <typename Message>
  std::future<Message> ParseAsync(
      io::ZeroCopyInputStream* input);

  // Parse incrementally
  template <typename Message>
  absl::StatusOr<Message> ParseIncremental(
      absl::Span<const char> bytes,
      ParseState* state);
};

// Usage
std::future<Person> future = parser.ParseAsync<Person>(input);
// Do other work...
Person person = future.get();
```

#### Streaming Parser

```cpp
// Stream of messages
template <typename Message>
class MessageStream {
 public:
  // Get next message (non-blocking if possible)
  absl::StatusOr<Message> Next();

  // Check if more messages available
  bool HasNext() const;

  // Async iteration
  void ForEach(std::function<void(Message)> callback);
};

// Usage
MessageStream<LogEntry> stream = parser.ParseStream<LogEntry>(input);
while (stream.HasNext()) {
  LogEntry entry = stream.Next().value();
  ProcessEntry(entry);
}
```

#### Incremental Parser

```cpp
// State machine for incremental parsing
class IncrementalParser {
 public:
  enum class State {
    kNeedMoreData,
    kMessageReady,
    kError,
    kDone
  };

  // Feed bytes incrementally
  State Feed(absl::Span<const char> bytes);

  // Get parsed message when ready
  template <typename Message>
  Message TakeMessage();

  // Bytes consumed
  size_t BytesConsumed() const;
};

// Usage with network stream
IncrementalParser parser;
while (true) {
  bytes = network.Read();
  auto state = parser.Feed(bytes);

  if (state == IncrementalParser::State::kMessageReady) {
    auto msg = parser.TakeMessage<Person>();
    ProcessMessage(msg);
  }
}
```

### Java Implementation

```java
// CompletableFuture API
public class AsyncParser {
    public <T extends Message> CompletableFuture<T> parseAsync(
            InputStream input, Parser<T> parser) {
        return CompletableFuture.supplyAsync(() -> {
            try {
                return parser.parseFrom(input);
            } catch (IOException e) {
                throw new CompletionException(e);
            }
        });
    }
}

// Reactive Streams support
public class ReactiveParser {
    public <T extends Message> Publisher<T> parseStream(
            Publisher<ByteBuffer> input, Parser<T> parser) {
        // Return Flux/Flow of parsed messages
    }
}

// Usage
CompletableFuture<Person> future = asyncParser.parseAsync(input, Person.parser());
future.thenAccept(person -> processPerson(person));
```

### Python Implementation

```python
# Async/await support
import asyncio
from google.protobuf import async_parser

async def parse_async(data: bytes, message_type) -> Message:
    """Parse message asynchronously."""
    parser = async_parser.AsyncParser()
    return await parser.parse(data, message_type)

# Async generator for streaming
async def parse_stream(stream, message_type):
    """Yield messages from async stream."""
    parser = async_parser.IncrementalParser()
    async for chunk in stream:
        parser.feed(chunk)
        while parser.has_message():
            yield parser.take_message(message_type)

# Usage
async def main():
    # Single message
    person = await parse_async(data, Person)

    # Stream of messages
    async for entry in parse_stream(network_stream, LogEntry):
        await process_entry(entry)
```

### Serialization Streaming

```cpp
// Serialize large message in chunks
class StreamingSerializer {
 public:
  // Serialize to output stream
  void SerializeStreaming(
      const Message& message,
      io::ZeroCopyOutputStream* output,
      size_t chunk_size = 64 * 1024);

  // Get serialization as stream of chunks
  std::generator<std::string> SerializeChunks(
      const Message& message,
      size_t chunk_size = 64 * 1024);
};

// Usage
for (auto chunk : serializer.SerializeChunks(large_message)) {
  network.Send(chunk);
}
```

## Example Usage

### Large File Processing

```cpp
// Parse 1GB proto file without loading entirely
std::ifstream file("large.pb", std::ios::binary);
MessageStream<DataRecord> stream =
    parser.ParseStream<DataRecord>(&file);

size_t count = 0;
stream.ForEach([&](DataRecord record) {
  ProcessRecord(record);
  count++;
});
std::cout << "Processed " << count << " records" << std::endl;
```

### Network Streaming

```cpp
// Process messages as they arrive
TcpConnection conn = ConnectToServer();
IncrementalParser parser;

conn.OnData([&](absl::Span<char> bytes) {
  parser.Feed(bytes);

  while (parser.State() == IncrementalParser::State::kMessageReady) {
    auto msg = parser.TakeMessage<Event>();
    HandleEvent(msg);
  }
});
```

### Async gRPC Handler

```java
// Non-blocking service implementation
@Override
public CompletableFuture<Response> handleRequest(Request request) {
    return asyncParser.parseAsync(request.getData(), InnerMessage.parser())
        .thenCompose(inner -> processAsync(inner))
        .thenApply(result -> Response.newBuilder()
            .setResult(result)
            .build());
}
```

## Implementation Plan

### Phase 1: Core Infrastructure (Weeks 1-3)
- [ ] Design incremental parser state machine
- [ ] Implement C++ IncrementalParser
- [ ] Implement C++ MessageStream
- [ ] Add async helpers

### Phase 2: Language Support (Weeks 4-6)
- [ ] Java CompletableFuture integration
- [ ] Python async/await support
- [ ] Streaming serialization

### Phase 3: Testing & Optimization (Weeks 7-8)
- [ ] Performance benchmarks
- [ ] Memory usage tests
- [ ] Integration tests
- [ ] Documentation

## Backwards Compatibility

### Fully Compatible
- Existing sync APIs unchanged
- Async APIs are additions
- No breaking changes

### Integration Points
- Works with existing streams
- Compatible with gRPC
- Standard async patterns

## Alternatives Considered

### Alternative 1: Callback-Based Only
- **Pro:** No coroutine dependency
- **Con:** Callback hell
- **Decision:** Support both

### Alternative 2: External Async Library
- **Pro:** Less code to maintain
- **Con:** Dependency, less integration
- **Decision:** Native support preferred

### Alternative 3: Only Streaming, No Async
- **Pro:** Simpler implementation
- **Con:** Doesn't solve async integration
- **Decision:** Need both

## Open Questions

1. **Coroutine support** - C++20 coroutines or custom?
   - Suggestion: Support both std::future and coroutines

2. **Backpressure** - How to handle slow consumers?
   - Suggestion: Bounded buffers with configurable policy

3. **Cancellation** - How to cancel in-progress parsing?
   - Suggestion: Cancellation tokens

## Success Criteria

- [ ] Parse 1GB message with <10MB memory
- [ ] Non-blocking parsing in all languages
- [ ] <5% overhead vs sync for small messages
- [ ] Integration with major async frameworks
- [ ] Adopted by gRPC-protobuf

## Effort Estimation

| Task | Days |
|------|------|
| Design | 5 |
| C++ implementation | 15 |
| Java implementation | 10 |
| Python implementation | 8 |
| Testing | 10 |
| Documentation | 5 |
| **Total** | **53** (8 weeks) |

---

## References

- [C++20 Coroutines](https://en.cppreference.com/w/cpp/language/coroutines)
- [Java CompletableFuture](https://docs.oracle.com/javase/8/docs/api/java/util/concurrent/CompletableFuture.html)
- [Python asyncio](https://docs.python.org/3/library/asyncio.html)
- [Reactive Streams](https://www.reactive-streams.org/)
