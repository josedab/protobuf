# RFC-0015: Enhanced Documentation Support

**Status:** Draft
**Author:** Codebase Analysis
**Created:** November 19, 2025
**Effort:** 5 weeks
**Category:** Strategic

---

## Summary

Extend Protocol Buffers comment syntax to support structured documentation with Markdown formatting, cross-references, examples, and deprecation notices, enabling automatic generation of rich API documentation.

## Motivation

### Problem Statement

Proto comments are basic text only:

```protobuf
// User represents a person
message User {
  // The user's email
  string email = 1;
}
```

Missing features:
- Markdown formatting
- Cross-references to other types
- Code examples
- Structured metadata (since version, deprecated in)
- Generated documentation output

### Impact

- Poor API documentation
- Difficult onboarding
- Repeated questions
- Inconsistent docs across teams

### Current Workarounds

- External documentation (drifts from schema)
- Custom protoc plugins (non-standard)
- Wiki pages (maintenance burden)

## Detailed Design

### Extended Comment Syntax

Support doc comments with structured annotations:

```protobuf
/**
 * User represents a person in the system.
 *
 * Users are created via the {@link UserService.CreateUser} RPC and can be
 * retrieved using their {@link #uuid_id}.
 *
 * ## Lifecycle
 *
 * 1. Created with `CreateUser`
 * 2. Updated with `UpdateUser`
 * 3. Deleted with `DeleteUser`
 *
 * @example
 * ```protobuf
 * User user = {
 *   uuid_id: "550e8400-e29b-41d4-a716-446655440000",
 *   email: "user@example.com",
 *   name: "Alice Smith"
 * };
 * ```
 *
 * @see UserService
 * @since v1.0.0
 * @deprecated v3.0.0 Use UserV2 instead
 */
message User {
  /// The user's unique identifier.
  ///
  /// @format uuid
  /// @required
  string uuid_id = 1;

  /// Primary email address for notifications.
  ///
  /// @format email
  /// @example "user@example.com"
  string email = 2;

  /// Display name shown in UI.
  ///
  /// @minLength 1
  /// @maxLength 100
  string name = 3;
}
```

### Supported Annotations

#### Type-Level Annotations
```protobuf
/**
 * @see OtherMessage           - Link to related type
 * @since v1.0.0               - Version introduced
 * @deprecated v3.0.0 Reason   - Deprecation info
 * @author team-name           - Responsible team
 * @example {...}              - Usage example
 */
```

#### Field-Level Annotations
```protobuf
/// @format email|uuid|uri|datetime - Format hint
/// @required                       - Required field
/// @example "value"                - Example value
/// @default "value"                - Default value
/// @minLength N                    - Minimum length
/// @maxLength N                    - Maximum length
/// @min N                          - Minimum value
/// @max N                          - Maximum value
/// @pattern "regex"                - Validation pattern
```

### Cross-References

```protobuf
/// See {@link UserService.GetUser} for retrieval.
/// This field is related to {@link Organization#members}.
/// Returns a {@link User} object.
```

Link syntax:
- `{@link MessageName}` - Link to message
- `{@link ServiceName.MethodName}` - Link to RPC
- `{@link MessageName#field_name}` - Link to field
- `{@link #field_name}` - Link to field in same message

### Markdown Support

```protobuf
/**
 * # Overview
 *
 * This service handles **user management** operations.
 *
 * ## Features
 *
 * - Create users
 * - Update profiles
 * - Delete accounts
 *
 * ## Code Example
 *
 * ```python
 * user = User(name="Alice", email="alice@example.com")
 * client.CreateUser(CreateUserRequest(user=user))
 * ```
 *
 * > **Note:** All operations require authentication.
 */
service UserService {
  // ...
}
```

### Annotation Extensions

Alternative structured approach:

```protobuf
message User {
  string email = 1 [
    (doc.description) = "Primary contact email",
    (doc.example) = "user@example.com",
    (doc.format) = "email",
    (doc.required) = true,
    (doc.since) = "v1.0.0"
  ];

  string name = 2 [
    (doc.description) = "Display name",
    (doc.min_length) = 1,
    (doc.max_length) = 100
  ];
}
```

### Generated Documentation

#### HTML Output

```bash
protoc --doc_out=./docs --doc_opt=html,index.html *.proto
```

Generates:
- Searchable HTML documentation
- Cross-linked types
- Table of contents
- Version info

#### Markdown Output

```bash
protoc --doc_out=./docs --doc_opt=markdown,api.md *.proto
```

#### JSON Output (For Custom Renderers)

```bash
protoc --doc_out=./docs --doc_opt=json,api.json *.proto
```

```json
{
  "files": [{
    "name": "user.proto",
    "messages": [{
      "name": "User",
      "description": "User represents a person...",
      "since": "v1.0.0",
      "fields": [{
        "name": "email",
        "type": "string",
        "description": "Primary contact email",
        "example": "user@example.com",
        "format": "email"
      }]
    }]
  }]
}
```

### Documentation Website

Generate a static documentation site:

```bash
# Generate documentation site
protoc --doc_site_out=./site *.proto

# Output structure:
# site/
# ├── index.html
# ├── messages/
# │   ├── User.html
# │   └── Organization.html
# ├── services/
# │   └── UserService.html
# ├── search.json
# └── assets/
```

Features:
- Search functionality
- Dark/light mode
- Responsive design
- Version selector
- Code examples with syntax highlighting

## Example Usage

### Documenting a Service

```protobuf
/**
 * UserService provides operations for managing user accounts.
 *
 * ## Authentication
 *
 * All methods require a valid JWT token in the `Authorization` header.
 *
 * ## Rate Limiting
 *
 * - `GetUser`: 100 requests/minute
 * - `ListUsers`: 10 requests/minute
 * - `CreateUser`: 5 requests/minute
 *
 * @see AuthService for authentication
 * @since v1.0.0
 */
service UserService {
  /**
   * GetUser retrieves a user by ID.
   *
   * @example
   * ```
   * request: { user_id: "123" }
   * response: { user: { name: "Alice", email: "alice@example.com" } }
   * ```
   *
   * @throws NOT_FOUND if user doesn't exist
   * @throws PERMISSION_DENIED if not authorized
   */
  rpc GetUser(GetUserRequest) returns (GetUserResponse);

  /**
   * ListUsers returns all users matching criteria.
   *
   * Results are paginated with max 100 per page.
   */
  rpc ListUsers(ListUsersRequest) returns (stream User);
}
```

### Documenting Enums

```protobuf
/**
 * UserStatus represents the lifecycle state of a user.
 *
 * ```
 * PENDING → ACTIVE → SUSPENDED → DELETED
 *                  ↘           ↗
 *                    INACTIVE
 * ```
 */
enum UserStatus {
  /// Default value, should not be used.
  USER_STATUS_UNSPECIFIED = 0;

  /// User has registered but not verified email.
  PENDING = 1;

  /// User is active and can use the system.
  ACTIVE = 2;

  /// User temporarily disabled by admin.
  SUSPENDED = 3;

  /// User chose to deactivate account.
  INACTIVE = 4;

  /// User permanently deleted.
  DELETED = 5;
}
```

## Implementation Plan

### Week 1: Parser Extensions
- [ ] Extend comment parser for annotations
- [ ] Implement Markdown parsing
- [ ] Cross-reference resolution

### Week 2: Documentation Generator
- [ ] HTML template engine
- [ ] Markdown output
- [ ] JSON output

### Week 3: Static Site Generator
- [ ] Site template
- [ ] Search indexing
- [ ] Asset bundling

### Week 4: Tooling Integration
- [ ] VS Code hover documentation
- [ ] IntelliJ integration
- [ ] CLI improvements

### Week 5: Testing and Polish
- [ ] Comprehensive tests
- [ ] Example documentation
- [ ] User guide

## Backwards Compatibility

### Fully Compatible
- Existing comments continue to work
- New annotations are optional
- Non-breaking additions

### Gradual Adoption
- Add annotations incrementally
- Generate docs for partially annotated protos
- Best effort for missing info

## Alternatives Considered

### Alternative 1: External Documentation
- **Pro:** Full control
- **Con:** Drifts from schema
- **Decision:** Inline is better

### Alternative 2: OpenAPI/Swagger Style
- **Pro:** Familiar to web developers
- **Con:** Verbose, external file
- **Decision:** Inline annotations cleaner

### Alternative 3: Only Protoc Plugin
- **Pro:** Simpler implementation
- **Con:** Fragmented ecosystem
- **Decision:** Official support better

## Open Questions

1. **Markdown flavor** - CommonMark or GFM?
   - Suggestion: CommonMark with extensions

2. **Versioned docs** - Support multiple versions?
   - Suggestion: Yes, with version selector

3. **Localization** - Support translations?
   - Suggestion: Future enhancement

## Success Criteria

- [ ] All annotation types implemented
- [ ] HTML/Markdown/JSON output working
- [ ] Cross-references resolved
- [ ] Search working in static site
- [ ] Editor hover documentation

## Effort Estimation

| Task | Days |
|------|------|
| Comment parser | 5 |
| Documentation generator | 6 |
| Static site generator | 5 |
| Editor integration | 3 |
| Testing | 4 |
| Documentation | 3 |
| **Total** | **26** (5 weeks) |

---

## References

- [Javadoc](https://docs.oracle.com/javase/8/docs/technotes/tools/windows/javadoc.html)
- [JSDoc](https://jsdoc.app/)
- [Rust doc](https://doc.rust-lang.org/rustdoc/)
- [protoc-gen-doc](https://github.com/pseudomuto/protoc-gen-doc)
