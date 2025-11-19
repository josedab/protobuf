# Java Best Practices

Best practices for using Protocol Buffers in Java applications.

## Performance

### Reuse Builders

```java
// Bad: creates new builder each time
for (Data data : dataList) {
  Person person = Person.newBuilder()
      .setName(data.name)
      .build();
}

// Good: reuse builder
Person.Builder builder = Person.newBuilder();
for (Data data : dataList) {
  builder.clear();
  Person person = builder
      .setName(data.name)
      .build();
}
```

### Use ByteString for Binary Data

```java
// Bad: byte array copying
message.getData().toByteArray();

// Good: use ByteString directly
ByteString data = message.getData();
```

### Avoid Repeated Serialization

```java
// Bad: serialize multiple times
cache.put(key1, message.toByteArray());
cache.put(key2, message.toByteArray());

// Good: serialize once
byte[] bytes = message.toByteArray();
cache.put(key1, bytes);
cache.put(key2, bytes);
```

### Pre-size Collections

```java
// When you know the size
AddressBook.Builder builder = AddressBook.newBuilder();
List<Person> people = new ArrayList<>(1000);
// ... populate list
builder.addAllPeople(people);
```

## Memory Efficiency

### Use Lite Runtime for Mobile

```protobuf
option optimize_for = LITE_RUNTIME;
```

Reduces:
- Library size
- Memory usage
- Features (no reflection/descriptors)

### Avoid Keeping References to Builders

```java
// Bad: keeping builder reference
class MyClass {
  private Person.Builder builder = Person.newBuilder();
}

// Good: create when needed
class MyClass {
  public Person createPerson() {
    return Person.newBuilder()
        .setName("test")
        .build();
  }
}
```

### Clear Repeated Fields Before Reuse

```java
builder.clearPhones();  // Release references
builder.addAllPhones(newPhones);
```

## Code Organization

### Use java_multiple_files

```protobuf
option java_multiple_files = true;
```

Generates separate files per message for:
- Better IDE support
- Cleaner imports
- Easier navigation

### Organize Proto Files

```
src/main/proto/
├── common/
│   ├── types.proto
│   └── errors.proto
├── api/
│   ├── requests.proto
│   └── responses.proto
└── internal/
    └── storage.proto
```

### Use Meaningful Package Names

```protobuf
option java_package = "com.company.service.proto";
```

## Error Handling

### Always Handle Parse Errors

```java
public Optional<Person> parsePerson(byte[] data) {
  try {
    return Optional.of(Person.parseFrom(data));
  } catch (InvalidProtocolBufferException e) {
    logger.error("Failed to parse Person", e);
    return Optional.empty();
  }
}
```

### Validate Input

```java
public void processPerson(Person person) {
  if (person.getName().isEmpty()) {
    throw new IllegalArgumentException("Name is required");
  }
  if (person.getId() <= 0) {
    throw new IllegalArgumentException("Invalid ID");
  }
  // Process...
}
```

## API Design

### Wrap Proto Types in Domain Objects

```java
public class User {
  private final Person proto;

  public User(Person proto) {
    this.proto = proto;
  }

  public String getName() {
    return proto.getName();
  }

  public Optional<String> getEmail() {
    return proto.getEmail().isEmpty() ?
        Optional.empty() :
        Optional.of(proto.getEmail());
  }

  public Person toProto() {
    return proto;
  }
}
```

### Use Factory Methods

```java
public class PersonFactory {
  public static Person create(String name, int id) {
    return Person.newBuilder()
        .setName(name)
        .setId(id)
        .build();
  }
}
```

## Testing

### Use TextFormat for Test Data

```java
@Test
void testParsing() throws Exception {
  String textProto = """
      name: "Alice"
      id: 123
      email: "alice@test.com"
      """;

  Person.Builder builder = Person.newBuilder();
  TextFormat.merge(textProto, builder);
  Person person = builder.build();

  assertEquals("Alice", person.getName());
}
```

### Compare with Equals

```java
@Test
void testEquality() {
  Person expected = Person.newBuilder()
      .setName("Alice")
      .setId(123)
      .build();

  Person actual = service.getPerson(123);

  assertEquals(expected, actual);
}
```

## Common Pitfalls

### Don't Rely on Field Defaults

```java
// Bad: unclear intent
if (person.getId() == 0) {
  // Is it unset or explicitly zero?
}

// Good: use optional fields
if (!person.hasId()) {
  // Definitely unset
}
```

### Don't Modify Immutable Messages

```java
// Compilation error - messages are immutable
person.setName("Alice");

// Correct
Person updated = person.toBuilder()
    .setName("Alice")
    .build();
```

### Handle Unknown Enum Values

```java
// Proto3 enums are open
Person.PhoneType type = phone.getType();
switch (type) {
  case PHONE_TYPE_MOBILE:
    // ...
    break;
  case UNRECOGNIZED:  // Handle unknown values
    logger.warn("Unknown phone type: " + phone.getTypeValue());
    break;
}
```

## JSON Interoperability

### Configure JSON Format

```java
JsonFormat.Printer printer = JsonFormat.printer()
    .includingDefaultValueFields()    // Include zeros/empty
    .preservingProtoFieldNames()      // Use proto names
    .omittingInsignificantWhitespace();

String json = printer.print(message);
```

### Handle JSON Parsing

```java
JsonFormat.Parser parser = JsonFormat.parser()
    .ignoringUnknownFields();  // Forward compatibility

Person.Builder builder = Person.newBuilder();
parser.merge(jsonString, builder);
```

## See Also

- [API Reference](api-reference.md)
- [Performance Guide](https://protobuf.dev/programming-guides/performance/)
