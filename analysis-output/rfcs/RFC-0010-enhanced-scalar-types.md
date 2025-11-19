# RFC-0010: Enhanced Scalar Types

**Status:** Draft
**Author:** Codebase Analysis
**Created:** November 19, 2025
**Effort:** 6 weeks
**Category:** Long-term

---

## Summary

Extend Protocol Buffers with additional well-defined scalar types (uuid, decimal, url, timestamp_tz) to provide type safety, compact wire format, and consistent representation for common data types that currently require verbose wrapper messages or error-prone string encoding.

## Motivation

### Problem Statement

Common data types require suboptimal representations:

1. **Type safety loss** - UUIDs as strings lose validation
2. **Wire format bloat** - UUID as string (36 bytes) vs binary (16 bytes)
3. **Precision errors** - Currency as float/double causes rounding
4. **Parsing overhead** - Runtime parsing of string representations
5. **Inconsistency** - Different services encode the same type differently

### Scale Impact

At scale, inefficiencies compound:
- UUID: string (36 bytes) vs bytes (16 bytes) = 55% savings
- Decimal: Arbitrary precision prevents floating-point errors
- URL: Validated at parse time, not runtime

### Common Workarounds

```protobuf
// Current approaches - all suboptimal

// UUID as string (wastes space, no validation)
string transaction_id = 1;

// UUID as bytes (no type safety)
bytes transaction_id = 1;

// Decimal as string (parsing overhead)
string amount = 2;

// URL as string (no validation)
string callback = 3;
```

## Detailed Design

### New Scalar Types

```protobuf
message Payment {
  uuid transaction_id = 1;        // 128-bit UUID
  decimal amount = 2;             // Arbitrary precision decimal
  url callback_url = 3;           // Validated URL
  timestamp_tz created_at = 4;    // Timestamp with timezone
}
```

### Wire Format Specifications

#### UUID (128 bits)
```
Wire type: 2 (length-delimited)
Format: 16 bytes, big-endian

Example: 550e8400-e29b-41d4-a716-446655440000
Bytes: [55 0e 84 00 e2 9b 41 d4 a7 16 44 66 55 44 00 00]
```

#### Decimal
```
Wire type: 2 (length-delimited)
Format: varint scale + bytes coefficient

Structure:
- scale: varint (number of decimal places)
- coefficient: bytes (signed integer, big-endian)

Example: 123.45
- scale: 2
- coefficient: 12345
Bytes: [02 30 39]
```

#### URL
```
Wire type: 2 (length-delimited)
Format: UTF-8 string with validation

Validation (at parse time):
- Valid URL scheme (http, https, etc.)
- Valid hostname
- Well-formed path/query
```

#### Timestamp with Timezone
```
Wire type: 2 (length-delimited)
Format: int64 seconds + int32 nanos + string timezone

Structure:
- seconds: int64 (UTC seconds since epoch)
- nanos: int32 (nanosecond offset)
- timezone: string (IANA timezone name)

Example: 2024-01-15T10:30:00-05:00[America/New_York]
```

### Generated Code

#### C++ Implementation

```cpp
// uuid type
class Uuid {
 public:
  // Constructors
  Uuid();
  explicit Uuid(const std::string& str);  // Parse from string
  explicit Uuid(const uint8_t bytes[16]); // From bytes

  // Accessors
  std::string ToString() const;           // "550e8400-e29b-41d4-..."
  const uint8_t* data() const;            // Raw 16 bytes

  // Comparison
  bool operator==(const Uuid& other) const;
  bool operator<(const Uuid& other) const;

  // Generation
  static Uuid Generate();                 // Random UUID v4

 private:
  uint8_t bytes_[16];
};

// decimal type
class Decimal {
 public:
  Decimal();
  explicit Decimal(const std::string& str);  // "123.45"
  Decimal(int64_t coefficient, int32_t scale);

  std::string ToString() const;
  double ToDouble() const;  // Lossy conversion

  // Arithmetic
  Decimal operator+(const Decimal& other) const;
  Decimal operator-(const Decimal& other) const;
  Decimal operator*(const Decimal& other) const;
  Decimal operator/(const Decimal& other) const;

  // Comparison
  int Compare(const Decimal& other) const;

 private:
  std::vector<uint8_t> coefficient_;
  int32_t scale_;
};

// Generated message
class Payment : public Message {
 public:
  const Uuid& transaction_id() const;
  Uuid* mutable_transaction_id();
  void set_transaction_id(const Uuid& value);

  const Decimal& amount() const;
  Decimal* mutable_amount();
  void set_amount(const Decimal& value);

  const Url& callback_url() const;
  void set_callback_url(const Url& value);
};
```

#### Java Implementation

```java
// UUID type (wraps java.util.UUID)
public final class ProtoUuid {
  private final java.util.UUID uuid;

  public static ProtoUuid fromString(String str) {
    return new ProtoUuid(java.util.UUID.fromString(str));
  }

  public static ProtoUuid generate() {
    return new ProtoUuid(java.util.UUID.randomUUID());
  }

  public byte[] toBytes() {
    ByteBuffer bb = ByteBuffer.wrap(new byte[16]);
    bb.putLong(uuid.getMostSignificantBits());
    bb.putLong(uuid.getLeastSignificantBits());
    return bb.array();
  }
}

// Decimal type (wraps java.math.BigDecimal)
public final class ProtoDecimal {
  private final java.math.BigDecimal value;

  public static ProtoDecimal fromString(String str) {
    return new ProtoDecimal(new BigDecimal(str));
  }

  public ProtoDecimal add(ProtoDecimal other) {
    return new ProtoDecimal(value.add(other.value));
  }

  public int compareTo(ProtoDecimal other) {
    return value.compareTo(other.value);
  }
}

// Generated message
public final class Payment extends GeneratedMessage {
  public ProtoUuid getTransactionId() { ... }
  public ProtoDecimal getAmount() { ... }
  public ProtoUrl getCallbackUrl() { ... }

  public static final class Builder {
    public Builder setTransactionId(ProtoUuid value) { ... }
    public Builder setAmount(ProtoDecimal value) { ... }
    public Builder setAmount(String value) {
      return setAmount(ProtoDecimal.fromString(value));
    }
  }
}
```

#### Python Implementation

```python
# uuid type (wraps uuid.UUID)
class ProtoUuid:
    def __init__(self, value: Union[str, bytes, uuid.UUID]):
        if isinstance(value, str):
            self._uuid = uuid.UUID(value)
        elif isinstance(value, bytes):
            self._uuid = uuid.UUID(bytes=value)
        else:
            self._uuid = value

    @classmethod
    def generate(cls) -> 'ProtoUuid':
        return cls(uuid.uuid4())

    def __str__(self) -> str:
        return str(self._uuid)

    @property
    def bytes(self) -> bytes:
        return self._uuid.bytes

# decimal type (wraps decimal.Decimal)
class ProtoDecimal:
    def __init__(self, value: Union[str, int, decimal.Decimal]):
        self._decimal = decimal.Decimal(value)

    def __add__(self, other: 'ProtoDecimal') -> 'ProtoDecimal':
        return ProtoDecimal(self._decimal + other._decimal)

    def __str__(self) -> str:
        return str(self._decimal)

# Generated message
class Payment(Message):
    @property
    def transaction_id(self) -> ProtoUuid:
        return self._transaction_id

    @transaction_id.setter
    def transaction_id(self, value: Union[str, ProtoUuid]):
        if isinstance(value, str):
            value = ProtoUuid(value)
        self._transaction_id = value
```

### JSON Mapping

```json
{
  "transactionId": "550e8400-e29b-41d4-a716-446655440000",
  "amount": "123.45",
  "callbackUrl": "https://example.com/callback",
  "createdAt": "2024-01-15T10:30:00-05:00[America/New_York]"
}
```

## Example Usage

### Creating Messages

```cpp
Payment payment;
payment.set_transaction_id(Uuid::Generate());
payment.mutable_amount()->SetFromString("99.99");
payment.set_callback_url(Url("https://example.com/webhook"));

std::string data = payment.SerializeAsString();
// UUID is 16 bytes (not 36), decimal is compact
```

### Currency Calculations

```cpp
Decimal price("19.99");
Decimal quantity("3");
Decimal tax_rate("0.08");

Decimal subtotal = price * quantity;           // 59.97
Decimal tax = subtotal * tax_rate;             // 4.7976
Decimal total = subtotal + tax;                // 64.7676

// No floating-point errors!
assert(total.ToString() == "64.7676");
```

### URL Validation

```cpp
// Valid URLs
payment.set_callback_url(Url("https://api.example.com/hook"));

// Invalid URL - throws at parse time
try {
  payment.set_callback_url(Url("not-a-url"));
} catch (const InvalidUrlError& e) {
  LOG(ERROR) << "Invalid URL: " << e.what();
}
```

## Implementation Plan

### Phase 1: Type Definitions (Week 1)
- [ ] Define wire format for each type
- [ ] Create type wrappers for each language
- [ ] Define JSON mapping

### Phase 2: Parser Integration (Weeks 2-3)
- [ ] Update protoc to recognize new types
- [ ] Implement wire format serialization
- [ ] Add validation during parsing

### Phase 3: Code Generation (Weeks 4-5)
- [ ] C++ code generation
- [ ] Java code generation
- [ ] Python code generation

### Phase 4: Testing & Polish (Week 6)
- [ ] Cross-language compatibility tests
- [ ] Performance benchmarks
- [ ] Documentation

## Backwards Compatibility

### Fully Compatible
- New scalar types are additions
- Old protos continue to work
- New types opt-in only

### Wire Format Evolution
- Each type has unique wire encoding
- Cannot accidentally interpret as old type

## Alternatives Considered

### Alternative 1: Continue with Well-Known Types
- **Pro:** Already exists
- **Con:** Verbose, not true scalars
- **Decision:** True scalars are more ergonomic

### Alternative 2: Bytes Fields Only
- **Pro:** Simple
- **Con:** No type safety, no validation
- **Decision:** Type safety too valuable

### Alternative 3: String Encoding
- **Pro:** Human-readable
- **Con:** Larger, slower, error-prone
- **Decision:** Binary is better for wire format

## Open Questions

1. **Which types to include** - uuid, decimal, url, timestamp_tz sufficient?
   - Suggestion: Start with these, add more based on demand

2. **Decimal precision** - Maximum scale/coefficient size?
   - Suggestion: 38 digits precision (like SQL Server)

3. **URL validation strictness** - How strict?
   - Suggestion: RFC 3986 compliant

4. **Timezone handling** - Store timezone name or offset?
   - Suggestion: Both (IANA name preferred)

## Success Criteria

- [ ] All four types implemented in C++, Java, Python
- [ ] Wire format 50%+ smaller than string for UUID
- [ ] Zero precision loss for decimal arithmetic
- [ ] Validation at parse time for URL
- [ ] Complete documentation and examples

## Effort Estimation

| Task | Days |
|------|------|
| Type wrapper classes | 5 |
| Wire format implementation | 5 |
| Parser updates | 5 |
| C++ generator | 5 |
| Java generator | 4 |
| Python generator | 3 |
| Testing | 5 |
| Documentation | 3 |
| **Total** | **35** (6 weeks) |

---

## References

- [RFC 4122 - UUID](https://datatracker.ietf.org/doc/html/rfc4122)
- [RFC 3986 - URI](https://datatracker.ietf.org/doc/html/rfc3986)
- [IEEE 754-2008 Decimal](https://en.wikipedia.org/wiki/Decimal_floating_point)
- [IANA Time Zone Database](https://www.iana.org/time-zones)
