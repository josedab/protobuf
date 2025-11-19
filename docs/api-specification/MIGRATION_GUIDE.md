# Migration Guide: Canonical API

This guide helps you migrate from language-specific Protocol Buffers APIs to the new canonical API aliases. The canonical API provides consistent method names across all language implementations.

## Overview

The canonical API introduces new method names that are aliases for existing methods. Your existing code will continue to work unchanged. The new methods are optional but recommended for new code and for projects that work across multiple languages.

## Benefits of Migration

1. **Consistent code** - Same method names across C++, Java, and Python
2. **Easier context switching** - Less cognitive load when switching languages
3. **Better documentation** - Unified API reference
4. **Future-proof** - Old method names may be deprecated in future versions

## Quick Reference

### Serialization

| Canonical | C++ | Java | Python |
|-----------|-----|------|--------|
| `serialize()` | `SerializeAsString()` | `toByteArray()` | `SerializeToString()` |
| `parse()` | `ParseFromString()` | N/A (use static) | `ParseFromString()` |
| `clear()` | `Clear()` | N/A (Builder) | `Clear()` |

### Other Operations

| Canonical | C++ | Java | Python |
|-----------|-----|------|--------|
| `isInitialized()` | `IsInitialized()` | `isInitialized()` | `IsInitialized()` |
| `getSerializedSize()` | `ByteSizeLong()` | `getSerializedSize()` | `ByteSize()` |
| `mergeFrom()` | `MergeFromString()` | `mergeFrom()` | `MergeFrom()` |
| `clone()` | N/A | `clone()` | `clone()` |

## Migration Examples

### C++

**Before:**
```cpp
Person person;
person.set_name("Alice");
person.set_age(30);

// Serialize
std::string data = person.SerializeAsString();

// Parse
Person other;
other.ParseFromString(data);

// Check initialization
if (other.IsInitialized()) {
  // ...
}
```

**After (using canonical API):**
```cpp
Person person;
person.set_name("Alice");
person.set_age(30);

// Serialize
std::string data = person.serialize();

// Parse
Person other;
other.parse(data);

// Check initialization
if (other.isInitialized()) {
  // ...
}
```

### Java

**Before:**
```java
Person person = Person.newBuilder()
    .setName("Alice")
    .setAge(30)
    .build();

// Serialize
byte[] data = person.toByteArray();

// Parse
Person other = Person.parseFrom(data);

// Check initialization
if (other.isInitialized()) {
  // ...
}
```

**After (using canonical API):**
```java
Person person = Person.newBuilder()
    .setName("Alice")
    .setAge(30)
    .build();

// Serialize
byte[] data = person.serialize();

// Parse (static method unchanged)
Person other = Person.parseFrom(data);

// Check initialization (already canonical)
if (other.isInitialized()) {
  // ...
}
```

### Python

**Before:**
```python
person = Person()
person.name = "Alice"
person.age = 30

# Serialize
data = person.SerializeToString()

# Parse
other = Person()
other.ParseFromString(data)

# Check initialization
if other.IsInitialized():
    # ...
```

**After (using canonical API):**
```python
person = Person()
person.name = "Alice"
person.age = 30

# Serialize
data = person.serialize()

# Parse
other = Person()
other.parse(data)

# Check initialization
if other.is_initialized():
    # ...
```

## Gradual Migration

You can migrate gradually:

1. **Start with new code** - Use canonical API for all new code
2. **Update during refactoring** - Update old code when refactoring
3. **Use linting** - Consider adding lint rules to prefer canonical methods

## Automated Migration

You can use simple search-and-replace to migrate:

### C++

```bash
# These are suggestions - review changes before applying
sed -i 's/\.SerializeAsString()/\.serialize()/g' *.cc
sed -i 's/\.ParseFromString(/\.parse(/g' *.cc
sed -i 's/\.Clear()/\.clear()/g' *.cc
sed -i 's/\.IsInitialized()/\.isInitialized()/g' *.cc
sed -i 's/\.ByteSizeLong()/\.getSerializedSize()/g' *.cc
```

### Python

```bash
# These are suggestions - review changes before applying
sed -i 's/\.SerializeToString()/\.serialize()/g' *.py
sed -i 's/\.ParseFromString(/\.parse(/g' *.py
sed -i 's/\.Clear()/\.clear()/g' *.py
sed -i 's/\.IsInitialized()/\.is_initialized()/g' *.py
sed -i 's/\.ByteSize()/\.get_serialized_size()/g' *.py
```

## Backwards Compatibility

- **All existing methods continue to work** - No breaking changes
- **Mix old and new** - You can use both styles in the same codebase
- **Same behavior** - Canonical methods are pure aliases

## Timeline

1. **Now** - Canonical API available as aliases
2. **Year 2** - Documentation will recommend canonical API
3. **Year 3+** - Old methods may show deprecation warnings
4. **Year 5+** - Old methods may be removed (major version)

## FAQ

### Do I have to migrate?

No. All existing code will continue to work. Migration is optional but recommended for new code.

### Are there performance differences?

No. Canonical methods are inline aliases that call the existing implementations directly.

### What about field accessors?

Field accessors (getters/setters) are not changed. The canonical API focuses on message-level operations.

### What about generated code?

Generated code continues to use the language-specific conventions. The canonical aliases are in the base message classes.

## Additional Resources

- [Canonical API Specification](CANONICAL_API.md)
- [API Consistency Checker Tool](../../tools/api_consistency_checker.py)
