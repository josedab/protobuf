#!/bin/bash
# Python Code Coverage Script
# Generates code coverage reports for Python protobuf code using coverage.py

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "=== Python Coverage Report Generation ==="
echo "Project root: $PROJECT_ROOT"

# Create coverage output directory
mkdir -p "$PROJECT_ROOT/coverage/python"

cd "$PROJECT_ROOT/python"

# Check if coverage is installed
if ! python3 -c "import coverage" 2>/dev/null; then
    echo "Installing coverage.py..."
    pip3 install coverage pytest
fi

# Run tests with coverage
echo "Running Python tests with coverage..."
python3 -m coverage run \
    --source=google/protobuf \
    --omit="*_test.py,*_pb2.py" \
    -m pytest google/protobuf/ -v --tb=short || true

# Generate reports
echo "Generating coverage reports..."

# XML report for Codecov
python3 -m coverage xml -o "$PROJECT_ROOT/coverage/python/coverage.xml"
echo "XML report generated at coverage/python/coverage.xml"

# HTML report for local viewing
python3 -m coverage html -d "$PROJECT_ROOT/coverage/python/html"
echo "HTML report generated at coverage/python/html/index.html"

# Text report for quick viewing
echo ""
echo "Coverage Summary:"
python3 -m coverage report --show-missing

echo ""
echo "=== Python Coverage Complete ==="
