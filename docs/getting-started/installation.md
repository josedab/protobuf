# Installation

This guide covers installing the Protocol Buffer compiler (`protoc`) and runtime libraries for various languages.

## Installing the Compiler (protoc)

The `protoc` compiler transforms `.proto` files into language-specific code.

### Pre-built Binaries (Recommended)

Download pre-built binaries from the [GitHub releases page](https://github.com/protocolbuffers/protobuf/releases).

=== "Linux"

    ```bash
    # Download the latest release (adjust version as needed)
    PROTOC_VERSION=25.1
    PROTOC_ZIP=protoc-${PROTOC_VERSION}-linux-x86_64.zip

    curl -OL https://github.com/protocolbuffers/protobuf/releases/download/v${PROTOC_VERSION}/${PROTOC_ZIP}

    # Extract to /usr/local (requires sudo)
    sudo unzip -o $PROTOC_ZIP -d /usr/local bin/protoc
    sudo unzip -o $PROTOC_ZIP -d /usr/local 'include/*'

    # Verify installation
    protoc --version
    ```

=== "macOS"

    Using Homebrew:

    ```bash
    brew install protobuf

    # Verify installation
    protoc --version
    ```

    Or download the binary:

    ```bash
    PROTOC_VERSION=25.1
    PROTOC_ZIP=protoc-${PROTOC_VERSION}-osx-x86_64.zip

    curl -OL https://github.com/protocolbuffers/protobuf/releases/download/v${PROTOC_VERSION}/${PROTOC_ZIP}

    unzip -o $PROTOC_ZIP -d $HOME/.local
    export PATH="$HOME/.local/bin:$PATH"
    ```

=== "Windows"

    1. Download `protoc-{VERSION}-win64.zip` from the [releases page](https://github.com/protocolbuffers/protobuf/releases)
    2. Extract to a directory (e.g., `C:\protoc`)
    3. Add `C:\protoc\bin` to your PATH environment variable
    4. Verify: `protoc --version`

### Package Managers

=== "Ubuntu/Debian"

    ```bash
    sudo apt update
    sudo apt install -y protobuf-compiler
    ```

    !!! note
        Package manager versions may be outdated. For the latest version, use pre-built binaries.

=== "Fedora"

    ```bash
    sudo dnf install protobuf-compiler
    ```

=== "Arch Linux"

    ```bash
    sudo pacman -S protobuf
    ```

### Building from Source

For the latest features or custom builds:

```bash
# Clone the repository
git clone https://github.com/protocolbuffers/protobuf.git
cd protobuf

# Build with CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel

# Install (requires sudo)
sudo cmake --install build
```

See [Building from Source](../contributing/setup.md) for detailed build instructions.

## Installing Runtime Libraries

Each language requires its own runtime library.

### C++

The C++ runtime is included when you build protobuf from source or install via package manager.

For CMake projects:

```cmake
find_package(Protobuf REQUIRED)
target_link_libraries(your_target PRIVATE protobuf::libprotobuf)
```

### Java

Add to your `pom.xml`:

```xml
<dependency>
  <groupId>com.google.protobuf</groupId>
  <artifactId>protobuf-java</artifactId>
  <version>3.25.1</version>
</dependency>
```

Or for Gradle:

```groovy
implementation 'com.google.protobuf:protobuf-java:3.25.1'
```

### Python

```bash
pip install protobuf
```

### Go

```bash
go install google.golang.org/protobuf/cmd/protoc-gen-go@latest
```

### Other Languages

See the [Language Guides](../languages/index.md) for installation instructions for other languages.

## Verifying Installation

Test your installation by creating a simple `.proto` file:

```protobuf
// test.proto
syntax = "proto3";

message Test {
  string name = 1;
}
```

Generate code:

```bash
protoc --cpp_out=. test.proto    # For C++
protoc --java_out=. test.proto   # For Java
protoc --python_out=. test.proto # For Python
```

If the command completes without errors, your installation is working correctly.

## Troubleshooting

### `protoc: command not found`

Ensure the `protoc` binary is in your PATH:

```bash
# Check where protoc is installed
which protoc

# Add to PATH if needed
export PATH="$PATH:/usr/local/bin"
```

### Version Mismatch Errors

Ensure your runtime library version matches your `protoc` version:

```bash
protoc --version  # Check compiler version
```

Compare with your runtime library version and update if needed.

### Permission Denied

On Linux/macOS, you may need to make the binary executable:

```bash
chmod +x /usr/local/bin/protoc
```

## Next Steps

Now that you have Protocol Buffers installed, proceed to the [Quick Start](quickstart.md) tutorial.
