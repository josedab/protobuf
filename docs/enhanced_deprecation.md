# Enhanced Field Deprecation

Protocol Buffers now supports enhanced field deprecation with structured metadata. This allows you to provide additional information about deprecated fields, including:

- Replacement field name
- Version when the field will be removed
- Version when deprecation started
- Migration instructions
- Whether removal is a breaking change
- Documentation URLs
- Custom tags for tooling

## Basic Usage

### Defining Enhanced Deprecation

First, import the deprecation extension:

```protobuf
import "google/protobuf/deprecation.proto";
```

Then annotate your deprecated fields with the extension:

```protobuf
message User {
  int32 legacy_id = 1 [
    deprecated = true,
    (google.protobuf.deprecation) = {
      replacement: "uuid_id"
      removal_version: "v3.0"
      since_version: "v2.0"
      migration_guide: "Use uuid_id for better uniqueness guarantees."
      breaking: true
    }
  ];

  string uuid_id = 2;
}
```

### DeprecationInfo Message

The `DeprecationInfo` message contains the following fields:

| Field | Type | Description |
|-------|------|-------------|
| `replacement` | string | The field name to use instead |
| `removal_version` | string | Version when field will be removed |
| `since_version` | string | Version when deprecation started |
| `migration_guide` | string | Instructions for migrating |
| `breaking` | bool | Whether removal is a breaking change |
| `documentation_url` | string | URL for detailed documentation |
| `tags` | map<string, string> | Custom key-value pairs for tooling |

## Generated Code

### C++

The C++ code generator creates enhanced deprecation attributes:

```cpp
// Generated code
[[deprecated("Deprecated since v2.0, removed in v3.0. Use uuid_id instead. "
             "Migration: Use uuid_id for better uniqueness guarantees.")]]
int32_t legacy_id() const;
```

### Java

The Java code generator creates enhanced Javadoc:

```java
/**
 * @deprecated protobuf_unittest.User.legacy_id is deprecated.
 *             Since v2.0, removed in v3.0.
 *             Use {@link #getUuidId()} instead.
 *             Migration: Use uuid_id for better uniqueness guarantees.
 */
@Deprecated
public int getLegacyId() { ... }
```

### Python

In Python, deprecation metadata is accessible through the descriptor API at runtime:

```python
from google.protobuf import deprecation_pb2

field = User.DESCRIPTOR.fields_by_name['legacy_id']
if field.GetOptions().deprecated:
    deprecation_info = field.GetOptions().Extensions[deprecation_pb2.deprecation]
    print(f"Replacement: {deprecation_info.replacement}")
    print(f"Removed in: {deprecation_info.removal_version}")
```

## Deprecation Tools

### Deprecation Reporter

Generate a comprehensive report of all deprecated fields:

```bash
# Generate a FileDescriptorSet
protoc --descriptor_set_out=schema.pb your_protos/*.proto

# Generate deprecation report
python tools/deprecation/deprecation_reporter.py \
    --schema=schema.pb \
    --current-version=v2.5

# Output as JSON
python tools/deprecation/deprecation_reporter.py \
    --schema=schema.pb \
    --format=json
```

Example output:

```
============================================================
Deprecation Report
============================================================

Current Version: v2.5

Breaking Changes:
----------------------------------------
  - User.legacy_id -> uuid_id (removed in v3.0)

Non-Breaking Deprecations:
----------------------------------------
  - User.deprecated_metadata -> metadata

Timeline:
----------------------------------------
  - v2.0: User.legacy_id deprecated
  - v3.0: User.legacy_id removed (BREAKING)

============================================================
Total deprecated fields: 2
Breaking changes: 1
============================================================
```

### Usage Analyzer

Find usage of deprecated fields in your codebase:

```bash
python tools/deprecation/usage_analyzer.py \
    --schema=schema.pb \
    --codebase=./src \
    --fail-on-deprecated
```

Example output:

```
============================================================
Deprecated Field Usage Report
============================================================

User.legacy_id (deprecated v2.0):
  - src/user_service.cc:45, 89
  - src/auth_service.java:123
  Suggested replacement: uuid_id

============================================================
Total usages: 3
Unique deprecated fields: 1
============================================================
```

Options:
- `--fail-on-deprecated`: Exit with error if deprecated usage found (for CI)
- `--format=json`: Output as JSON
- `--extensions=.cc,.java,.py`: Limit file types to scan

## CI Integration

### GitHub Actions

```yaml
name: Deprecation Check

on: pull_request

jobs:
  check-deprecated:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Generate descriptor set
        run: |
          protoc --descriptor_set_out=schema.pb \
                 --include_imports \
                 protos/**/*.proto

      - name: Check deprecated field usage
        run: |
          python tools/deprecation/usage_analyzer.py \
            --schema=schema.pb \
            --codebase=src/ \
            --fail-on-deprecated

      - name: Generate deprecation report
        run: |
          python tools/deprecation/deprecation_reporter.py \
            --schema=schema.pb \
            --current-version=${{ github.event.release.tag_name || 'dev' }}
```

## Best Practices

### When to Use Enhanced Deprecation

1. **Version planning**: When you know the removal timeline
2. **Breaking changes**: Mark as breaking when removal will break consumers
3. **Clear migration path**: Always provide a replacement if one exists
4. **Complex migrations**: Use migration_guide for detailed instructions

### Migration Guide Contents

Good migration guides should include:
- What to use instead
- Why the change was made
- Any behavioral differences
- Code examples if complex

Example:

```protobuf
(google.protobuf.deprecation) = {
  replacement: "emails"
  migration_guide: "Migrate to 'emails' repeated field for multi-email support. "
                   "Old code: user.email() -> New code: user.emails(0) for primary. "
                   "Note: The first email in the list is considered primary."
}
```

### Versioning Conventions

We recommend semantic versioning format:
- `v1.0.0` or `v1.0` for full versions
- Be consistent across your project

### Custom Tags

Use tags for organization-specific metadata:

```protobuf
(google.protobuf.deprecation) = {
  tags: { key: "team" value: "platform" }
  tags: { key: "ticket" value: "PROJ-1234" }
  tags: { key: "priority" value: "high" }
}
```

## Backward Compatibility

- Existing `deprecated = true` fields continue to work unchanged
- The enhanced deprecation extension is optional
- Code that doesn't use the extension will see standard deprecation warnings
- You can add enhanced metadata to existing deprecated fields incrementally

## Extending to Other Elements

Enhanced deprecation is also available for:

- **Enum values**: Use `(google.protobuf.enum_value_deprecation)`
- **Service methods**: Use `(google.protobuf.method_deprecation)`
- **Services**: Use `(google.protobuf.service_deprecation)`

Example for enum values:

```protobuf
enum Status {
  UNKNOWN = 0;
  ACTIVE = 1;
  INACTIVE = 2 [
    deprecated = true,
    (google.protobuf.enum_value_deprecation) = {
      replacement: "DISABLED"
      migration_guide: "Use DISABLED for clearer semantics."
    }
  ];
  DISABLED = 3;
}
```
