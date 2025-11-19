# Protocol Buffers Canonical API Specification

**Version:** 1.0.0
**Status:** Draft
**Last Updated:** November 19, 2025

---

## Overview

This document defines the canonical API that all Protocol Buffers language runtimes should implement. The goal is to provide a consistent developer experience across languages while respecting language-specific idioms.

## Consistency Levels

### Level 1: Naming Consistency (Non-Breaking)
Add canonical method names as aliases to existing methods.

### Level 2: Pattern Consistency (Optional)
Add optional patterns (like Builder) to languages that don't have them.

### Level 3: Semantic Consistency (Future)
Unify error handling and other semantic patterns.

---

## Canonical API Methods

### Message Operations

| Canonical Method | Description | Returns |
|-----------------|-------------|---------|
| `serialize()` | Serialize message to bytes | bytes/string |
| `serializeTo(stream)` | Serialize to output stream | bool/void |
| `parse(bytes)` | Parse message from bytes | bool/Message |
| `parseFrom(bytes)` | Static parse from bytes | Message |
| `clear()` | Clear all fields | void |
| `clone()` | Create a copy | Message |
| `isInitialized()` | Check required fields | bool |
| `getSerializedSize()` | Get serialized byte size | int |

### Field Access

| Canonical Method | Description | Returns |
|-----------------|-------------|---------|
| `get<Field>()` | Get field value | value |
| `set<Field>(value)` | Set field value | void/Builder |
| `has<Field>()` | Check if field is set | bool |
| `clear<Field>()` | Clear field value | void |

### Repeated Field Access

| Canonical Method | Description | Returns |
|-----------------|-------------|---------|
| `get<Field>Count()` | Get repeated field size | int |
| `get<Field>(index)` | Get element at index | value |
| `set<Field>(index, value)` | Set element at index | void |
| `add<Field>(value)` | Append element | void |
| `addAll<Field>(values)` | Append multiple elements | void |
| `clear<Field>()` | Clear all elements | void |

### Map Field Access

| Canonical Method | Description | Returns |
|-----------------|-------------|---------|
| `get<Field>Count()` | Get map size | int |
| `get<Field>()` | Get entire map | map |
| `get<Field>OrDefault(key, default)` | Get value with default | value |
| `contains<Field>(key)` | Check if key exists | bool |
| `put<Field>(key, value)` | Put key-value pair | void |
| `putAll<Field>(map)` | Put multiple pairs | void |
| `remove<Field>(key)` | Remove by key | void |
| `clear<Field>()` | Clear all entries | void |

### Reflection

| Canonical Method | Description | Returns |
|-----------------|-------------|---------|
| `getDescriptor()` | Get message descriptor | Descriptor |
| `getDescriptorForType()` | Get descriptor (instance) | Descriptor |
| `getField(fieldDescriptor)` | Get field by descriptor | value |
| `setField(fieldDescriptor, value)` | Set field by descriptor | void |
| `hasField(fieldDescriptor)` | Check field by descriptor | bool |
| `clearField(fieldDescriptor)` | Clear field by descriptor | void |
| `getAllFields()` | Get all set fields | map |

### Builder Operations (where applicable)

| Canonical Method | Description | Returns |
|-----------------|-------------|---------|
| `newBuilder()` | Create new builder | Builder |
| `toBuilder()` | Convert to builder | Builder |
| `build()` | Build immutable message | Message |
| `buildPartial()` | Build without validation | Message |
| `mergeFrom(message)` | Merge from another message | Builder |

---

## Language Mappings

### C++ Implementation

| Canonical | C++ Current | C++ Alias |
|-----------|-------------|-----------|
| `serialize()` | `SerializeAsString()` | `serialize()` |
| `serializeTo(stream)` | `SerializeToOstream()` | `serializeTo()` |
| `parse(bytes)` | `ParseFromString()` | `parse()` |
| `clear()` | `Clear()` | `clear()` |
| `clone()` | `New()->CopyFrom()` | `clone()` |
| `isInitialized()` | `IsInitialized()` | `isInitialized()` |
| `getSerializedSize()` | `ByteSizeLong()` | `getSerializedSize()` |
| `getName()` | `name()` | `getName()` |
| `setName(v)` | `set_name(v)` | `setName(v)` |
| `hasName()` | `has_name()` | `hasName()` |
| `clearName()` | `clear_name()` | `clearName()` |

### Java Implementation

| Canonical | Java Current | Java Alias |
|-----------|--------------|------------|
| `serialize()` | `toByteArray()` | `serialize()` |
| `serializeTo(stream)` | `writeTo()` | `serializeTo()` |
| `parse(bytes)` | N/A (static) | N/A |
| `parseFrom(bytes)` | `parseFrom()` | Already canonical |
| `clear()` | Builder only | N/A |
| `clone()` | `toBuilder().build()` | `clone()` |
| `isInitialized()` | `isInitialized()` | Already canonical |
| `getSerializedSize()` | `getSerializedSize()` | Already canonical |
| `getName()` | `getName()` | Already canonical |
| `setName(v)` | Builder.setName() | Already canonical |
| `hasName()` | `hasName()` | Already canonical |
| `clearName()` | Builder.clearName() | Already canonical |

### Python Implementation

| Canonical | Python Current | Python Alias |
|-----------|----------------|--------------|
| `serialize()` | `SerializeToString()` | `serialize()` |
| `serializeTo(stream)` | N/A | `serialize_to()` |
| `parse(bytes)` | `ParseFromString()` | `parse()` |
| `clear()` | `Clear()` | `clear()` |
| `clone()` | `CopyFrom()` + new | `clone()` |
| `is_initialized()` | `IsInitialized()` | `is_initialized()` |
| `get_serialized_size()` | `ByteSize()` | `get_serialized_size()` |
| `name` | `name` (property) | Keep property |
| `has_name()` | `HasField('name')` | `has_name()` |
| `clear_name()` | `ClearField('name')` | `clear_name()` |

---

## Implementation Guidelines

### Adding Aliases

When adding canonical aliases:

1. **Preserve existing methods** - Never remove or change existing behavior
2. **Document as canonical** - Mark new methods as the preferred API
3. **Delegate to existing** - Aliases should call existing implementations
4. **Test both paths** - Ensure aliases behave identically to originals

### Example: C++ Alias Implementation

```cpp
// In message_lite.h
class PROTOBUF_EXPORT MessageLite {
 public:
  // Canonical aliases
  std::string serialize() const { return SerializeAsString(); }
  bool parse(absl::string_view data) { return ParseFromString(data); }
  void clear() { Clear(); }
  bool isInitialized() const { return IsInitialized(); }
  size_t getSerializedSize() const { return ByteSizeLong(); }

  // Existing methods preserved
  virtual bool SerializeToString(std::string* output) const = 0;
  virtual bool ParseFromString(absl::string_view data) = 0;
  // ...
};
```

### Example: Java Alias Implementation

```java
// In MessageLite.java
public interface MessageLite extends MessageLiteOrBuilder {
  // Canonical alias
  default byte[] serialize() {
    return toByteArray();
  }

  // Existing method preserved
  byte[] toByteArray();
}
```

### Example: Python Alias Implementation

```python
# In message.py
class Message:
    def serialize(self):
        """Canonical alias for SerializeToString()."""
        return self.SerializeToString()

    def parse(self, data):
        """Canonical alias for ParseFromString()."""
        return self.ParseFromString(data)
```

---

## Deprecation Plan

### Phase 1: Addition (Current)
- Add all canonical aliases
- Document as preferred API
- No deprecation warnings

### Phase 2: Soft Deprecation (Year 2)
- Add deprecation notices to documentation
- IDEs show soft warnings
- Old methods still fully supported

### Phase 3: Hard Deprecation (Year 3+)
- Compiler warnings for old methods
- Migration tools available
- Old methods still functional

### Phase 4: Removal (Year 5+)
- Consider removal of old methods
- Requires major version bump
- Subject to community feedback

---

## Conformance Testing

All implementations must pass the canonical API conformance tests:

1. **Method existence** - All canonical methods exist
2. **Behavior equivalence** - Aliases produce identical results
3. **Performance** - Aliases have no performance penalty
4. **Documentation** - Methods are properly documented

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| 1.0.0 | 2025-11-19 | Initial specification |
