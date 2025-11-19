# Development Setup

This guide helps you set up a development environment for contributing to Protocol Buffers.

## Prerequisites

### All Platforms

- Git
- CMake 3.16+
- Python 3.7+ (for tests)

### Linux (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y \
  git \
  cmake \
  g++ \
  make \
  python3 \
  python3-pip
```

### macOS

```bash
brew install cmake git python
```

### Windows

1. Install [Visual Studio](https://visualstudio.microsoft.com/) with C++ support
2. Install [CMake](https://cmake.org/download/)
3. Install [Git](https://git-scm.com/download/win)
4. Install [Python](https://www.python.org/downloads/)

## Getting the Source

```bash
# Clone the repository
git clone https://github.com/protocolbuffers/protobuf.git
cd protobuf

# Initialize submodules
git submodule update --init --recursive
```

## Building

### CMake (Recommended)

```bash
# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build --parallel

# The protoc binary will be at build/protoc
```

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `protobuf_BUILD_TESTS` | ON | Build tests |
| `protobuf_BUILD_SHARED_LIBS` | OFF | Build shared libraries |
| `protobuf_BUILD_EXAMPLES` | OFF | Build examples |
| `protobuf_WITH_ZLIB` | ON | Use zlib |

Example with options:

```bash
cmake -B build \
  -DCMAKE_BUILD_TYPE=Debug \
  -Dprotobuf_BUILD_TESTS=ON \
  -Dprotobuf_BUILD_SHARED_LIBS=ON
```

### Bazel

```bash
# Build everything
bazel build //...

# Build specific target
bazel build //src/google/protobuf:protobuf
```

## Running Tests

### CMake

```bash
# Build and run all tests
cd build
ctest --output-on-failure

# Run specific test
./tests/protobuf-test

# Run with verbose output
ctest -V
```

### Bazel

```bash
# Run all tests
bazel test //...

# Run specific test
bazel test //src/google/protobuf:message_test
```

## Development Workflow

### 1. Create a Branch

```bash
git checkout -b my-feature
```

### 2. Make Changes

Edit code, add tests, update documentation.

### 3. Build and Test

```bash
cmake --build build --parallel
cd build && ctest
```

### 4. Commit

```bash
git add .
git commit -m "component: Description of change"
```

### 5. Submit PR

Push your branch and create a pull request on GitHub.

## IDE Setup

### Visual Studio Code

Recommended extensions:
- C/C++ (Microsoft)
- CMake Tools
- Clangd (alternative to C/C++)

Create `.vscode/settings.json`:

```json
{
  "cmake.buildDirectory": "${workspaceFolder}/build",
  "cmake.configureArgs": [
    "-Dprotobuf_BUILD_TESTS=ON"
  ]
}
```

### CLion

1. Open the project (CMakeLists.txt)
2. Configure CMake profile for Debug

### Visual Studio

```bash
cmake -B build -G "Visual Studio 17 2022"
```

Open `build/protobuf.sln`.

## Debugging

### GDB (Linux)

```bash
# Build with debug symbols
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Debug
gdb ./build/protoc
(gdb) run --cpp_out=. test.proto
```

### LLDB (macOS)

```bash
lldb ./build/protoc
(lldb) run --cpp_out=. test.proto
```

### Visual Studio

Set breakpoints and run with F5.

## Common Tasks

### Regenerate Generated Files

Some files are generated and checked in:

```bash
# Regenerate descriptor.pb.cc/h
./src/google/protobuf/generate_descriptor_proto.sh
```

### Format Code

```bash
# Format C++ files
clang-format -i src/google/protobuf/*.cc
```

### Run Specific Language Tests

```bash
# Python tests
cd python
python -m pytest

# Java tests
cd java
mvn test
```

## Troubleshooting

### CMake Can't Find Dependencies

```bash
# Specify paths explicitly
cmake -B build \
  -DCMAKE_PREFIX_PATH=/usr/local \
  -DAbseil_DIR=/path/to/abseil
```

### Build Failures

1. Clean and rebuild:
   ```bash
   rm -rf build
   cmake -B build
   cmake --build build
   ```

2. Update submodules:
   ```bash
   git submodule update --init --recursive
   ```

### Test Failures

1. Run specific test for more details:
   ```bash
   ./build/tests/protobuf-test --gtest_filter="*TestName*"
   ```

2. Check for environment issues:
   ```bash
   # Ensure LD_LIBRARY_PATH is set
   export LD_LIBRARY_PATH=$PWD/build/lib:$LD_LIBRARY_PATH
   ```

## Next Steps

- [Code Style](style.md) - Follow coding standards
- [Testing Guide](testing.md) - Write and run tests
- [Pull Request Process](pull-requests.md) - Submit your changes
