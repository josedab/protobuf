# RFC-0005: Developer Documentation Portal

**Status:** Draft
**Author:** Codebase Analysis
**Created:** November 19, 2025
**Effort:** 2 weeks
**Category:** Strategic

---

## Summary

Create a unified developer documentation portal that consolidates architecture guides, API references, tutorials, and contribution guidelines, reducing onboarding time and support burden.

## Motivation

### Problem Statement

Current documentation is scattered and incomplete:

1. **Scattered locations** - docs/, README files, external sites
2. **Missing guides** - No architecture overview for contributors
3. **Outdated content** - Some docs reference old APIs
4. **No search** - Hard to find specific information
5. **Language silos** - Each language has separate docs

### Evidence from Analysis

| Documentation Type | Location | Issue |
|-------------------|----------|-------|
| Design docs | `docs/design/` | Technical, not onboarding |
| Language guides | Each runtime dir | Inconsistent format |
| API reference | Scattered headers | Not navigable |
| Tutorials | External sites | Not maintained |

### Business Impact

- Increased support burden
- Slower contributor onboarding
- Repeated questions in issues
- Barrier to adoption

## Detailed Design

### Portal Architecture

```
docs.protobuf.dev/
├── /getting-started/        # Quick start guides
├── /concepts/               # Core concepts
├── /languages/              # Language-specific guides
│   ├── /cpp/
│   ├── /java/
│   └── /python/
├── /architecture/           # Codebase architecture
├── /api/                    # API reference
├── /contributing/           # Contribution guides
└── /search/                 # Full-text search
```

### Content Structure

#### 1. Getting Started
```markdown
# Getting Started

## Installation
- Package managers (apt, brew, etc.)
- Building from source
- Docker images

## Quick Start
- Your first .proto file
- Generating code
- Using generated code

## Next Steps
- [Language guides](/languages/)
- [Core concepts](/concepts/)
```

#### 2. Core Concepts
```markdown
# Core Concepts

## Messages and Fields
- Field types
- Field rules (optional, repeated)
- Default values

## Wire Format
- Encoding overview
- Varints
- Length-delimited

## Descriptors and Reflection
- Runtime metadata
- Dynamic messages
```

#### 3. Architecture (For Contributors)
```markdown
# Codebase Architecture

## Overview
[Diagram of layers]

## Compiler Pipeline
- Parser
- Descriptor construction
- Code generation

## Runtime Libraries
- C++ runtime
- Java runtime
- Python runtime

## Key Files
| Purpose | File |
|---------|------|
| Compiler entry | src/google/protobuf/compiler/main.cc |
| Message base | src/google/protobuf/message.h |
```

#### 4. Language Guides

Each language gets:
- Installation
- Quick start
- API reference
- Best practices
- FAQ

#### 5. API Reference

Auto-generated from source:
```markdown
# Class: google::protobuf::Message

Base class for all protocol buffer message types.

## Methods

### SerializeToString
```cpp
bool SerializeToString(std::string* output) const
```
Serializes the message to a string.

**Parameters:**
- `output`: String to write to

**Returns:** `true` on success
```

### Technology Stack

1. **Static site generator**: MkDocs with Material theme
2. **API docs**: Doxygen for C++, Javadoc for Java
3. **Search**: Algolia DocSearch
4. **Hosting**: GitHub Pages
5. **CI**: GitHub Actions for build/deploy

### Build Configuration

```yaml
# mkdocs.yml
site_name: Protocol Buffers Documentation
site_url: https://docs.protobuf.dev/
repo_url: https://github.com/protocolbuffers/protobuf

theme:
  name: material
  features:
    - navigation.tabs
    - navigation.sections
    - search.suggest
    - content.code.copy

plugins:
  - search
  - mkdocstrings  # API docs from source

nav:
  - Home: index.md
  - Getting Started:
    - Installation: getting-started/installation.md
    - Quick Start: getting-started/quickstart.md
  - Concepts:
    - Messages: concepts/messages.md
    - Wire Format: concepts/wire-format.md
  - Languages:
    - C++: languages/cpp/index.md
    - Java: languages/java/index.md
    - Python: languages/python/index.md
  - Architecture:
    - Overview: architecture/overview.md
    - Compiler: architecture/compiler.md
  - API Reference: api/index.md
  - Contributing: contributing/index.md
```

### GitHub Actions Workflow

```yaml
# .github/workflows/docs.yml
name: Documentation

on:
  push:
    branches: [main]
    paths:
      - 'docs/**'
      - 'mkdocs.yml'
  pull_request:
    paths:
      - 'docs/**'

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Setup Python
        uses: actions/setup-python@v4
        with:
          python-version: '3.11'

      - name: Install dependencies
        run: pip install mkdocs-material mkdocstrings

      - name: Build docs
        run: mkdocs build --strict

      - name: Deploy
        if: github.ref == 'refs/heads/main'
        run: mkdocs gh-deploy --force
```

### Search Integration

```javascript
// Algolia DocSearch configuration
docsearch({
  appId: 'YOUR_APP_ID',
  apiKey: 'YOUR_SEARCH_KEY',
  indexName: 'protobuf',
  container: '#search',
  searchParameters: {
    facetFilters: ['language:en']
  }
});
```

## Example Usage

### Finding Information

1. **Search**: Type "arena allocation" → direct link to performance guide
2. **Navigation**: Languages → C++ → Memory Management
3. **API lookup**: Search "SerializeToString" → method documentation

### Contributing Documentation

```bash
# Clone repo
git clone https://github.com/protocolbuffers/protobuf.git

# Install docs dependencies
pip install -r docs/requirements.txt

# Serve locally
mkdocs serve

# Preview at http://localhost:8000
```

## Implementation Plan

### Week 1: Infrastructure
- [ ] Set up MkDocs project
- [ ] Configure Material theme
- [ ] Set up GitHub Actions deploy
- [ ] Configure Algolia search

### Week 2: Content Migration
- [ ] Write getting started guide
- [ ] Create concept docs
- [ ] Migrate language guides
- [ ] Write architecture overview
- [ ] Set up API doc generation

## Backwards Compatibility

- Existing docs.protobuf.dev redirects preserved
- Old URLs redirect to new locations
- GitHub docs/ folder remains (source for portal)

## Alternatives Considered

### Alternative 1: ReadTheDocs
- **Pro:** Less infrastructure
- **Con:** Less customization
- **Decision:** MkDocs gives more control

### Alternative 2: Wiki
- **Pro:** Easy editing
- **Con:** No version control, poor search
- **Decision:** Rejected

### Alternative 3: In-Repo Only
- **Pro:** No external hosting
- **Con:** Poor discoverability
- **Decision:** Rejected - need proper site

## Open Questions

1. **Custom domain** - docs.protobuf.dev vs protobuf.dev/docs?
   - Suggestion: Subdomain for clear separation

2. **Multi-language** - Should docs be translated?
   - Suggestion: English first, translations later

3. **Version selector** - Support multiple protobuf versions?
   - Suggestion: Latest + LTS

## Success Criteria

- [ ] All key topics documented
- [ ] Search returning relevant results
- [ ] 50% reduction in "how do I" GitHub issues
- [ ] <5 min to find any topic
- [ ] Positive contributor feedback

## Effort Estimation

| Task | Days |
|------|------|
| Infrastructure setup | 2 |
| Getting started content | 2 |
| Concept docs | 2 |
| Language guides | 3 |
| Architecture docs | 2 |
| API reference setup | 1 |
| Search configuration | 1 |
| **Total** | **13** (2 weeks) |

---

## References

- [MkDocs Material](https://squidfunk.github.io/mkdocs-material/)
- [Algolia DocSearch](https://docsearch.algolia.com/)
- [Divio Documentation System](https://documentation.divio.com/)
