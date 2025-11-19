# RFC-0006: Cross-Language API Consistency

**Status:** Draft
**Author:** Codebase Analysis
**Created:** November 19, 2025
**Effort:** 6 weeks
**Category:** Long-term

---

## Summary

Standardize and harmonize API patterns across all language runtimes (C++, Java, Python, etc.) to provide a consistent developer experience, reduce cognitive load when switching languages, and simplify documentation.

## Motivation

### Problem Statement

Each language runtime evolved independently, leading to inconsistencies:

1. **Method naming** - `SerializeToString` (C++) vs `toByteArray` (Java)
2. **Error handling** - Return values (C++) vs exceptions (Java)
3. **Mutability** - Mutable (C++) vs immutable (Java)
4. **Builder patterns** - No builder (C++) vs required builder (Java)
5. **Reflection APIs** - Different interfaces per language

### Examples of Inconsistency

| Operation | C++ | Java | Python |
|-----------|-----|------|--------|
| Serialize | `SerializeToString()` | `toByteArray()` | `SerializeToString()` |
| Parse | `ParseFromString()` | `parseFrom()` | `ParseFromString()` |
| Get field | `name()` | `getName()` | `name` |
| Set field | `set_name()` | `setName()` / Builder | `name =` |

### Benefits

1. **Reduced learning curve** - Learn once, use everywhere
2. **Simpler documentation** - Unified API reference
3. **Easier migrations** - Moving code between languages
4. **Better tooling** - Consistent patterns enable better IDE support

## Detailed Design

### Consistency Framework

Define a canonical API that all languages implement:

```
Protocol Buffers Canonical API
├── Message Operations
│   ├── serialize() / serializeTo()
│   ├── parse() / parseFrom()
│   ├── clear()
│   └── clone()
├── Field Access
│   ├── get<Field>() / <field>()
│   ├── set<Field>() / <field>=
│   ├── has<Field>()
│   └── clear<Field>()
├── Reflection
│   ├── getDescriptor()
│   ├── getField(descriptor)
│   └── setField(descriptor, value)
└── Builder (where applicable)
    ├── newBuilder()
    ├── set<Field>()
    └── build()
```

### Consistency Levels

#### Level 1: Naming Consistency (Low Breaking)

Standardize method names while keeping existing as aliases:

```cpp
// C++ - Add consistent aliases
class Message {
  // New canonical names
  std::string serialize() const { return SerializeAsString(); }
  bool parse(const std::string& data) { return ParseFromString(data); }

  // Existing names preserved
  std::string SerializeAsString() const;
  bool ParseFromString(const std::string& data);
};
```

#### Level 2: Pattern Consistency (Medium Breaking)

Add optional builder pattern to C++:

```cpp
// C++ Builder (optional pattern)
class Person::Builder {
 public:
  Builder& setName(std::string value) {
    message_.set_name(std::move(value));
    return *this;
  }

  Builder& setAge(int32_t value) {
    message_.set_age(value);
    return *this;
  }

  Person build() { return std::move(message_); }

 private:
  Person message_;
};

// Usage
Person person = Person::Builder()
    .setName("Alice")
    .setAge(30)
    .build();
```

#### Level 3: Semantic Consistency (High Breaking)

Unify error handling patterns:

```cpp
// C++ with StatusOr (like modern Google code)
absl::StatusOr<Person> person = Person::Parse(data);
if (!person.ok()) {
  LOG(ERROR) << person.status();
  return;
}
ProcessPerson(*person);
```

### API Mapping

#### Serialization

| Canonical | C++ | Java | Python |
|-----------|-----|------|--------|
| `serialize()` | `SerializeAsString()` | `toByteArray()` | `SerializeToString()` |
| `serializeTo(stream)` | `SerializeToOstream()` | `writeTo()` | `SerializeToString()` |
| `parse(bytes)` | `ParseFromString()` | `parseFrom()` | `ParseFromString()` |

**Proposal:** Add `serialize()` and `parse()` as standard methods in all languages.

#### Field Access

| Canonical | C++ | Java | Python |
|-----------|-----|------|--------|
| `getName()` | `name()` | `getName()` | `name` (property) |
| `setName(v)` | `set_name(v)` | Builder only | `name = v` |
| `hasName()` | `has_name()` | `hasName()` | `HasField('name')` |

**Proposal:** C++ gets `getName()`/`setName()` aliases.

#### Reflection

```cpp
// Unified reflection interface
class UnifiedReflection {
 public:
  virtual const Descriptor* getDescriptor() const = 0;
  virtual Value getField(const FieldDescriptor* field) const = 0;
  virtual void setField(const FieldDescriptor* field, Value value) = 0;
};
```

### Conformance Score

Measure API consistency across languages:

```python
# tools/api_consistency_checker.py

CANONICAL_API = {
    'serialize': {'cpp': 'SerializeAsString', 'java': 'toByteArray', ...},
    'parse': {'cpp': 'ParseFromString', 'java': 'parseFrom', ...},
    # ...
}

def calculate_consistency_score(language):
    matching = 0
    total = len(CANONICAL_API)
    for method, mappings in CANONICAL_API.items():
        if mappings[language] == method:
            matching += 1
    return matching / total * 100

# Output:
# C++: 60% consistency
# Java: 45% consistency
# Python: 70% consistency
```

## Example Usage

### Before (Current State)

```cpp
// C++
Person person;
person.set_name("Alice");
std::string data = person.SerializeAsString();
```

```java
// Java
Person person = Person.newBuilder()
    .setName("Alice")
    .build();
byte[] data = person.toByteArray();
```

```python
# Python
person = Person()
person.name = "Alice"
data = person.SerializeToString()
```

### After (With Canonical API)

```cpp
// C++ (canonical names available)
Person person;
person.setName("Alice");
std::string data = person.serialize();
```

```java
// Java (unchanged but documented as canonical)
Person person = Person.newBuilder()
    .setName("Alice")
    .build();
byte[] data = person.serialize();  // Alias for toByteArray
```

```python
# Python (unchanged but documented as canonical)
person = Person()
person.name = "Alice"
data = person.serialize()  # Alias for SerializeToString
```

## Implementation Plan

### Phase 1: Define Standards (Weeks 1-2)
- [ ] Create canonical API specification
- [ ] Document all current API variations
- [ ] Build consistency checker tool
- [ ] Get approval from all language maintainers

### Phase 2: Non-Breaking Additions (Weeks 3-4)
- [ ] Add method aliases in C++
- [ ] Add method aliases in Java
- [ ] Add method aliases in Python
- [ ] Update code generators

### Phase 3: Documentation (Week 5)
- [ ] Update all language guides
- [ ] Create migration guide
- [ ] Update API reference

### Phase 4: Testing & Rollout (Week 6)
- [ ] Test all aliases
- [ ] Update examples
- [ ] Announce changes

## Backwards Compatibility

### Fully Compatible (Phase 2)
- All existing methods preserved
- New methods are aliases
- No breaking changes

### Future Deprecation (Phase 3+)
- Old names marked deprecated
- Migration period of 2+ years
- Eventually remove old names

## Alternatives Considered

### Alternative 1: Language-Idiomatic Only
- **Pro:** Each language feels native
- **Con:** Hard to switch between languages
- **Decision:** Balance between idiomatic and consistent

### Alternative 2: Single Reference Implementation
- **Pro:** Perfect consistency
- **Con:** Doesn't leverage language strengths
- **Decision:** Rejected

### Alternative 3: Codegen-Level Consistency
- **Pro:** Consistency in generated code
- **Con:** Runtime API still inconsistent
- **Decision:** Both needed

## Open Questions

1. **Deprecation timeline** - How long to keep old names?
   - Suggestion: 2 years minimum

2. **Python properties** - Keep Pythonic `msg.name` syntax?
   - Suggestion: Yes, with method aliases

3. **Builder in C++** - Optional or required?
   - Suggestion: Optional, for those who want it

## Success Criteria

- [ ] 90% API parity score across languages
- [ ] Canonical methods available in all languages
- [ ] Documentation updated
- [ ] No breaking changes in Phase 2
- [ ] Positive community feedback

## Effort Estimation

| Task | Days |
|------|------|
| Specification | 5 |
| C++ implementation | 8 |
| Java implementation | 6 |
| Python implementation | 6 |
| Other languages | 5 |
| Testing | 5 |
| Documentation | 5 |
| **Total** | **40** (6 weeks) |

---

## References

- [Java Naming Conventions](https://google.github.io/styleguide/javaguide.html)
- [C++ Naming Conventions](https://google.github.io/styleguide/cppguide.html)
- [PEP 8 - Python Style Guide](https://www.python.org/dev/peps/pep-0008/)
