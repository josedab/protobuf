# C++ Installation

This guide covers installing the Protocol Buffers compiler and C++ runtime library.

## Pre-built Packages

### Package Managers

=== "Ubuntu/Debian"

    ```bash
    sudo apt update
    sudo apt install -y protobuf-compiler libprotobuf-dev
    ```

=== "Fedora"

    ```bash
    sudo dnf install protobuf-compiler protobuf-devel
    ```

=== "Arch Linux"

    ```bash
    sudo pacman -S protobuf
    ```

=== "macOS"

    ```bash
    brew install protobuf
    ```

=== "Windows (vcpkg)"

    ```powershell
    vcpkg install protobuf
    ```

### Verify Installation

```bash
protoc --version
```

## Building from Source

For the latest version or custom builds:

### Prerequisites

=== "Ubuntu/Debian"

    ```bash
    sudo apt install -y cmake g++ git
    ```

=== "macOS"

    ```bash
    brew install cmake
    ```

=== "Windows"

    Install Visual Studio with C++ support and CMake.

### Build Steps

```bash
# Clone repository
git clone https://github.com/protocolbuffers/protobuf.git
cd protobuf

# Configure with CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release \
    -Dprotobuf_BUILD_TESTS=OFF \
    -Dprotobuf_BUILD_SHARED_LIBS=ON

# Build
cmake --build build --parallel

# Install (requires sudo on Linux/macOS)
sudo cmake --install build
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `protobuf_BUILD_TESTS` | ON | Build test suite |
| `protobuf_BUILD_SHARED_LIBS` | OFF | Build shared libraries |
| `protobuf_BUILD_EXAMPLES` | OFF | Build examples |
| `protobuf_ABSL_PROVIDER` | package | Abseil provider (package/module) |

### Installing to Custom Location

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build build --parallel
cmake --install build

# Add to PATH
export PATH="$HOME/.local/bin:$PATH"
export LD_LIBRARY_PATH="$HOME/.local/lib:$LD_LIBRARY_PATH"
```

## CMake Integration

### Using find_package

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(myproject)

find_package(Protobuf REQUIRED)

add_executable(myapp main.cc)
target_link_libraries(myapp protobuf::libprotobuf)
```

Configure with protobuf location:

```bash
cmake -B build -DProtobuf_DIR=/path/to/protobuf/lib/cmake/protobuf
```

### Generating Protobuf Files

```cmake
find_package(Protobuf REQUIRED)

# Generate C++ from .proto files
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS
    proto/message1.proto
    proto/message2.proto
)

add_executable(myapp main.cc ${PROTO_SRCS})
target_link_libraries(myapp protobuf::libprotobuf)
target_include_directories(myapp PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
```

### FetchContent (Download During Build)

```cmake
include(FetchContent)

FetchContent_Declare(
  protobuf
  GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
  GIT_TAG        v25.1
)

set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(protobuf)

add_executable(myapp main.cc)
target_link_libraries(myapp protobuf::libprotobuf)
```

## Bazel Integration

### WORKSPACE

```python
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_google_protobuf",
    sha256 = "...",
    strip_prefix = "protobuf-25.1",
    urls = ["https://github.com/protocolbuffers/protobuf/archive/v25.1.tar.gz"],
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")
protobuf_deps()
```

### BUILD

```python
load("@rules_proto//proto:defs.bzl", "proto_library")
load("@rules_cc//cc:defs.bzl", "cc_proto_library")

proto_library(
    name = "person_proto",
    srcs = ["person.proto"],
)

cc_proto_library(
    name = "person_cc_proto",
    deps = [":person_proto"],
)

cc_binary(
    name = "main",
    srcs = ["main.cc"],
    deps = [":person_cc_proto"],
)
```

## Lite Runtime

For resource-constrained environments, use the lite runtime:

```protobuf
option optimize_for = LITE_RUNTIME;
```

Link with `-lprotobuf-lite` instead of `-lprotobuf`.

Limitations:
- No reflection
- No descriptors
- No text format
- Smaller binary size

## Troubleshooting

### Library Not Found

```bash
# Linux
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
sudo ldconfig

# macOS
export DYLD_LIBRARY_PATH=/usr/local/lib:$DYLD_LIBRARY_PATH
```

### Version Mismatch

Ensure protoc and library versions match:

```bash
protoc --version
# libprotoc 25.1

# Check installed library version in code
google::protobuf::internal::VerifyVersion(GOOGLE_PROTOBUF_VERSION, ...)
```

### CMake Can't Find Protobuf

```bash
# Specify installation path
cmake -B build -DProtobuf_DIR=/usr/local/lib/cmake/protobuf

# Or use CMAKE_PREFIX_PATH
cmake -B build -DCMAKE_PREFIX_PATH=/usr/local
```

## Next Steps

- [Quick Start](quickstart.md) - Create your first program
- [API Reference](api-reference.md) - Detailed API docs
- [Best Practices](best-practices.md) - Performance optimization
