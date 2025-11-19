# Enhanced Scalar Types

This document describes the enhanced scalar types added to Protocol Buffers to provide type-safe, compact representations for common data types.

## Overview

Four new well-known types have been added:

- **Uuid** - 128-bit universally unique identifier (RFC 4122)
- **Decimal** - Arbitrary-precision decimal number
- **Url** - Validated URL (RFC 3986)
- **TimestampTz** - Timestamp with timezone information

## UUID

The `Uuid` type provides a compact, type-safe representation for UUIDs.

### Wire Format Benefits
- String representation: 36 bytes
- Binary representation: 16 bytes
- **Savings: 55%**

### Usage

```cpp
#include "google/protobuf/uuid.pb.h"
#include "google/protobuf/util/uuid_util.h"

using google::protobuf::Uuid;
using google::protobuf::util::UuidUtil;

// Generate a random UUID
Uuid uuid = UuidUtil::Generate();

// Convert to string
std::string str = UuidUtil::ToString(uuid);
// "550e8400-e29b-41d4-a716-446655440000"

// Parse from string
Uuid parsed;
UuidUtil::FromString("550e8400-e29b-41d4-a716-446655440000", &parsed);

// Convert to/from bytes
uint8_t bytes[16];
UuidUtil::ToBytes(uuid, bytes);
UuidUtil::FromBytes(bytes, &uuid);
```

### Proto Definition

```protobuf
message Uuid {
  uint64 high = 1;  // Most significant 64 bits
  uint64 low = 2;   // Least significant 64 bits
}
```

## Decimal

The `Decimal` type provides arbitrary-precision decimal arithmetic without floating-point rounding errors.

### Wire Format Benefits
- Exact representation of decimal values
- No floating-point rounding errors
- Suitable for financial calculations

### Usage

```cpp
#include "google/protobuf/decimal.pb.h"
#include "google/protobuf/util/decimal_util.h"

using google::protobuf::Decimal;
using google::protobuf::util::DecimalUtil;

// Parse from string
Decimal price, quantity, total;
DecimalUtil::FromString("19.99", &price);
DecimalUtil::FromString("3", &quantity);

// Arithmetic operations
total = DecimalUtil::Multiply(price, quantity);  // 59.97

// Convert to string
std::string str = DecimalUtil::ToString(total);  // "59.97"

// Comparison
if (DecimalUtil::Compare(total, price) > 0) {
  // total is greater
}

// Operators
Decimal sum = price + quantity;
Decimal diff = price - quantity;
```

### Proto Definition

```protobuf
message Decimal {
  bytes coefficient = 1;  // Signed integer in big-endian
  int32 scale = 2;        // Number of decimal places
}
```

## URL

The `Url` type provides validated URL handling with parsed components.

### Wire Format Benefits
- Validation at parse time
- Parsed components for efficient access
- RFC 3986 compliant

### Usage

```cpp
#include "google/protobuf/url.pb.h"
#include "google/protobuf/util/url_util.h"

using google::protobuf::Url;
using google::protobuf::util::UrlUtil;

// Parse a URL
Url url;
if (UrlUtil::FromString("https://user:pass@example.com:8080/path?query=value#fragment", &url)) {
  // Access components
  std::string scheme = url.scheme();    // "https"
  std::string host = url.host();        // "example.com"
  int32_t port = url.port();            // 8080
  std::string path = url.path();        // "/path"
  std::string query = url.query();      // "query=value"
}

// URL encoding/decoding
std::string encoded = UrlUtil::Encode("hello world");  // "hello%20world"
std::string decoded = UrlUtil::Decode("hello%20world");  // "hello world"

// Query parsing
std::vector<std::pair<std::string, std::string>> params;
UrlUtil::ParseQuery("key1=value1&key2=value2", &params);

// Normalize URLs
UrlUtil::Normalize(&url);  // Lowercase scheme/host, remove default port
```

### Proto Definition

```protobuf
message Url {
  string value = 1;     // Complete URL string
  string scheme = 2;    // e.g., "https"
  string host = 3;      // e.g., "example.com"
  int32 port = 4;       // Port number (0 = default)
  string path = 5;      // e.g., "/api/users"
  string query = 6;     // e.g., "key=value"
  string fragment = 7;  // e.g., "section1"
  string userinfo = 8;  // e.g., "user:pass"
}
```

## TimestampTz

The `TimestampTz` type extends `Timestamp` with timezone information.

### Wire Format Benefits
- Preserves timezone information
- Supports timezone-aware calculations
- Uses IANA timezone names

### Usage

```cpp
#include "google/protobuf/timestamp_tz.pb.h"
#include "google/protobuf/util/timestamp_tz_util.h"

using google::protobuf::TimestampTz;
using google::protobuf::Duration;
using google::protobuf::util::TimestampTzUtil;

// Parse from string
TimestampTz ts;
TimestampTzUtil::FromString(
    "2024-01-15T10:30:00-05:00[America/New_York]", &ts);

// Get current time in a timezone
TimestampTz now = TimestampTzUtil::Now("America/Los_Angeles");

// Convert to local time string
std::string local = TimestampTzUtil::ToLocalTimeString(ts);

// Extract components
int year = TimestampTzUtil::GetYear(ts);
int month = TimestampTzUtil::GetMonth(ts);
int day = TimestampTzUtil::GetDay(ts);

// Arithmetic with Duration
Duration hour;
hour.set_seconds(3600);
TimestampTz later = TimestampTzUtil::Add(ts, hour);

// Operators
ts += hour;
Duration diff = ts - later;
```

### Proto Definition

```protobuf
message TimestampTz {
  int64 seconds = 1;           // UTC seconds since epoch
  int32 nanos = 2;             // Nanoseconds (0-999999999)
  string timezone = 3;         // IANA timezone name
  int32 utc_offset_seconds = 4; // UTC offset at this time
}
```

## JSON Mapping

All types serialize to JSON in human-readable formats:

```json
{
  "transactionId": "550e8400-e29b-41d4-a716-446655440000",
  "amount": "123.45",
  "callbackUrl": "https://example.com/callback",
  "createdAt": "2024-01-15T10:30:00-05:00[America/New_York]"
}
```

## Build Targets

The following Bazel targets are available:

### Proto libraries
```
//src/google/protobuf:uuid_proto
//src/google/protobuf:uuid_cc_proto
//src/google/protobuf:decimal_proto
//src/google/protobuf:decimal_cc_proto
//src/google/protobuf:url_proto
//src/google/protobuf:url_cc_proto
//src/google/protobuf:timestamp_tz_proto
//src/google/protobuf:timestamp_tz_cc_proto
```

### Utility libraries
```
//src/google/protobuf/util:uuid_util
//src/google/protobuf/util:decimal_util
//src/google/protobuf/util:url_util
//src/google/protobuf/util:timestamp_tz_util
```

## References

- [RFC 4122 - UUID](https://datatracker.ietf.org/doc/html/rfc4122)
- [RFC 3986 - URI](https://datatracker.ietf.org/doc/html/rfc3986)
- [IEEE 754-2008 Decimal](https://en.wikipedia.org/wiki/Decimal_floating_point)
- [IANA Time Zone Database](https://www.iana.org/time-zones)
