# Enhanced Field Deprecation

This document describes the enhanced field deprecation feature that provides structured metadata for deprecated fields, enabling better tooling integration and migration guidance.

## Overview

The standard `deprecated = true` option provides minimal information about why a field is deprecated and what to use instead. The enhanced deprecation feature extends this with structured metadata including:

- Replacement field name
- Version when the field will be removed
- Version when deprecation started
- Migration guide
- Breaking change indicator
- Documentation URL
- Custom tags for tooling

## Usage

### Basic Example

```protobuf
import "google/protobuf/deprecation.proto";

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

### Full Example with All Fields

```protobuf
import "google/protobuf/deprecation.proto";

message Config {
  string old_setting = 1 [
    deprecated = true,
    (google.protobuf.deprecation) = {
      replacement: "settings"
      removal_version: "v4.0"
      since_version: "v3.0"
      migration_guide: "Migrate to the structured settings message. "
                       "See https://docs.example.com/config-migration"
      breaking: true
      documentation_url: "https://docs.example.com/config-migration"
      tags: {
        key: "team"
        value: "platform"
      }
      tags: {
        key: "ticket"
        value: "PROJ-1234"
      }
    }
  ];

  Settings settings = 2;
}
```

## DeprecationInfo Message

The `DeprecationInfo` message contains the following fields:

| Field | Type | Description |
|-------|------|-------------|
| `replacement` | string | Name of the field to use instead |
| `removal_version` | string | Version when the field will be removed |
| `since_version` | string | Version when deprecation started |
| `migration_guide` | string | Human-readable migration instructions |
| `breaking` | bool | Whether removal is a breaking change |
| `documentation_url` | string | URL to detailed documentation |
| `tags` | map<string, string> | Custom key-value tags for tooling |

## Generated Code

### C++

Deprecated fields generate enhanced `[[deprecated]]` attributes with detailed messages:

```cpp
[[deprecated(
  "Deprecated since v2.0, removed in v3.0. "
  "Use uuid_id instead. "
  "Migration: Use uuid_id for better uniqueness guarantees."
)]]
int32_t legacy_id() const;
```

### Java

Deprecated fields generate enhanced Javadoc with `@deprecated` tags:

```java
/**
 * @deprecated User.legacy_id is deprecated.
 *     Since v2.0, removed in v3.0
 *     Use {@link #getUuidId()} instead.
 *     Migration: Use uuid_id for better uniqueness guarantees.
 *     See https://docs.example.com/migrations/uuid
 */
@Deprecated
public int getLegacyId() { ... }
```

### Python

Deprecated fields generate comments in `.pyi` type stub files:

```python
# deprecated: use uuid_id instead, removed in v3.0
legacy_id: int
```

## Backward Compatibility

The enhanced deprecation feature is fully backward compatible:

- Existing `deprecated = true` options continue to work unchanged
- The deprecation extension is optional
- Code that doesn't use the extension will see standard deprecation behavior
- Old protoc versions will ignore the extension

## Querying Deprecation Info at Runtime

You can query deprecation information at runtime using reflection:

### C++

```cpp
const FieldDescriptor* field =
    User::descriptor()->FindFieldByName("legacy_id");

if (field->options().deprecated()) {
  // Access deprecation extension via reflection
  const FieldOptions& options = field->options();
  const Reflection* reflection = options.GetReflection();

  std::vector<const FieldDescriptor*> fields;
  reflection->ListFields(options, &fields);

  for (const FieldDescriptor* f : fields) {
    if (f->is_extension() && f->number() == 1001) {
      // Extract deprecation info
      const Message& deprecation = reflection->GetMessage(options, f);
      // Process deprecation metadata...
    }
  }
}
```

### Java

```java
FieldDescriptor field = User.getDescriptor().findFieldByName("legacy_id");
if (field.getOptions().getDeprecated()) {
  // Access deprecation extension
  if (field.getOptions().hasExtension(DeprecationProto.deprecation)) {
    DeprecationInfo info = field.getOptions()
        .getExtension(DeprecationProto.deprecation);
    String replacement = info.getReplacement();
    String removalVersion = info.getRemovalVersion();
    // Process deprecation metadata...
  }
}
```

## Best Practices

1. **Always provide a replacement**: Tell users what to use instead
2. **Include version information**: Help users plan migrations
3. **Write clear migration guides**: Make migration as easy as possible
4. **Mark breaking changes**: Use `breaking: true` for major changes
5. **Use semantic versioning**: Follow semver for version fields
6. **Link to documentation**: Provide URLs for complex migrations
7. **Use tags for tracking**: Add team/ticket info for internal tracking

## Tooling Integration

The structured metadata enables building tools such as:

- **Deprecation reporters**: Generate reports of deprecated field usage
- **Usage analyzers**: Find all usages of deprecated fields in code
- **Migration tools**: Automated refactoring based on replacement info
- **CI/CD checks**: Fail builds that use deprecated fields

Example CI integration:

```yaml
# .github/workflows/deprecation-check.yml
name: Deprecation Check
on: pull_request
jobs:
  check:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Check deprecated field usage
        run: |
          protoc-usage-analyzer \
            --schema=protos/**/*.proto \
            --codebase=src/ \
            --fail-on-deprecated
```

## Extension Numbers

The deprecation extensions use extension number 1001 in the following options:

- `google.protobuf.FieldOptions` - for field deprecation
- `google.protobuf.EnumValueOptions` - for enum value deprecation
- `google.protobuf.MessageOptions` - for message deprecation
- `google.protobuf.MethodOptions` - for method deprecation
- `google.protobuf.ServiceOptions` - for service deprecation
- `google.protobuf.EnumOptions` - for enum deprecation

## Future Enhancements

Planned future improvements include:

- Compiler strict mode to fail on deprecated field usage
- Cross-repository usage tracking
- Automated migration tool generation
- IDE integration for better warnings
