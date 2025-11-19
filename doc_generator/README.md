# Protocol Buffer Documentation Generator

A protoc plugin that generates rich API documentation from proto files with enhanced comment syntax supporting Markdown, cross-references, annotations, and multiple output formats.

## Features

- **Enhanced Comment Syntax**: Support for Javadoc-style `/** */` and `///` doc comments
- **Markdown Support**: Full Markdown formatting in descriptions
- **Cross-References**: Link to other types with `{@link TypeName}` syntax
- **Structured Annotations**: Version info, deprecation, validation constraints
- **Multiple Output Formats**: HTML, Markdown, JSON, and static site
- **Proto Extensions**: Optional structured documentation via proto options

## Installation

### Building from Source

```bash
# Using Bazel
bazel build //doc_generator:protoc-gen-doc

# The binary will be at bazel-bin/doc_generator/protoc-gen-doc
```

## Usage

### Basic Usage

```bash
# Generate HTML documentation
protoc --plugin=protoc-gen-doc=./protoc-gen-doc \
       --doc_out=./docs \
       --doc_opt=html,api *.proto

# Generate Markdown
protoc --doc_out=./docs --doc_opt=markdown,api *.proto

# Generate JSON
protoc --doc_out=./docs --doc_opt=json,api *.proto

# Generate static site
protoc --doc_out=./site --doc_opt=site *.proto
```

### Options

Options are passed via `--doc_opt` as comma-separated key=value pairs:

| Option | Description | Default |
|--------|-------------|---------|
| `html=filename` | Generate HTML output | `documentation` |
| `markdown=filename` | Generate Markdown output | `documentation` |
| `json=filename` | Generate JSON output | `documentation` |
| `site` | Generate static site | - |
| `title=text` | Set documentation title | `API Documentation` |
| `version=text` | Set version string | - |
| `dark` | Enable dark mode for HTML | false |

Example:
```bash
protoc --doc_opt=html=api,title=My API,version=v1.0.0,dark *.proto
```

## Comment Syntax

### Doc Comments

Use `/** */` for block comments or `///` for line comments:

```protobuf
/**
 * User represents a person in the system.
 *
 * Users can be created, updated, and deleted.
 */
message User {
  /// The user's unique identifier.
  string id = 1;
}
```

### Markdown Formatting

Full Markdown support in descriptions:

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
 * ```python
 * user = User(name="Alice", email="alice@example.com")
 * ```
 *
 * > **Note:** All operations require authentication.
 */
service UserService {
  // ...
}
```

### Cross-References

Link to other types and fields:

```protobuf
/**
 * See {@link UserService.GetUser} for retrieval.
 * This field is related to {@link Organization#members}.
 * Returns a {@link User} object.
 * Check the {@link #email} field for contact info.
 */
```

Link syntax:
- `{@link MessageName}` - Link to message
- `{@link ServiceName.MethodName}` - Link to RPC
- `{@link MessageName#field_name}` - Link to field
- `{@link #field_name}` - Link to field in same message

### Annotations

#### Type-Level Annotations

```protobuf
/**
 * Description here.
 *
 * @see OtherMessage           - Link to related type
 * @since v1.0.0               - Version introduced
 * @deprecated v3.0.0 Reason   - Deprecation info
 * @author team-name           - Responsible team
 */
```

#### Field-Level Annotations

```protobuf
/// Description here.
///
/// @format email              - Format hint
/// @required                  - Required field
/// @example "user@example.com" - Example value
/// @default "value"           - Default value
/// @minLength 1               - Minimum length
/// @maxLength 100             - Maximum length
/// @min 0                     - Minimum value
/// @max 150                   - Maximum value
/// @pattern "^[A-Z]+$"        - Validation regex
```

#### Method-Level Annotations

```protobuf
/**
 * Get user by ID.
 *
 * @throws NOT_FOUND if user doesn't exist
 * @throws PERMISSION_DENIED if not authorized
 */
rpc GetUser(GetUserRequest) returns (GetUserResponse);
```

### Code Examples

Include code examples with syntax highlighting:

```protobuf
/**
 * @example
 * ```python
 * user = User(name="Alice")
 * response = client.CreateUser(CreateUserRequest(user=user))
 * ```
 */
```

## Proto Extensions (Alternative)

For structured metadata, use the provided proto extensions:

```protobuf
import "doc_generator/doc_options.proto";

message User {
  string email = 1 [
    (google.protobuf.doc.field_doc) = {
      description: "Primary contact email"
      example: "user@example.com"
      format: "email"
      required: true
      since: "v1.0.0"
    }
  ];
}
```

## Output Formats

### HTML

Generates a single HTML file with:
- Responsive design
- Table of contents navigation
- Search functionality (in site mode)
- Dark/light mode support
- Code syntax highlighting
- Cross-linked types

### Markdown

Generates a Markdown file suitable for:
- GitHub/GitLab wikis
- Static site generators
- README files

### JSON

Generates structured JSON for:
- Custom documentation renderers
- API documentation portals
- Tooling integration

Example output:
```json
{
  "title": "API Documentation",
  "version": "v1.0.0",
  "files": [{
    "name": "user.proto",
    "package": "example.users.v1",
    "messages": [{
      "name": "User",
      "fullName": "example.users.v1.User",
      "description": "User represents a person...",
      "since": "v1.0.0",
      "children": [{
        "name": "email",
        "type": "field",
        "description": "Primary contact email",
        "format": "email",
        "required": true
      }]
    }]
  }]
}
```

### Static Site

Generates a complete documentation website with:
- `index.html` - Main documentation
- `search.json` - Search index
- Searchable interface
- Version selector support

## Best Practices

### 1. Document Everything

Add descriptions to all messages, fields, enums, services, and methods:

```protobuf
/// Brief description on one line.
string field = 1;

/**
 * Longer description that needs
 * multiple lines of explanation.
 */
message Complex {
  // ...
}
```

### 2. Use Meaningful Examples

Provide realistic examples that users can copy:

```protobuf
/// @example "user@example.com"
string email = 1;

/// @example ["admin", "user", "guest"]
repeated string roles = 2;
```

### 3. Document Deprecations Properly

Always provide migration guidance:

```protobuf
/// @deprecated v2.0.0 Use email_address instead
string email = 1;
```

### 4. Add Version Information

Track when features were added:

```protobuf
/// @since v1.2.0
optional string optional_field = 10;
```

### 5. Link Related Types

Help users navigate the API:

```protobuf
/// Returns a {@link User}. See {@link UserService} for related operations.
message GetUserResponse {
  User user = 1;
}
```

### 6. Document Error Conditions

For RPC methods, list possible errors:

```protobuf
/**
 * @throws NOT_FOUND if user doesn't exist
 * @throws INVALID_ARGUMENT if ID format is wrong
 * @throws PERMISSION_DENIED if caller lacks access
 */
rpc GetUser(GetUserRequest) returns (GetUserResponse);
```

## Integration

### CI/CD Pipeline

Add documentation generation to your CI:

```yaml
# .github/workflows/docs.yml
name: Generate Docs
on: [push]
jobs:
  docs:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Generate documentation
        run: |
          protoc --doc_out=./docs --doc_opt=site *.proto
      - name: Deploy to Pages
        uses: peaceiris/actions-gh-pages@v3
        with:
          github_token: ${{ secrets.GITHUB_TOKEN }}
          publish_dir: ./docs
```

### Pre-commit Hook

Validate documentation on commit:

```bash
#!/bin/bash
protoc --doc_out=/tmp --doc_opt=json,test *.proto
if [ $? -ne 0 ]; then
  echo "Documentation generation failed"
  exit 1
fi
```

## Troubleshooting

### Common Issues

**Issue**: Cross-references not resolving
- Ensure the target type is in the same proto file or imported
- Use the full path: `{@link package.MessageName}`

**Issue**: Markdown not rendering
- Check for proper spacing around headers
- Ensure code blocks use triple backticks

**Issue**: Annotations not parsed
- Annotations must be on their own line
- Use the exact format: `@tag value`

## License

Copyright 2024 Google LLC. Licensed under BSD-style license.
