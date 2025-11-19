# RFC-0011: Improved Field Deprecation

**Status:** Draft
**Author:** Codebase Analysis
**Created:** November 19, 2025
**Effort:** 4 weeks
**Category:** Strategic

---

## Summary

Enhance Protocol Buffers field deprecation with structured metadata including replacement field, removal version, and migration guidance, enabling tooling for automated migration assistance and breaking change detection.

## Motivation

### Problem Statement

Current deprecation is a hint-only mechanism with no guidance:

```protobuf
int32 old_field = 1 [deprecated = true];  // What should I use instead?
```

Missing information:
- What field to use instead
- When will it be removed
- How to migrate
- Breaking change impact

### Business Impact

In large organizations:
- Deprecated fields remain used for years
- Breaking changes surprise teams
- Manual migration is error-prone
- No visibility into deprecation status

### Current State

```cpp
// Generated code just has a warning
[[deprecated("Use new_field instead")]]
int32_t old_field() const;
```

But this doesn't provide:
- Actionable migration path
- Timeline for removal
- Tooling integration

## Detailed Design

### Enhanced Syntax

```protobuf
import "google/protobuf/deprecation.proto";

message User {
  int32 legacy_id = 1 [
    deprecated = true,
    (deprecation) = {
      replacement: "uuid_id"
      removal_version: "v3.0"
      since_version: "v2.0"
      migration_guide: "Use uuid_id for better uniqueness guarantees. "
                       "Run `migrate-tool --field=legacy_id` to update."
      breaking: true
    }
  ];

  string uuid_id = 2;

  string old_email = 3 [
    deprecated = true,
    (deprecation) = {
      replacement: "emails"
      removal_version: "v3.0"
      migration_guide: "Migrate to emails repeated field for multi-email support."
    }
  ];

  repeated string emails = 4;
}
```

### Deprecation Metadata

```protobuf
// google/protobuf/deprecation.proto
message DeprecationInfo {
  // Field to use instead
  string replacement = 1;

  // Version when field will be removed
  string removal_version = 2;

  // Version when deprecation started
  string since_version = 3;

  // Migration instructions
  string migration_guide = 4;

  // Is this a breaking change?
  bool breaking = 5;

  // URL for more information
  string documentation_url = 6;

  // Custom tags for tooling
  map<string, string> tags = 7;
}
```

### Generated Code

#### C++
```cpp
// Enhanced deprecation warning
[[deprecated(
  "Deprecated since v2.0, removed in v3.0. "
  "Use uuid_id instead. "
  "Migration: Use uuid_id for better uniqueness guarantees."
)]]
int32_t legacy_id() const;

// Deprecation info accessor
static const DeprecationInfo& GetDeprecationInfo_legacy_id() {
  static const DeprecationInfo info = {
    .replacement = "uuid_id",
    .removal_version = "v3.0",
    .since_version = "v2.0",
    .migration_guide = "Use uuid_id for better uniqueness..."
  };
  return info;
}
```

#### Java
```java
/**
 * @deprecated Since v2.0, removed in v3.0.
 *             Use {@link #getUuidId()} instead.
 *             Migration: Use uuid_id for better uniqueness guarantees.
 */
@Deprecated
public int getLegacyId() { ... }

// Deprecation info
public static DeprecationInfo getLegacyIdDeprecationInfo() {
  return DeprecationInfo.newBuilder()
      .setReplacement("uuid_id")
      .setRemovalVersion("v3.0")
      .setSinceVersion("v2.0")
      .setMigrationGuide("Use uuid_id for better uniqueness...")
      .build();
}
```

### Compiler Warnings

```bash
$ protoc --cpp_out=. --deprecation_warnings=strict user.proto

Warning: user.proto:15: Field 'User.legacy_id' is deprecated
  Replacement: uuid_id
  Removal: v3.0
  Migration: Use uuid_id for better uniqueness guarantees.

Warning: user.proto:23: Field 'User.old_email' is deprecated
  Replacement: emails
  Removal: v3.0
```

### Tooling Integration

#### Migration Reporter
```bash
$ protoc-deprecation-report --schema=user.proto --current-version=v2.5

Deprecation Report
==================

Breaking Changes Before v3.0:
- User.legacy_id -> User.uuid_id
- User.old_email -> User.emails

Timeline:
- v2.0: legacy_id deprecated
- v2.0: old_email deprecated
- v3.0: legacy_id removed (BREAKING)
- v3.0: old_email removed

Recommended Actions:
1. Update legacy_id usage to uuid_id
2. Update old_email usage to emails
```

#### Usage Analyzer
```bash
$ protoc-usage-analyzer --schema=user.proto --codebase=./src

Deprecated Field Usage Report
=============================

User.legacy_id (deprecated v2.0):
  - src/user_service.cc:45
  - src/auth_service.cc:123
  - src/report_generator.cc:89

User.old_email (deprecated v2.0):
  - src/notification_service.cc:234

Total: 4 usages of deprecated fields
```

#### Automated Refactoring
```bash
$ protoc-migrate --schema=user.proto --field=legacy_id --codebase=./src

Migrating User.legacy_id -> User.uuid_id

Files to update:
  - src/user_service.cc (3 changes)
  - src/auth_service.cc (1 change)
  - src/report_generator.cc (2 changes)

Preview changes? [Y/n] Y

--- src/user_service.cc
+++ src/user_service.cc
@@ -45,7 +45,7 @@
-  int32_t id = user.legacy_id();
+  std::string id = user.uuid_id();

Apply changes? [Y/n]
```

## Example Usage

### Defining Deprecations

```protobuf
message Config {
  // Fully documented deprecation
  string old_setting = 1 [
    deprecated = true,
    (deprecation) = {
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
    }
  ];

  Settings settings = 2;
}
```

### Querying Deprecation Info

```cpp
// Check if field is deprecated
const FieldDescriptor* field = User::descriptor()->FindFieldByName("legacy_id");
if (field->options().deprecated()) {
  const DeprecationInfo& info = field->options().GetExtension(deprecation);

  LOG(WARNING) << "Field " << field->name() << " is deprecated"
               << "\n  Replacement: " << info.replacement()
               << "\n  Removed in: " << info.removal_version()
               << "\n  Migration: " << info.migration_guide();
}
```

### CI Integration

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
            --fail-on-deprecated \
            --current-version=${{ github.event.release.tag_name }}
```

## Implementation Plan

### Week 1: Schema and Parser
- [ ] Define deprecation.proto
- [ ] Update protoc parser
- [ ] Implement deprecation option reading

### Week 2: Code Generation
- [ ] Update C++ generator for enhanced warnings
- [ ] Update Java generator
- [ ] Update Python generator

### Week 3: Tooling
- [ ] Implement deprecation reporter
- [ ] Implement usage analyzer
- [ ] Basic migration tool

### Week 4: Testing and Documentation
- [ ] Comprehensive tests
- [ ] Documentation
- [ ] Example projects

## Backwards Compatibility

### Fully Compatible
- Existing `deprecated = true` continues to work
- New metadata is optional extension
- Old code ignores new options

### Migration Path
- Add metadata to existing deprecations
- Tooling works with partial information
- Gradual adoption

## Alternatives Considered

### Alternative 1: External Deprecation Database
- **Pro:** Centralized
- **Con:** Disconnected from schema, drift risk
- **Decision:** Keep with schema

### Alternative 2: Comment-Based Metadata
- **Pro:** Simpler syntax
- **Con:** Not machine-readable
- **Decision:** Structured options better

### Alternative 3: Separate Deprecation Files
- **Pro:** Doesn't clutter schema
- **Con:** Synchronization issues
- **Decision:** Inline with fields

## Open Questions

1. **Compiler strictness** - Should protoc fail on deprecated usage?
   - Suggestion: Optional strict mode

2. **Version format** - Semver or custom?
   - Suggestion: Semver recommended, but allow custom

3. **Cross-repo tracking** - Track usage across repositories?
   - Suggestion: Future enhancement

## Success Criteria

- [ ] Enhanced deprecation syntax in protoc
- [ ] Rich warnings in generated code
- [ ] Deprecation report tool working
- [ ] Usage analyzer working
- [ ] Documentation complete

## Effort Estimation

| Task | Days |
|------|------|
| Schema definition | 2 |
| Parser updates | 3 |
| Code generators | 5 |
| Reporter tool | 3 |
| Analyzer tool | 3 |
| Testing | 3 |
| Documentation | 2 |
| **Total** | **21** (4 weeks) |

---

## References

- [Semantic Versioning](https://semver.org/)
- [Java @Deprecated Best Practices](https://docs.oracle.com/javase/9/docs/api/java/lang/Deprecated.html)
- [API Evolution Guidelines](https://cloud.google.com/apis/design/compatibility)
