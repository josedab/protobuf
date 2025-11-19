# Java API Reference

Reference for the Protocol Buffers Java API.

## Generated Message API

### Building Messages

```java
// Using builder
Person person = Person.newBuilder()
    .setName("Alice")
    .setId(123)
    .build();

// Modify existing
Person updated = person.toBuilder()
    .setEmail("alice@example.com")
    .build();

// Copy from another message
Person copy = Person.newBuilder(original).build();
```

### Field Accessors

```java
// Singular fields
String name = person.getName();
boolean hasEmail = person.hasEmail();  // For optional/message fields
int id = person.getId();

// Repeated fields
int count = person.getPhonesCount();
Person.PhoneNumber phone = person.getPhones(0);
List<Person.PhoneNumber> phones = person.getPhonesList();

// Map fields
Map<String, Integer> counts = message.getCountsMap();
int value = message.getCountsOrDefault("key", 0);
int value = message.getCountsOrThrow("key");
```

### Builder Setters

```java
Person.Builder builder = Person.newBuilder();

// Singular fields
builder.setName("Alice");
builder.clearName();

// Repeated fields
builder.addPhones(phone);
builder.addPhones(0, phone);  // Insert at index
builder.addAllPhones(phoneList);
builder.setPhones(0, phone);  // Replace at index
builder.removePhones(0);
builder.clearPhones();

// Nested messages
builder.setAddress(Address.newBuilder()...);
builder.mergeAddress(otherAddress);  // Merge fields

// Map fields
builder.putCounts("key", 42);
builder.putAllCounts(map);
builder.removeCounts("key");
builder.clearCounts();
```

### Oneof Fields

```java
// Check which field is set
Content.DataCase dataCase = content.getDataCase();

switch (dataCase) {
  case TEXT:
    String text = content.getText();
    break;
  case NUMBER:
    int number = content.getNumber();
    break;
  case DATA_NOT_SET:
    // Nothing set
    break;
}

// Setting one clears others
Content.Builder builder = Content.newBuilder();
builder.setText("hello");   // text is set
builder.setNumber(42);      // text is cleared
```

## Serialization

### To Bytes/Streams

```java
// To byte array
byte[] bytes = message.toByteArray();

// To stream
message.writeTo(outputStream);

// To ByteString (protobuf immutable byte array)
ByteString byteString = message.toByteString();

// Delimited (with length prefix)
message.writeDelimitedTo(outputStream);
```

### From Bytes/Streams

```java
// From byte array
Person person = Person.parseFrom(bytes);

// From stream
Person person = Person.parseFrom(inputStream);

// From ByteString
Person person = Person.parseFrom(byteString);

// Delimited
Person person = Person.parseDelimitedFrom(inputStream);
```

### Parsing Options

```java
// With extensions
ExtensionRegistry registry = ExtensionRegistry.newInstance();
MyExtensions.registerAllExtensions(registry);
Person person = Person.parseFrom(bytes, registry);

// Partial parsing (allows uninitialized messages)
Person person = Person.parsePartialFrom(bytes);
```

## Common Operations

### Size and Validation

```java
// Get serialized size
int size = message.getSerializedSize();

// Check initialization (proto2 required fields)
boolean valid = message.isInitialized();
```

### Comparison

```java
// Equality
boolean equal = message1.equals(message2);

// Hash code
int hash = message.hashCode();
```

### Copying and Merging

```java
// Deep copy
Person copy = Person.newBuilder(original).build();

// Merge (non-default fields from other overwrite)
Person.Builder builder = Person.newBuilder(base);
builder.mergeFrom(other);
```

### JSON Conversion

```java
import com.google.protobuf.util.JsonFormat;

// To JSON
String json = JsonFormat.printer().print(message);

// From JSON
Person.Builder builder = Person.newBuilder();
JsonFormat.parser().merge(json, builder);
Person person = builder.build();

// Options
JsonFormat.printer()
    .includingDefaultValueFields()
    .preservingProtoFieldNames()
    .print(message);
```

### Text Format

```java
import com.google.protobuf.TextFormat;

// To text
String text = TextFormat.printer().printToString(message);

// From text
Person.Builder builder = Person.newBuilder();
TextFormat.merge(text, builder);
```

## Descriptors and Reflection

```java
import com.google.protobuf.Descriptors.*;

// Get descriptor
Descriptor descriptor = Person.getDescriptor();

// Field information
for (FieldDescriptor field : descriptor.getFields()) {
  System.out.println(field.getName() + ": " + field.getType());
}

// Dynamic field access
FieldDescriptor field = descriptor.findFieldByName("name");
Object value = message.getField(field);
boolean hasField = message.hasField(field);

// Set via reflection
Person.Builder builder = Person.newBuilder();
builder.setField(field, "value");
```

## Dynamic Messages

```java
import com.google.protobuf.DynamicMessage;

// Create from descriptor
DynamicMessage.Builder builder = DynamicMessage.newBuilder(descriptor);
builder.setField(nameField, "Alice");
DynamicMessage message = builder.build();

// Parse binary data
DynamicMessage message = DynamicMessage.parseFrom(descriptor, bytes);
```

## ByteString

Immutable byte array type:

```java
import com.google.protobuf.ByteString;

// Create
ByteString bs = ByteString.copyFromUtf8("hello");
ByteString bs = ByteString.copyFrom(bytes);
ByteString bs = ByteString.readFrom(inputStream);

// Use
byte[] bytes = bs.toByteArray();
String utf8 = bs.toStringUtf8();
bs.writeTo(outputStream);
int size = bs.size();
byte b = bs.byteAt(0);

// Concatenate
ByteString combined = bs1.concat(bs2);
```

## Error Handling

```java
try {
  Person person = Person.parseFrom(data);
} catch (InvalidProtocolBufferException e) {
  // Handle parse error
  System.err.println("Failed to parse: " + e.getMessage());
}

// Check initialization
if (!person.isInitialized()) {
  throw new IllegalStateException("Message not initialized");
}
```

## Thread Safety

- Message instances are immutable and thread-safe
- Builders are NOT thread-safe
- Create new builders or synchronize access

## See Also

- [Best Practices](best-practices.md)
- [Java API Docs](https://protobuf.dev/reference/java/api-docs/)
