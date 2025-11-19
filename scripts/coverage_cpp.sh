#!/bin/bash
# C++ Code Coverage Script
# Generates code coverage reports for C++ protobuf code using LLVM source-based coverage

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "=== C++ Coverage Report Generation ==="
echo "Project root: $PROJECT_ROOT"

# Create coverage output directory
mkdir -p "$PROJECT_ROOT/coverage/cpp"

# Run coverage with bazel
echo "Running bazel coverage for C++ targets..."
cd "$PROJECT_ROOT"

bazel coverage \
    --config=coverage \
    --combined_report=lcov \
    //src/google/protobuf:all

# Copy the coverage report to our output directory
COVERAGE_REPORT="bazel-out/_coverage/_coverage_report.dat"
if [ -f "$COVERAGE_REPORT" ]; then
    cp "$COVERAGE_REPORT" "coverage/cpp/lcov.info"
    echo "Coverage report copied to coverage/cpp/lcov.info"

    # Generate HTML report if genhtml is available
    if command -v genhtml &> /dev/null; then
        echo "Generating HTML coverage report..."
        genhtml "coverage/cpp/lcov.info" \
            --output-directory "coverage/cpp/html" \
            --title "Protocol Buffers C++ Coverage" \
            --legend \
            --show-details
        echo "HTML report generated at coverage/cpp/html/index.html"
    else
        echo "Note: genhtml not found. Install lcov for HTML reports."
        echo "  Ubuntu/Debian: apt-get install lcov"
        echo "  macOS: brew install lcov"
    fi
else
    echo "Error: Coverage report not found at $COVERAGE_REPORT"
    exit 1
fi

echo "=== C++ Coverage Complete ==="
