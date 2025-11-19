# Python Installation

This guide covers installing Protocol Buffers for Python.

## Installing the Runtime

### pip (Recommended)

```bash
pip install protobuf
```

### With Version Pinning

```bash
pip install protobuf==4.25.1
```

### In requirements.txt

```
protobuf>=4.21.0,<5.0.0
```

### conda

```bash
conda install -c conda-forge protobuf
```

## Installing the Compiler

You need `protoc` to generate Python code from `.proto` files.

### Using pip

```bash
pip install grpcio-tools
```

Then use:

```bash
python -m grpc_tools.protoc --python_out=. myfile.proto
```

### Pre-built Binary

See [Installation Guide](../../getting-started/installation.md).

## Generating Python Code

### Basic Generation

```bash
protoc --python_out=. message.proto
```

### With Type Hints (Python 3.7+)

```bash
protoc --python_out=. --pyi_out=. message.proto
```

This generates:
- `message_pb2.py` - Python module
- `message_pb2.pyi` - Type hints

### From Multiple Directories

```bash
protoc \
  --proto_path=src/protos \
  --proto_path=third_party/protos \
  --python_out=src/python \
  src/protos/myfile.proto
```

## Project Setup

### Directory Structure

```
myproject/
├── pyproject.toml
├── requirements.txt
├── protos/
│   └── myservice.proto
└── src/
    └── myproject/
        └── generated/
            └── myservice_pb2.py
```

### pyproject.toml

```toml
[project]
name = "myproject"
dependencies = [
    "protobuf>=4.21.0",
]

[project.optional-dependencies]
dev = [
    "grpcio-tools",
]
```

### Build Script

Create `generate_protos.py`:

```python
#!/usr/bin/env python3
import subprocess
from pathlib import Path

proto_dir = Path("protos")
output_dir = Path("src/myproject/generated")
output_dir.mkdir(parents=True, exist_ok=True)

for proto_file in proto_dir.glob("*.proto"):
    subprocess.run([
        "protoc",
        f"--proto_path={proto_dir}",
        f"--python_out={output_dir}",
        f"--pyi_out={output_dir}",
        str(proto_file)
    ], check=True)
    print(f"Generated {proto_file.stem}_pb2.py")
```

## Implementation Options

### Pure Python vs C++ Extension

By default, protobuf uses a C++ extension for performance. To use pure Python:

```bash
PROTOCOL_BUFFERS_PYTHON_IMPLEMENTATION=python python myapp.py
```

### Checking Implementation

```python
from google.protobuf.internal import api_implementation
print(api_implementation.Type())  # 'cpp' or 'python'
```

## Verify Installation

```python
import google.protobuf
print(google.protobuf.__version__)

# Test with a simple message
from google.protobuf.any_pb2 import Any
msg = Any()
msg.Pack(google.protobuf.empty_pb2.Empty())
print("Installation successful!")
```

## Troubleshooting

### Import Error

```python
ImportError: No module named 'google.protobuf'
```

Solution: Install protobuf

```bash
pip install protobuf
```

### Version Mismatch

```
TypeError: Descriptors cannot not be directly created
```

Solution: Ensure protoc version matches library version

```bash
protoc --version
python -c "import google.protobuf; print(google.protobuf.__version__)"
```

### Missing protoc

```bash
protoc: command not found
```

Solution: Install protoc or use grpcio-tools

```bash
pip install grpcio-tools
python -m grpc_tools.protoc --help
```

## Next Steps

- [Quick Start](quickstart.md) - Create your first program
- [API Reference](api-reference.md) - Detailed API documentation
