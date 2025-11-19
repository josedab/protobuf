# Async/Streaming API for Protocol Buffers

This document describes the async and streaming APIs added to Protocol Buffers for efficient handling of large messages and integration with modern async frameworks.

## Overview

The async/streaming API provides:
- **Asynchronous parsing/serialization** - Non-blocking operations with futures
- **Incremental parsing** - Parse messages as bytes arrive
- **Streaming message processing** - Process multiple messages from a stream
- **Memory-efficient serialization** - Serialize large messages in chunks

## C++ API

### IncrementalParser

Parse messages incrementally as bytes become available:

```cpp
#include "google/protobuf/incremental_parser.h"

IncrementalParser parser;

// Feed bytes as they arrive
while (true) {
  auto bytes = network.Read();
  auto state = parser.Feed(bytes);

  if (state == IncrementalParser::State::kMessageReady) {
    auto msg = parser.TakeMessage<Person>();
    ProcessMessage(msg);
  } else if (state == IncrementalParser::State::kError) {
    std::cerr << parser.GetErrorMessage() << std::endl;
    break;
  }
}
```

#### Configuration Options

```cpp
IncrementalParser::Options options;
options.max_message_size = 100 * 1024 * 1024;  // 100MB max
options.length_delimited = true;  // Expect length-prefixed messages
options.initial_buffer_capacity = 8192;

IncrementalParser parser(options);
```

### AsyncParser

Parse messages asynchronously using futures:

```cpp
#include "google/protobuf/async.h"

AsyncParser parser;

// Parse from stream
std::future<Person> future = parser.ParseAsync<Person>(input_stream);

// Do other work...

Person person = future.get();
```

### MessageStream

Iterate over multiple messages from a stream:

```cpp
#include "google/protobuf/async.h"

MessageStream<LogEntry> stream = parser.ParseStream<LogEntry>(input);

// Iterate with ForEach
stream.ForEach([](LogEntry entry) {
  ProcessEntry(entry);
});

// Or manually iterate
while (stream.HasNext()) {
  auto result = stream.Next();
  if (result.ok()) {
    ProcessEntry(result.value());
  }
}
```

### StreamingSerializer

Serialize messages in chunks for streaming:

```cpp
#include "google/protobuf/streaming_serializer.h"

StreamingSerializer serializer;

// Serialize with callbacks
serializer.SerializeChunks(large_message, [](std::string chunk) {
  network.Send(chunk);
});

// Serialize with length prefix for multiple messages
serializer.SerializeWithLengthPrefix(message, output_stream);

// Serialize multiple messages
std::vector<Person> people = ...;
serializer.SerializeMessages(people.begin(), people.end(), output_stream);
```

## Java API

### AsyncParser

```java
import com.google.protobuf.AsyncParser;
import java.util.concurrent.CompletableFuture;

AsyncParser asyncParser = new AsyncParser();

// Parse from byte array
CompletableFuture<Person> future = asyncParser.parseAsync(data, Person.parser());
future.thenAccept(person -> {
    System.out.println("Parsed: " + person.getName());
});

// Parse from stream
CompletableFuture<Person> future = asyncParser.parseAsync(inputStream, Person.parser());

// Parse multiple delimited messages
asyncParser.parseStreamAsync(input, Person.parser(), person -> {
    processPerson(person);
});
```

### IncrementalParser

```java
import com.google.protobuf.IncrementalParser;

IncrementalParser<Person> parser = new IncrementalParser<>(Person.parser());

while (true) {
    byte[] bytes = network.read();
    IncrementalParser.State state = parser.feed(bytes);

    if (state == IncrementalParser.State.MESSAGE_READY) {
        Person person = parser.takeMessage();
        processPerson(person);
    }
}
```

## Python API

### Async Parsing

```python
import asyncio
from google.protobuf import async_parser
from myproto_pb2 import Person

async def main():
    # Parse a single message
    data = get_message_bytes()
    person = await async_parser.parse_async(data, Person)
    print(person.name)

asyncio.run(main())
```

### Streaming

```python
from google.protobuf import async_parser
from myproto_pb2 import LogEntry

async def process_stream(network_stream):
    async for entry in async_parser.parse_stream(network_stream, LogEntry):
        await process_entry(entry)
```

### IncrementalParser

```python
from google.protobuf import async_parser
from myproto_pb2 import Person

parser = async_parser.IncrementalParser()

while True:
    data = network.recv()
    parser.feed(data)
    while parser.has_message():
        person = parser.take_message(Person)
        process_person(person)
```

### AsyncSerializer

```python
from google.protobuf import async_parser

# Serialize with length prefix
data = await async_parser.AsyncSerializer.serialize_delimited(message)

# Write to stream
await async_parser.write_delimited_message(writer, message)
```

## Use Cases

### Large File Processing

Process large proto files without loading them entirely into memory:

```cpp
std::ifstream file("large.pb", std::ios::binary);
io::IstreamInputStream input(&file);

MessageStream<DataRecord> stream = parser.ParseStream<DataRecord>(&input);

size_t count = 0;
stream.ForEach([&](DataRecord record) {
  ProcessRecord(record);
  count++;
});

std::cout << "Processed " << count << " records" << std::endl;
```

### Network Streaming

Process messages as they arrive from the network:

```cpp
TcpConnection conn = ConnectToServer();
IncrementalParser parser;

conn.OnData([&](absl::Span<char> bytes) {
  parser.Feed(bytes);

  while (parser.HasMessage()) {
    auto msg = parser.TakeMessage<Event>();
    HandleEvent(msg);
  }
});
```

### Async Service Handler

Non-blocking service implementation:

```java
@Override
public CompletableFuture<Response> handleRequest(Request request) {
    AsyncParser asyncParser = new AsyncParser();
    return asyncParser.parseAsync(request.getData(), InnerMessage.parser())
        .thenCompose(inner -> processAsync(inner))
        .thenApply(result -> Response.newBuilder()
            .setResult(result)
            .build());
}
```

## Performance Considerations

1. **Buffer size** - Adjust `initial_buffer_capacity` based on typical message sizes
2. **Chunk size** - For serialization, larger chunks reduce overhead but increase memory
3. **Async executor** - In Java, use a custom executor for better control
4. **Memory limits** - Set `max_message_size` to prevent memory exhaustion

## Thread Safety

- `IncrementalParser` is NOT thread-safe - use one instance per stream
- `AsyncParser` is thread-safe for concurrent async operations
- `StreamingSerializer` is thread-safe

## Error Handling

All APIs provide error information:

```cpp
// C++
if (parser.GetState() == IncrementalParser::State::kError) {
  std::cerr << parser.GetErrorMessage() << std::endl;
}

// Java
if (parser.getState() == IncrementalParser.State.ERROR) {
  System.err.println(parser.getErrorMessage());
}

// Python
if parser.state == async_parser.IncrementalParser.ERROR:
    print(parser.error_message)
```

## Migration Guide

### From Sync to Async

**Before:**
```cpp
Person person;
person.ParseFromString(data);
```

**After:**
```cpp
AsyncParser parser;
auto future = parser.ParseAsync<Person>(data);
Person person = future.get();
```

### From Full Load to Streaming

**Before:**
```cpp
// Load entire file
std::string data = ReadEntireFile("data.pb");
Messages messages;
messages.ParseFromString(data);
```

**After:**
```cpp
// Stream messages
std::ifstream file("data.pb");
io::IstreamInputStream input(&file);
MessageStream<Message> stream = parser.ParseStream<Message>(&input);
stream.ForEach([](Message msg) { Process(msg); });
```
