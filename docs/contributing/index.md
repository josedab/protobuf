# Contributing to Protocol Buffers

Thank you for your interest in contributing to Protocol Buffers! This guide will help you get started.

## Ways to Contribute

- **Report bugs** - File issues for problems you find
- **Fix bugs** - Submit pull requests for bug fixes
- **Add features** - Propose and implement new features
- **Improve docs** - Enhance documentation
- **Review PRs** - Help review other contributions

## Getting Started

1. [Development Setup](setup.md) - Set up your development environment
2. [Code Style](style.md) - Follow our coding standards
3. [Testing Guide](testing.md) - Write and run tests
4. [Pull Request Process](pull-requests.md) - Submit your changes

## Quick Start

```bash
# Clone the repository
git clone https://github.com/protocolbuffers/protobuf.git
cd protobuf

# Build
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel

# Run tests
cd build && ctest --output-on-failure
```

## Contribution Guidelines

### Before You Start

1. **Check existing issues** - Your issue may already be reported
2. **Discuss major changes** - Open an issue to discuss first
3. **One PR per feature** - Keep changes focused

### Code Requirements

- Follow [code style](style.md) guidelines
- Add tests for new functionality
- Update documentation as needed
- Ensure all tests pass

### Commit Messages

Write clear, descriptive commit messages:

```
component: Brief description

More detailed explanation if needed. Explain the problem
being solved and why this approach was chosen.

Fixes #123
```

Examples:
- `cpp: Fix memory leak in arena allocation`
- `java: Add support for optional field presence`
- `docs: Update C++ installation guide`

## Areas for Contribution

### Good First Issues

Look for issues labeled:
- `good first issue`
- `help wanted`

### Documentation

- Fix typos and errors
- Add examples
- Improve clarity

### Code

- Bug fixes
- Performance improvements
- New features (discuss first)

### Tests

- Add missing tests
- Improve test coverage
- Fix flaky tests

## Communication

### Issues

- Use issue templates
- Provide reproduction steps
- Include version information

### Pull Requests

- Reference related issues
- Describe changes clearly
- Respond to review feedback

### Discussions

- Ask questions in [GitHub Discussions](https://github.com/protocolbuffers/protobuf/discussions)
- Be respectful and constructive

## Code of Conduct

We follow the [Contributor Covenant](https://www.contributor-covenant.org/). Please be respectful and professional in all interactions.

## Legal

### Contributor License Agreement

You must sign a [Contributor License Agreement (CLA)](https://cla.developers.google.com/) before your PR can be merged.

### License

Protocol Buffers is licensed under the [BSD 3-Clause License](https://github.com/protocolbuffers/protobuf/blob/main/LICENSE).

## Recognition

Contributors are recognized in:
- Git history
- Release notes for significant contributions

## Resources

- [GitHub Repository](https://github.com/protocolbuffers/protobuf)
- [Issue Tracker](https://github.com/protocolbuffers/protobuf/issues)
- [Pull Requests](https://github.com/protocolbuffers/protobuf/pulls)
- [Discussions](https://github.com/protocolbuffers/protobuf/discussions)

## Next Steps

- [Development Setup](setup.md) - Get your environment ready
- [Architecture Overview](../architecture/index.md) - Understand the codebase
