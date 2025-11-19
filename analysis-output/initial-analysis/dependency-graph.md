# Protocol Buffers Dependency Graph

**Analysis Commit:** `ea940efd2c20e4e8b6509153a703175a51e66749`

## Visual Dependency Map

```
                    ┌─────────────────────────────────────────┐
                    │           Protocol Buffers              │
                    │              (Root)                      │
                    └────────────────┬────────────────────────┘
                                     │
        ┌────────────────────────────┼────────────────────────────┐
        │                            │                            │
        ▼                            ▼                            ▼
┌───────────────┐          ┌─────────────────┐          ┌─────────────────┐
│  Build Tools  │          │  Core Runtime   │          │   Generators    │
└───────┬───────┘          └────────┬────────┘          └────────┬────────┘
        │                           │                            │
        ▼                           ▼                            ▼
┌───────────────┐          ┌─────────────────┐          ┌─────────────────┐
│ • Bazel 7.0+  │          │ • abseil-cpp    │          │ • C++ Generator │
│ • CMake 3.16+ │          │ • utf8_range    │          │ • Java Generator│
│ • Python 3.9+ │          │ • zlib          │          │ • Python Gen    │
│ • GCC/Clang   │          │                 │          │ • ... 7 more    │
└───────────────┘          └─────────────────┘          └─────────────────┘
```

## Core C++ Dependencies

### Build-Time Dependencies

```
┌─────────────────────────────────────────────────────────────────┐
│                       abseil-cpp (LTS)                          │
│                    Version: 20250512.1                          │
├─────────────────────────────────────────────────────────────────┤
│  Components Used:                                               │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐              │
│  │ absl/base   │  │ absl/strings│  │ absl/hash   │              │
│  └─────────────┘  └─────────────┘  └─────────────┘              │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐              │
│  │absl/container│ │ absl/status │  │ absl/log    │              │
│  └─────────────┘  └─────────────┘  └─────────────┘              │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐              │
│  │ absl/types  │  │absl/synchron│  │ absl/time   │              │
│  └─────────────┘  └─────────────┘  └─────────────┘              │
└─────────────────────────────────────────────────────────────────┘
```

### Runtime Dependencies

| Dependency | Version | Purpose | License |
|------------|---------|---------|---------|
| abseil-cpp | 20250512.1 LTS | Core utilities, containers, logging | Apache-2.0 |
| zlib | 1.3.1 | Compression support | zlib License |
| utf8_range | bundled | UTF-8 validation | Apache-2.0 |

### Testing Dependencies

| Dependency | Version | Purpose | License |
|------------|---------|---------|---------|
| GoogleTest | 1.15.2 | C++ unit testing | BSD-3-Clause |
| benchmark | latest | Performance benchmarks | Apache-2.0 |

## Language-Specific Dependencies

### Java Dependencies

```
┌─────────────────────────────────────────────────────────────────┐
│                        Java Runtime                             │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Runtime:                                                       │
│  ┌─────────────────┐  ┌─────────────────┐                       │
│  │ guava 32.0.1    │  │ gson 2.8.9      │                       │
│  │ (utilities)     │  │ (JSON support)  │                       │
│  └─────────────────┘  └─────────────────┘                       │
│                                                                 │
│  Testing:                                                       │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐  │
│  │ JUnit 4.13.2    │  │ Mockito 4.3.1   │  │ Truth 1.1.3     │  │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘  │
│                                                                 │
│  Build:                                                         │
│  ┌─────────────────┐  ┌─────────────────┐                       │
│  │ rules_jvm_ext   │  │ Maven Central   │                       │
│  └─────────────────┘  └─────────────────┘                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Python Dependencies

```
┌─────────────────────────────────────────────────────────────────┐
│                       Python Runtime                            │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Runtime:                                                       │
│  ┌─────────────────┐  ┌─────────────────┐                       │
│  │ absl-py 2.x     │  │ setuptools      │                       │
│  │ (Google utils)  │  │ ≤78.1.1         │                       │
│  └─────────────────┘  └─────────────────┘                       │
│                                                                 │
│  Optional:                                                      │
│  ┌─────────────────┐                                            │
│  │ numpy ≤2.3.4    │                                            │
│  │ (array support) │                                            │
│  └─────────────────┘                                            │
│                                                                 │
│  Testing:                                                       │
│  ┌─────────────────┐  ┌─────────────────┐                       │
│  │ pytest          │  │ unittest        │                       │
│  └─────────────────┘  └─────────────────┘                       │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

### Ruby Dependencies

| Dependency | Version | Purpose |
|------------|---------|---------|
| bigdecimal | - | Numeric handling |
| ffi | ≥1.15 | Foreign function interface |
| rake-compiler | ≥1.2.5 | Native extension building |
| test-unit | - | Testing framework |

### Rust Dependencies

| Dependency | Version | Purpose |
|------------|---------|---------|
| googletest | 0.12.0 | Testing framework |
| paste | 1 | Macro utilities |

## Build System Dependencies

```
┌─────────────────────────────────────────────────────────────────┐
│                      Bazel Build Rules                          │
├─────────────────────────────────────────────────────────────────┤
│                                                                 │
│  Platform Rules:                                                │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐  │
│  │rules_cc 0.1.1   │  │rules_java 8.7.2 │  │rules_python     │  │
│  │                 │  │                 │  │0.40.0           │  │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘  │
│                                                                 │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐  │
│  │rules_ruby 0.13.1│  │rules_rust 0.56.0│  │rules_pkg 0.10.1 │  │
│  │                 │  │                 │  │                 │  │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘  │
│                                                                 │
│  Support Rules:                                                 │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐  │
│  │apple_support    │  │rules_shell      │  │rules_testing    │  │
│  │1.17.1           │  │                 │  │                 │  │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘  │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

## Internal Dependency Flow

```
                    ┌─────────────────┐
                    │  .proto Files   │
                    └────────┬────────┘
                             │
                             ▼
                    ┌─────────────────┐
                    │     Parser      │
                    │  (parser.cc)    │
                    └────────┬────────┘
                             │
                             ▼
              ┌──────────────────────────────┐
              │      Descriptor System        │
              │  (descriptor.h, descriptor.cc)│
              └──────────────┬───────────────┘
                             │
           ┌─────────────────┼─────────────────┐
           │                 │                 │
           ▼                 ▼                 ▼
    ┌─────────────┐   ┌─────────────┐   ┌─────────────┐
    │  Reflection │   │    Code     │   │   Runtime   │
    │     API     │   │  Generator  │   │  Libraries  │
    └─────────────┘   └──────┬──────┘   └─────────────┘
                             │
         ┌───────────────────┼───────────────────┐
         │                   │                   │
         ▼                   ▼                   ▼
  ┌─────────────┐     ┌─────────────┐     ┌─────────────┐
  │   C++ Gen   │     │  Java Gen   │     │ Python Gen  │
  │  (cpp/*.cc) │     │ (java/*.cc) │     │(python/*.cc)│
  └─────────────┘     └─────────────┘     └─────────────┘
```

## Version Pinning Strategy

### Strictly Pinned
These dependencies have exact version requirements:

| Dependency | Version | Reason |
|------------|---------|--------|
| rules_ruby | 0.13.1 | API stability |
| apple_support | 1.17.1 | macOS compatibility |
| abseil-cpp | 20250512.1 LTS | LTS compatibility guarantee |

### Loosely Pinned
These allow minor version flexibility:

| Dependency | Constraint | Reason |
|------------|------------|--------|
| absl-py | 2.x | Major version stability |
| Python | 3.9-3.14 | Supported range |
| Java | 8+ | Minimum requirement |

### Unpinned
These use latest compatible versions:

- GoogleTest (testing only)
- benchmark (benchmarking only)
- Development tools

## Potential Dependency Issues

### Abandoned/Deprecated Concerns
- **None critical** - All major dependencies actively maintained

### Security Considerations
- **zlib**: Monitor for CVEs (historically had vulnerabilities)
- **gson**: Ensure 2.8.9+ for security fixes

### Heavyweight Dependencies

| Dependency | Size Impact | Lighter Alternative |
|------------|-------------|---------------------|
| guava (Java) | ~2.8MB | Could use only needed modules |
| abseil-cpp | Varies | Used extensively, hard to replace |

### License Compatibility

All dependencies use permissive licenses compatible with BSD-3-Clause:
- Apache-2.0 ✓
- BSD-3-Clause ✓
- zlib License ✓
- MIT ✓

## Dependency Update Recommendations

### High Priority
1. Keep abseil-cpp on latest LTS
2. Monitor zlib for security patches
3. Update GoogleTest for new features

### Medium Priority
1. Consider guava modularization for Java
2. Evaluate newer Python packaging tools
3. Keep Bazel rules updated

### Low Priority
1. Development tool updates
2. Optional dependency updates
3. Documentation tool updates

---

*This dependency analysis is part of a comprehensive codebase review. See other files in `/analysis-output/` for complete documentation.*
