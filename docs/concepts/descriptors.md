# Descriptors and Reflection

Descriptors provide runtime metadata about Protocol Buffer types, enabling reflection and dynamic message manipulation.

## Overview

Descriptors answer questions like:

- What fields does this message have?
- What is field 3's name and type?
- What are the valid enum values?

This enables dynamic serialization, validation, debugging tools, and more.

## Descriptor Hierarchy

```
FileDescriptor
├── MessageDescriptor
│   ├── FieldDescriptor
│   ├── OneofDescriptor
│   └── MessageDescriptor (nested)
├── EnumDescriptor
│   └── EnumValueDescriptor
└── ServiceDescriptor
    └── MethodDescriptor
```

## Using Descriptors

### Getting Descriptors

=== "C++"

    ```cpp
    #include <google/protobuf/descriptor.h>

    // From a message instance
    const Descriptor* desc = message.GetDescriptor();

    // From generated code
    const Descriptor* desc = MyMessage::descriptor();

    // From the descriptor pool
    const Descriptor* desc =
        DescriptorPool::generated_pool()
            ->FindMessageTypeByName("package.MyMessage");
    ```

=== "Java"

    ```java
    import com.google.protobuf.Descriptors.*;

    // From a message instance
    Descriptor desc = message.getDescriptorForType();

    // From generated code
    Descriptor desc = MyMessage.getDescriptor();
    ```

=== "Python"

    ```python
    # From a message class
    desc = MyMessage.DESCRIPTOR

    # From a message instance
    desc = message.DESCRIPTOR
    ```

### Examining Message Structure

=== "C++"

    ```cpp
    const Descriptor* desc = MyMessage::descriptor();

    // Message info
    std::cout << "Name: " << desc->name() << std::endl;
    std::cout << "Full name: " << desc->full_name() << std::endl;

    // Iterate fields
    for (int i = 0; i < desc->field_count(); i++) {
      const FieldDescriptor* field = desc->field(i);
      std::cout << "Field " << field->number() << ": "
                << field->name() << " ("
                << field->type_name() << ")" << std::endl;
    }
    ```

=== "Java"

    ```java
    Descriptor desc = MyMessage.getDescriptor();

    // Message info
    System.out.println("Name: " + desc.getName());
    System.out.println("Full name: " + desc.getFullName());

    // Iterate fields
    for (FieldDescriptor field : desc.getFields()) {
      System.out.println("Field " + field.getNumber() + ": "
          + field.getName() + " ("
          + field.getType() + ")");
    }
    ```

=== "Python"

    ```python
    desc = MyMessage.DESCRIPTOR

    # Message info
    print(f"Name: {desc.name}")
    print(f"Full name: {desc.full_name}")

    # Iterate fields
    for field in desc.fields:
        print(f"Field {field.number}: {field.name} ({field.type})")
    ```

### Field Descriptor Properties

| Property | Description |
|----------|-------------|
| `name` | Field name |
| `number` | Field number |
| `type` | Field type enum |
| `label` | Optional/required/repeated |
| `default_value` | Default value |
| `message_type` | For message fields, the nested descriptor |
| `enum_type` | For enum fields, the enum descriptor |

## Reflection API

Reflection allows reading and writing fields dynamically.

### Reading Fields

=== "C++"

    ```cpp
    const Reflection* refl = message.GetReflection();
    const Descriptor* desc = message.GetDescriptor();

    // Get field by name
    const FieldDescriptor* field =
        desc->FindFieldByName("my_field");

    // Read based on type
    if (field->type() == FieldDescriptor::TYPE_STRING) {
      std::string value = refl->GetString(message, field);
    } else if (field->type() == FieldDescriptor::TYPE_INT32) {
      int32_t value = refl->GetInt32(message, field);
    }

    // Check if field is set
    bool has_value = refl->HasField(message, field);
    ```

=== "Java"

    ```java
    Descriptor desc = message.getDescriptorForType();

    // Get field by name
    FieldDescriptor field = desc.findFieldByName("my_field");

    // Read field value
    Object value = message.getField(field);

    // Check if field is set
    boolean hasValue = message.hasField(field);
    ```

=== "Python"

    ```python
    desc = message.DESCRIPTOR

    # Get field by name
    field = desc.fields_by_name['my_field']

    # Read field value
    value = getattr(message, field.name)

    # Check if field is set
    has_value = message.HasField(field.name)
    ```

### Writing Fields

=== "C++"

    ```cpp
    const Reflection* refl = message.GetReflection();
    const Descriptor* desc = message.GetDescriptor();

    const FieldDescriptor* field =
        desc->FindFieldByName("my_field");

    // Write based on type
    if (field->type() == FieldDescriptor::TYPE_STRING) {
      refl->SetString(&message, field, "value");
    } else if (field->type() == FieldDescriptor::TYPE_INT32) {
      refl->SetInt32(&message, field, 42);
    }

    // Clear field
    refl->ClearField(&message, field);
    ```

=== "Python"

    ```python
    # Set field value
    setattr(message, field.name, value)

    # Clear field
    message.ClearField(field.name)
    ```

### Handling Repeated Fields

=== "C++"

    ```cpp
    const Reflection* refl = message.GetReflection();
    const FieldDescriptor* field =
        desc->FindFieldByName("items");

    // Get size
    int size = refl->FieldSize(message, field);

    // Read element
    std::string item = refl->GetRepeatedString(message, field, 0);

    // Add element
    refl->AddString(&message, field, "new item");
    ```

=== "Python"

    ```python
    # Access repeated field
    items = getattr(message, 'items')

    # Iterate
    for item in items:
        print(item)

    # Add element
    items.append("new item")
    ```

## Dynamic Messages

Create messages without compiled `.proto` files.

### Using DynamicMessage (C++)

```cpp
#include <google/protobuf/dynamic_message.h>

// Parse .proto file at runtime
DiskSourceTree source_tree;
source_tree.MapPath("", "/path/to/protos");

Importer importer(&source_tree, nullptr);
const FileDescriptor* file =
    importer.Import("myfile.proto");

// Get message descriptor
const Descriptor* desc =
    file->FindMessageTypeByName("MyMessage");

// Create dynamic message
DynamicMessageFactory factory;
const Message* prototype = factory.GetPrototype(desc);
Message* message = prototype->New();

// Use reflection to set fields
const Reflection* refl = message->GetReflection();
const FieldDescriptor* field = desc->FindFieldByName("name");
refl->SetString(message, field, "dynamic value");

// Serialize
std::string output;
message->SerializeToString(&output);

delete message;
```

### Using DynamicMessage (Java)

```java
import com.google.protobuf.DynamicMessage;

// Build from descriptor
DynamicMessage.Builder builder =
    DynamicMessage.newBuilder(descriptor);

// Set fields
FieldDescriptor field =
    descriptor.findFieldByName("name");
builder.setField(field, "dynamic value");

// Build and serialize
DynamicMessage message = builder.build();
byte[] bytes = message.toByteArray();
```

### Using Dynamic Messages (Python)

```python
from google.protobuf import descriptor_pb2
from google.protobuf import descriptor_pool
from google.protobuf import message_factory

# Create descriptor from FileDescriptorProto
file_desc_proto = descriptor_pb2.FileDescriptorProto()
file_desc_proto.ParseFromString(descriptor_bytes)

pool = descriptor_pool.DescriptorPool()
pool.Add(file_desc_proto)

# Get message class
factory = message_factory.MessageFactory(pool)
message_desc = pool.FindMessageTypeByName('MyMessage')
message_class = factory.GetPrototype(message_desc)

# Create instance
message = message_class()
message.name = "dynamic value"
```

## Common Use Cases

### 1. Generic Serialization

Convert any message to JSON:

```cpp
#include <google/protobuf/util/json_util.h>

std::string json;
google::protobuf::util::MessageToJsonString(message, &json);
```

### 2. Field Validation

Check that all required fields are set:

```cpp
std::vector<std::string> missing;
for (int i = 0; i < desc->field_count(); i++) {
  const FieldDescriptor* field = desc->field(i);
  if (field->is_required() &&
      !refl->HasField(message, field)) {
    missing.push_back(field->name());
  }
}
```

### 3. Deep Comparison

Compare messages field by field:

```cpp
bool CompareMessages(const Message& a, const Message& b) {
  const Descriptor* desc = a.GetDescriptor();
  const Reflection* refl_a = a.GetReflection();
  const Reflection* refl_b = b.GetReflection();

  for (int i = 0; i < desc->field_count(); i++) {
    const FieldDescriptor* field = desc->field(i);
    // Compare field values...
  }
  return true;
}
```

### 4. Schema Registry

Load schemas dynamically for message routing:

```cpp
class SchemaRegistry {
 public:
  void RegisterSchema(const FileDescriptorProto& proto) {
    pool_.BuildFile(proto);
  }

  Message* CreateMessage(const std::string& name) {
    const Descriptor* desc =
        pool_.FindMessageTypeByName(name);
    return factory_.GetPrototype(desc)->New();
  }

 private:
  DescriptorPool pool_;
  DynamicMessageFactory factory_;
};
```

## Performance Considerations

### Descriptor Access

- **Caching**: Cache descriptor/reflection pointers; lookup is O(n)
- **Generated code is faster**: Direct field access vs reflection

### Reflection Overhead

| Operation | Relative Cost |
|-----------|---------------|
| Direct field access | 1x |
| Reflection get/set | 10-100x |
| Dynamic message | 10-100x |

!!! tip "When to use reflection"
    Use reflection for tools, debugging, and generic code. For performance-critical paths, use generated code.

## Best Practices

1. **Cache descriptors** - Don't look them up repeatedly
2. **Use generated code when possible** - Much faster than reflection
3. **Handle unknown fields** - They appear in reflection too
4. **Check field presence** - Before reading optional fields

## Further Reading

- [C++ Descriptor API](https://protobuf.dev/reference/cpp/api-docs/google.protobuf.descriptor/)
- [Java Descriptors](https://protobuf.dev/reference/java/api-docs/com/google/protobuf/Descriptors.html)
- [Python Descriptor API](https://googleapis.dev/python/protobuf/latest/google/protobuf/descriptor.html)
