# Protocol Buffers (Protobuf) - Dependencies and Technology Stack Analysis

## Overview
- **Project Version**: 6.34.0 (C++/Python), 4.34.0 (Java)
- **Project Type**: Cross-platform serialization framework
- **Primary Build System**: Bazel (with CMake and Maven support)
- **Repository**: https://github.com/protocolbuffers/protobuf

---

## 1. BUILD SYSTEM DEPENDENCIES

### Primary Build System: Bazel
**Configuration Files**:
- WORKSPACE (legacy) - main dependency declaration file
- WORKSPACE.bzlmod - empty, used with MODULE.bazel for Bzlmod migration
- MODULE.bazel - Bazel modules and dependency declarations

**Bazel Build Rules Dependencies**:
| Dependency | Version | Purpose |
|------------|---------|---------|
| rules_proto | 7.1.0 | Proto language toolchain rules |
| rules_cc | 0.0.17 | C++ rules for compiling and linking |
| rules_java | 8.6.1 | Java compilation and packaging rules |
| rules_python | 1.6.0 | Python rules for build isolation |
| rules_jvm_external | 6.7 | Maven artifact resolution for JVM |
| rules_kotlin | 1.9.6 | Kotlin language support |
| rules_apple | 3.16.0 | Apple platform (iOS, macOS) support |
| apple_support | 1.15.1 | Apple-specific build support |
| rules_pkg | 1.0.1 | Package distribution generation |
| rules_shell | 0.2.0 | Shell script rules |
| bazel_skylib | 1.7.1 | Bazel standard library utilities |
| bazel_features | 1.33.0 | Bazel feature detection |
| platforms | 0.0.11 | Platform definitions and constraints |
| rules_license | 1.0.0 | License rule for declarations |
| rules_fuzzing | 0.5.3 | Fuzzing test support (dev) |
| rules_rust | 0.63.0 | Rust language support (dev) |
| rules_ruby | 0.17.3 | Ruby language support with patches |
| rules_buf | 0.3.0 | Buf CLI integration (dev) |
| rules_testing | 0.9.0 | Testing utilities (dev) |

### Alternative Build System: CMake
**Minimum Version**: 3.16 with policy support up to 3.26
**Key Build Options**:
- `protobuf_BUILD_TESTS` - Enable test compilation
- `protobuf_BUILD_SHARED_LIBS` - Build shared libraries (default: OFF)
- `protobuf_BUILD_PROTOC_BINARIES` - Build protoc compiler (default: ON)
- `protobuf_WITH_ZLIB` - Compression support (default: ON)
- `protobuf_BUILD_LIBUPB` - Micro protobuf library (default: ON)

**CMake Build Files Located in**: `/cmake/`
- `abseil-cpp.cmake` - Abseil dependency setup
- `gtest.cmake` - GoogleTest framework setup
- `utf8_range.cmake` - UTF-8 validation library
- `dependencies.cmake` - Auto-generated dependency versions
- `libprotobuf.cmake` - Main protobuf library
- `libprotobuf-lite.cmake` - Lightweight protobuf library
- `libupb.cmake` - UPB C implementation
- `libprotoc.cmake` - Protoc compiler library
- `tests.cmake` - Test definitions
- `protoc.cmake` - Protoc executable

### Package Manager: Maven
**Maven Repositories**:
- `https://repo1.maven.org/maven2`
- `https://repo.maven.apache.org/maven2`

---

## 2. EXTERNAL C++ DEPENDENCIES

### Core Runtime Dependencies

| Dependency | Version | Purpose | Source |
|------------|---------|---------|--------|
| **abseil-cpp** | 20250512.1 (LTS May 2025) | Foundational C++ library (containers, logging, synchronization) | GitHub: abseil/abseil-cpp |
| **zlib** | 1.3.1 | Compression/decompression support | GitHub: madler/zlib |
| **jsoncpp** | 1.9.6 | JSON format support for protobuf | GitHub: open-source-parsers/jsoncpp |

**Abseil-cpp Components Used** (from abseil-cpp.cmake):
- `absl::strings` - String utilities
- `absl::absl_check` - Assertion checking
- `absl::absl_log` - Logging framework
- `absl::status` / `absl::statusor` - Error handling
- `absl::flat_hash_map` / `absl::flat_hash_set` - Hash containers
- `absl::btree` - B-tree containers
- `absl::cord` - Rope-like string type
- `absl::time` - Time utilities
- `absl::random_*` - Random number generation
- `absl::synchronization` - Threading primitives
- `absl::span` - Array span views
- `absl::algorithm` - Algorithm utilities

### Testing Dependencies

| Dependency | Version | Purpose | Scope |
|------------|---------|---------|-------|
| **GoogleTest** | 1.15.2 (dev) | C++ unit testing framework | Development |
| **google_benchmark** | 1.9.2 (dev) | Performance benchmarking | Development |

### Platform-Specific Support

| Dependency | Platform | Purpose |
|------------|----------|---------|
| **Lua** | 5.4.6 | UPB Lua bindings (dev) |
| **rules_apple** | 3.16.0 | Apple platform toolchain |
| **apple_support** | 1.15.1 | iOS/macOS specific build support |

**Compatibility Testing**:
- `com_google_protobuf_v25` (25.0) - Previous release version for compatibility testing
- `com_google_protobuf_previous_release` (29.0) - Breaking changes detection

---

## 3. JAVA DEPENDENCIES

### Runtime Dependencies

| Dependency | Version | Scope | Purpose |
|------------|---------|-------|---------|
| **guava** | 32.0.1-jre/android | compile/test | Google utilities library (collections, caching, etc.) |
| **jsr305** | 3.0.2 | compile | Annotations for null safety |
| **error_prone_annotations** | 2.18.0 | compile | Error-prone checker annotations |
| **j2objc-annotations** | 2.8 | compile | J2ObjC compatibility annotations |
| **gson** | 2.8.9 | compile | JSON serialization |
| **checker-qual** | 3.33.0 | compile | Type checker framework |
| **listenablefuture** | 9999.0 | compile | Future utilities (empty shim) |
| **failureaccess** | 1.0.1 | compile | Failure utility for Guava |

### Test Dependencies

| Dependency | Version | Scope | Purpose |
|------------|---------|-------|---------|
| **junit** | 4.13.2 | test | Unit testing framework |
| **mockito-core** | 4.3.1 | test | Mocking framework |
| **guava-testlib** | 32.0.1-jre | test | Guava testing utilities |
| **truth** | 1.1.2 | test | Assertion library for tests |
| **test-parameter-injector** | 1.18 | test | Parameter injection for tests |
| **caliper** | 1.0-beta-3 | test | Micro-benchmarking framework |
| **biz.aQute.bndlib** | 6.4.0 | test | OSGi bundle utilities |
| **picocli** | 4.6.3 | test | CLI framework utilities |

### Kotlin Support

| Dependency | Version | Purpose |
|------------|---------|---------|
| **Kotlin** | 1.6.0+ | Language support |

**Kotlin Build**: Uses `rules_kotlin` with Java interop

### Java Build Configuration

**Maven Compiler**: Java 1.8 source/target
**Distribution**: Maven Central (Sonatype Nexus)
**BOM (Bill of Materials)**: `protobuf-bom` for version management

**Java Modules**:
- `protobuf-java` - Core runtime
- `protobuf-java-util` - Utilities (JSON formatting)
- `protobuf-java-lite` - Lightweight for Android
- `protobuf-kotlin` - Kotlin bindings
- `protobuf-kotlin-lite` - Kotlin lightweight

---

## 4. PYTHON DEPENDENCIES

### Runtime Dependencies

**From `python/requirements.txt`**:
```
numpy<=2.3.4
setuptools<=78.1.1
absl-py==2.*
```

| Dependency | Version Constraint | Purpose |
|------------|------------------|---------|
| **numpy** | ≤2.3.4 | Numerical computing support (for extension modules) |
| **absl-py** | 2.x | Abseil Python library (logging, testing) |
| **setuptools** | ≤78.1.1 | Build and distribution utilities |

### Python Build System

**setup.py Configuration** (`python/dist/setup.py`):
- Uses C extension modules compiled from upb C code
- Supports platforms: Linux, Windows, macOS
- Requires Python ≥3.9
- Supported Python versions: 3.9, 3.10, 3.11, 3.12, 3.13, 3.14

**Extension Module**: `google._upb._message` (C extension)

**Bazel Python Support**:
- `rules_python` 1.6.0
- `system_python` for hermetic builds
- `pip` extension for dependency resolution

**Python Testing**: Uses built-in `unittest` framework

---

## 5. RUBY DEPENDENCIES

### Build and Test Dependencies

**From `ruby/Gemfile`**:

| Dependency | Version | Group | Purpose |
|------------|---------|-------|---------|
| **bigdecimal** | latest | runtime | Decimal number support |
| **ffi** | ~>1 | development/jruby | Foreign function interface |
| **ffi-compiler** | ~>1 | development/jruby | FFI compiler support |
| **rake** | ≥13 | development | Build automation |
| **rake-compiler** | ~>1.2 | development | Native C extension compiler |
| **rake-compiler-dock** | ~>1.9 | development | Docker for compilation |
| **test-unit** | ~>3.0, ≥3.0.9 | test | Unit testing framework |

**Ruby Build System**: 
- Uses `rules_ruby` 0.17.3 (with patches)
- Gem-based distribution
- Native C extensions via rake-compiler

---

## 6. GO DEPENDENCIES

**Build System**: `rules_go` (Bazel)
**Location**: `/go` directory
**Status**: Community-maintained via gRPC

---

## 7. OTHER LANGUAGE SUPPORT

### C#
**Location**: `/csharp` directory
**Status**: Community-maintained

### Objective-C
**Location**: `/objectivec` directory
**Build System**: Bazel with Apple rules

### PHP
**Location**: `/php` directory
**Build System**: Bazel
**Extension**: Native PHP extension using upb C implementation

### Lua
**Location**: `/lua` directory
**Build System**: Bazel
**Dependency**: Lua 5.4.6
**Runtime**: Uses upb C bindings

### Rust
**Location**: `/rust` directory
**Build System**: Bazel with `rules_rust` 0.63.0
**Crate Dependencies** (via `crate_universe`):
- `googletest` - Testing framework
- `paste` - Macro utilities
- `quote` - Quote generation
- `syn` - Syntax tree parsing

**Minimum Rust Version**: 1.79 (tested with 1.85.0 for edition 2024)

---

## 8. TESTING FRAMEWORKS

### By Language

| Language | Test Framework | Bazel Rule | Location |
|----------|---|---|---|
| **C++** | GoogleTest (gtest) | `cc_test` | `/src/google/protobuf/` |
| **Java** | JUnit 4 + Mockito | `junit_tests` | `/java/` |
| **Python** | unittest (stdlib) + pytest | `py_test` | `/python/` |
| **Ruby** | test-unit | Rake | `/ruby/` |
| **Rust** | googletest-rs | `rust_test` | `/rust/` |

### Specialized Testing

| Test Type | Tool | Purpose | Location |
|-----------|------|---------|----------|
| **Conformance** | Conformance runner | Cross-language compatibility | `/conformance/` |
| **Fuzzing** | rules_fuzzing 0.5.3 | Fuzz testing | Dev dependency |
| **Benchmarks** | google_benchmark | Performance measurement | `/benchmarks/` |
| **Protoc Plugin** | Test plugin injection | Compiler plugin testing | `/src/google/protobuf/compiler/` |

### Test Organization

**Test Execution Methods**:
1. **Bazel**: Native build system `bazel test`
2. **CMake**: `ctest` command with XML output support
3. **Maven**: `mvn test` for Java modules
4. **Direct**: Individual test runners per language

---

## 9. INTERNAL DEPENDENCIES

### upb (Micro Protocol Buffers)
**Location**: `/upb` directory
**Purpose**: 
- Lightweight C implementation of protobuf
- Core runtime for Ruby, PHP, and Python extensions
- No stable C ABI (internal use only)

**Key Features**:
- Fast parsing and serialization
- Minimal code size (~order of magnitude smaller than C++)
- Reflection support
- Binary & JSON wire formats
- Text format serialization

**Bootstrap Components**:
- `google/protobuf/descriptor.upb.h` - Generated descriptor definitions
- `google/protobuf/descriptor.upb_minitable.h` - Minimal table definitions

### utf8_range
**Location**: `/third_party/utf8_range` directory
**Version**: 1.3.1.bcr.5
**Purpose**: UTF-8 validation for string fields

### googleapis
**Location**: Bazel-downloaded version
**Version**: fe8ba054ad4f7eca946c2d14a63c3f07c0b586a0 (dev dependency)
**Purpose**: Well-known type definitions for benchmarks

---

## 10. BUILD DEPENDENCY VERSIONS (Auto-Generated)

**Auto-generated from `cmake/dependencies.cmake`**:

```
rules_apple-version: 3.16.0
apple_support-version: 1.15.1
abseil-cpp-version: 20250512.1
rules_cc-version: 0.0.17
zlib-version: 1.3.1.bcr.5
bazel_features-version: 1.33.0
bazel_skylib-version: 1.7.1
jsoncpp-version: 1.9.6
rules_java-version: 8.6.1
rules_jvm_external-version: 6.7
rules_kotlin-version: 1.9.6
rules_license-version: 1.0.0
rules_pkg-version: 1.0.1
rules_python-version: 1.6.0
rules_proto-version: 7.1.0
rules_rust-version: 0.63.0
rules_ruby-version: 0.17.3
rules_fuzzing-version: 0.5.3
rules_shell-version: 0.2.0
platforms-version: 0.0.11
re2-version: 2024-07-02.bcr.1
googletest-version: 1.15.2
rules_buf-version: 0.3.0
rules_testing-version: 0.9.0
abseil-py-version: 2.1.0
lua-version: 5.4.6
googleapis-version: 0.0.0-20240819-fe8ba054a
google_benchmark-version: 1.9.2
com_google_protobuf_v25-version: 25.0
com_google_protobuf_previous_release-version: 29.0
```

---

## 11. CROSS-PLATFORM CONSIDERATIONS

### C++ Compiler Support
- **MSVC** (Windows): Special preprocessing flags, static linking options
- **GCC/Clang** (Linux/macOS): Atomic library detection, version scripts

### CMake Platform Detection
- **Threads**: CMake FindThreads
- **ZLIB**: CMake FindZLIB with fallback fetch
- **Abseil**: CONFIG mode with GitHub fallback

### Bazel Platform Rules
- **Apple**: iOS, macOS, tvOS, watchOS support via `rules_apple`
- **Windows**: MSVC toolchain, shared/static library configuration
- **Linux**: Glibc/musl detection

---

## 12. DEPENDENCY MANAGEMENT STRATEGY

### Bazel Module Version Resolution
- **Strategy**: Minimum Version Selection (MVS)
- **Conflict Resolution**: Higher version wins
- **Lock Files**: `maven_install.json`, `maven_dev_install.json`

### Maven Dependency Management
**BOM Import Pattern**:
- Central `protobuf-bom` manages all internal protobuf artifacts
- Transitive dependencies through Guava
- Test dependencies isolated with `<scope>test</scope>`

### Python Dependency Pinning
- Uses upper bounds to manage compatibility
- absl-py: major version 2.x
- numpy: limited to ≤2.3.4

---

## 13. DEVELOPMENT VS PRODUCTION DEPENDENCIES

### Production (Public API)
- abseil-cpp
- zlib (optional)
- jsoncpp (for JSON support)
- Guava (Java)
- absl-py (Python)

### Development Only
- GoogleTest / googletest-rs
- google_benchmark
- rules_fuzzing
- rules_testing
- test-unit (Ruby)
- Kotlin compiler
- Rust toolchain (1.85.0 tested)
- All IDE/editor integrations

---

## 14. SUMMARY TABLE: Key Dependencies

| Category | Primary Dependency | Version | Why Needed |
|----------|------------------|---------|-----------|
| **C++ Runtime** | abseil-cpp | 20250512.1 | Foundational libraries (logging, containers) |
| **C++ Runtime** | zlib | 1.3.1 | Compression support |
| **C++ Build** | rules_cc | 0.0.17 | C++ compilation rules |
| **Java Runtime** | guava | 32.0.1 | Utility library |
| **Java Test** | junit | 4.13.2 | Unit testing |
| **Java Test** | mockito | 4.3.1 | Test mocking |
| **Python** | numpy | ≤2.3.4 | C extension support |
| **Python** | absl-py | 2.x | Logging framework |
| **All** | GoogleTest | 1.15.2 | C++ testing |
| **Build** | Bazel | 7.0+ | Primary build system |
| **Build** | rules_proto | 7.1.0 | Proto toolchain |
| **Build** | CMake | 3.16+ | Alternative build system |

---

## 15. VERSION PINNING & CONSTRAINTS

### Strict Pinning
- `rules_ruby` 0.17.3 with mandatory patches (bundler disable)
- `apple_support` 1.15.1 (workaround for issue #316)

### Version Ranges
- `absl-py` == 2.* (major version pinning)
- Guava: 32.0.1 Android edition
- Kotlin: 1.6.0+

### Compatibility Constraints
- CMake minimum: 3.16 (with C++ 17 requirement)
- Java: compiled for Java 1.8
- Python: 3.9+ (3.14 supported)
- Rust: minimum 1.79
