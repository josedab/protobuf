# RFC-0012: Safe Field Number Management

**Status:** Draft
**Author:** Codebase Analysis
**Created:** November 19, 2025
**Effort:** 4 weeks
**Category:** Strategic

---

## Summary

Implement automatic tracking and enforcement of reserved field numbers to prevent accidental field number reuse, which causes silent data corruption and backward compatibility breaks.

## Motivation

### Problem Statement

Accidentally reusing field numbers causes catastrophic, silent bugs:

```protobuf
message Foo {
  // Someone deleted: int32 old_count = 1;
  string name = 1;  // DISASTER - wire format collision!
}
```

Current `reserved` keyword requires manual vigilance:

```protobuf
message Foo {
  reserved 1;  // Must remember to add this!
  string name = 2;
}
```

### Bug Characteristics

Field reuse bugs are:
- **Silent** - No compiler error
- **Catastrophic** - Data corruption
- **Difficult to detect** - May work in tests, fail in production
- **Hard to debug** - Old clients send data, new servers misinterpret

### Real-World Impact

- Production incidents from field reuse
- Hours of debugging time
- Data corruption requiring manual fixes
- Loss of user trust

## Detailed Design

### 1. Schema History Tracking

Automatically track all field number usage in a history file:

```json
// user.proto.history
{
  "file": "user.proto",
  "messages": {
    "User": {
      "fields": {
        "1": {
          "name": "legacy_id",
          "type": "int32",
          "status": "deleted",
          "added_version": "v1.0.0",
          "deleted_version": "v2.0.0"
        },
        "2": {
          "name": "name",
          "type": "string",
          "status": "active",
          "added_version": "v1.0.0"
        },
        "3": {
          "name": "email",
          "type": "string",
          "status": "active",
          "added_version": "v1.5.0"
        }
      }
    }
  }
}
```

### 2. Compiler Enforcement

#### Strict Mode

```bash
$ protoc --strict-reserved user.proto

Error: user.proto:5: Field number 1 was previously used by 'legacy_id' (deleted in v2.0.0)
       Add 'reserved 1;' to message User or use a different field number.
```

#### Warning Mode

```bash
$ protoc --warn-reserved user.proto

Warning: user.proto:5: Field number 1 was previously used.
         Consider using 'reserved 1;' to prevent future issues.
```

### 3. Automatic Reserved Inference

Generate reserved statements from history:

```protobuf
// Generated from history
message User {
  reserved 1;  // legacy_id (deleted v2.0.0)
  reserved "legacy_id";

  string name = 2;
  string email = 3;
}
```

### 4. History Generation Tool

Generate history from git commits:

```bash
$ protoc-history-gen --repo=. --output=protos/

Analyzing git history for proto changes...

Generated history files:
  - protos/user.proto.history (3 deleted fields found)
  - protos/order.proto.history (1 deleted field found)

Summary:
  Messages analyzed: 15
  Deleted fields found: 4
  Potential issues: 0
```

### 5. Schema Annotations

Embed history directly in schema:

```protobuf
import "google/protobuf/history.proto";

message User {
  option (history) = {
    deleted_fields: [
      {
        number: 1
        name: "legacy_id"
        type: "int32"
        deleted_version: "v2.0.0"
        reason: "Migrated to UUID"
      }
    ]
  };

  // Compiler enforces: cannot use field 1
  string name = 2;
  string email = 3;
}
```

### Generated Code Enhancements

#### C++
```cpp
class User : public Message {
 public:
  // Static method to check field number safety
  static bool IsFieldNumberSafe(int number) {
    static const std::set<int> deleted = {1};
    return deleted.find(number) == deleted.end();
  }

  // Get deleted field info
  static const std::vector<DeletedFieldInfo>& GetDeletedFields() {
    static const std::vector<DeletedFieldInfo> fields = {
      {1, "legacy_id", "int32", "v2.0.0"}
    };
    return fields;
  }
};
```

### Workflow Integration

#### Pre-commit Hook
```bash
#!/bin/bash
# .git/hooks/pre-commit

# Check for field number reuse
protoc --strict-reserved $(git diff --cached --name-only '*.proto')
if [ $? -ne 0 ]; then
  echo "Error: Field number reuse detected"
  exit 1
fi

# Update history files
protoc-history-update $(git diff --cached --name-only '*.proto')
git add *.proto.history
```

#### CI Check
```yaml
# .github/workflows/proto-safety.yml
name: Proto Safety Check

on: pull_request

jobs:
  check:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
        with:
          fetch-depth: 0  # Full history

      - name: Generate history
        run: protoc-history-gen --repo=. --output=protos/

      - name: Check field numbers
        run: protoc --strict-reserved protos/**/*.proto

      - name: Check history updated
        run: |
          if ! git diff --exit-code *.proto.history; then
            echo "Error: History files not updated"
            exit 1
          fi
```

## Example Usage

### Detecting Field Reuse

```bash
$ cat user.proto
message User {
  string name = 1;  // Was previously 'int32 legacy_id'
}

$ protoc --strict-reserved user.proto
Error: Field number 1 was previously used by 'legacy_id'

# Fix: Add reserved
$ cat user.proto
message User {
  reserved 1;
  string name = 2;
}

$ protoc --strict-reserved user.proto
Success!
```

### Generating History from Git

```bash
$ protoc-history-gen --repo=. --output=protos/

Analyzing commit history...
  - abc123: Added User message
  - def456: Added legacy_id to User
  - ghi789: Removed legacy_id from User

Generated: protos/user.proto.history
```

### Viewing Field Usage

```bash
$ protoc-field-info user.proto User

Message: User
=============

Active Fields:
  2: name (string) - added v1.0.0
  3: email (string) - added v1.5.0

Deleted Fields:
  1: legacy_id (int32) - deleted v2.0.0

Reserved: 1, "legacy_id"

Available field numbers: 4-536870911
```

## Implementation Plan

### Week 1: History Format and Parser
- [ ] Define history file JSON schema
- [ ] Implement history file parser
- [ ] Add history option to proto syntax

### Week 2: Compiler Integration
- [ ] Add --strict-reserved flag
- [ ] Implement field number checking
- [ ] Generate reserved from history

### Week 3: Tooling
- [ ] Implement history generator from git
- [ ] Add field info tool
- [ ] Create pre-commit hooks

### Week 4: Testing and Documentation
- [ ] Comprehensive tests
- [ ] Migration documentation
- [ ] Example workflows

## Backwards Compatibility

### Fully Compatible
- New features are opt-in
- Existing protos work unchanged
- History files are supplementary

### Gradual Adoption
1. Generate history files from git
2. Enable warnings first
3. Move to strict mode over time

## Alternatives Considered

### Alternative 1: External Linting Only
- **Pro:** No compiler changes
- **Con:** Not enforced, easily skipped
- **Decision:** Need compiler enforcement

### Alternative 2: Never Delete Fields
- **Pro:** No reuse possible
- **Con:** Schema bloat
- **Decision:** Deletion needed, but tracked

### Alternative 3: Sequential-Only Numbers
- **Pro:** Simple rule
- **Con:** Too restrictive
- **Decision:** Allow any number, but track

## Open Questions

1. **History storage** - Separate files or in .proto?
   - Suggestion: Both options supported

2. **Git integration** - How deep into history?
   - Suggestion: Configurable, default full history

3. **Cross-repo coordination** - Shared protos?
   - Suggestion: Central history repo

## Success Criteria

- [ ] History generation from git working
- [ ] Compiler --strict-reserved enforced
- [ ] Pre-commit hooks available
- [ ] CI integration documented
- [ ] Zero field reuse bugs after adoption

## Effort Estimation

| Task | Days |
|------|------|
| History format | 2 |
| Parser integration | 4 |
| Git history analyzer | 4 |
| Tooling | 4 |
| Testing | 3 |
| Documentation | 3 |
| **Total** | **20** (4 weeks) |

---

## References

- [Protocol Buffers Reserved](https://developers.google.com/protocol-buffers/docs/proto3#reserved)
- [Buf Breaking Change Detection](https://buf.build/docs/breaking/overview)
- [Git Pre-commit Hooks](https://git-scm.com/docs/githooks)
