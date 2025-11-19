#!/bin/bash
# Protocol Buffers - Google's data interchange format
# Copyright 2023 Google LLC.  All rights reserved.
#
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file or at
# https://developers.google.com/open-source/licenses/bsd

# Script to compare benchmark results between two commits/branches
#
# Usage:
#   ./scripts/benchmark_compare.sh <baseline_ref> <compare_ref>
#
# Example:
#   ./scripts/benchmark_compare.sh main HEAD
#   ./scripts/benchmark_compare.sh v3.21.0 v3.22.0

set -e

BASELINE_REF="${1:-main}"
COMPARE_REF="${2:-HEAD}"
BENCHMARK_TARGET="//benchmarks:benchmark_messages"
OUTPUT_DIR="${BENCHMARK_OUTPUT_DIR:-/tmp/benchmark_compare}"
THRESHOLD_PERCENT="${BENCHMARK_THRESHOLD:-5}"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "================================================"
echo "Protocol Buffers Benchmark Comparison"
echo "================================================"
echo ""
echo "Baseline: ${BASELINE_REF}"
echo "Compare:  ${COMPARE_REF}"
echo "Threshold: ${THRESHOLD_PERCENT}%"
echo ""

# Create output directory
mkdir -p "${OUTPUT_DIR}"

# Function to run benchmarks for a given ref
run_benchmarks() {
    local ref=$1
    local output_file=$2
    local ref_name=$(echo "$ref" | tr '/' '_')

    echo "Running benchmarks for ${ref}..."

    # Stash any uncommitted changes
    git stash --quiet 2>/dev/null || true

    # Checkout the ref
    git checkout --quiet "${ref}"

    # Build benchmarks
    echo "  Building benchmarks..."
    bazel build "${BENCHMARK_TARGET}" -c opt --ui_event_filters=-info --noshow_progress 2>&1 | head -20

    # Run benchmarks
    echo "  Running benchmarks..."
    ./bazel-bin/benchmarks/benchmark_messages \
        --benchmark_format=json \
        --benchmark_out="${output_file}" \
        --benchmark_repetitions=3 \
        --benchmark_report_aggregates_only=true \
        2>&1 | grep -E "^(BM_|Running)" || true

    echo "  Done."
    echo ""
}

# Function to parse JSON benchmark results
parse_results() {
    local json_file=$1
    python3 - "${json_file}" << 'PYTHON_SCRIPT'
import json
import sys

with open(sys.argv[1], 'r') as f:
    data = json.load(f)

results = {}
for benchmark in data.get('benchmarks', []):
    name = benchmark.get('name', '')
    # Only get mean results
    if '_mean' in name:
        base_name = name.replace('_mean', '')
        real_time = benchmark.get('real_time', 0)
        results[base_name] = real_time

for name, time in sorted(results.items()):
    print(f"{name}:{time}")
PYTHON_SCRIPT
}

# Function to compare results
compare_results() {
    local baseline_file=$1
    local compare_file=$2

    echo "================================================"
    echo "Comparison Results"
    echo "================================================"
    echo ""

    # Parse both result files
    declare -A baseline_results
    declare -A compare_results

    while IFS=: read -r name time; do
        baseline_results["$name"]=$time
    done < <(parse_results "${baseline_file}")

    while IFS=: read -r name time; do
        compare_results["$name"]=$time
    done < <(parse_results "${compare_file}")

    # Print header
    printf "%-50s %15s %15s %10s %s\n" "Benchmark" "Baseline" "Compare" "Change" "Status"
    printf "%-50s %15s %15s %10s %s\n" "---------" "--------" "-------" "------" "------"

    has_regression=false

    # Compare results
    for name in $(echo "${!baseline_results[@]}" | tr ' ' '\n' | sort); do
        baseline=${baseline_results[$name]}
        compare=${compare_results[$name]:-0}

        if [ "$baseline" != "0" ] && [ "$compare" != "0" ]; then
            # Calculate percentage change
            change=$(python3 -c "
baseline = $baseline
compare = $compare
if baseline > 0:
    change = ((compare - baseline) / baseline) * 100
    print(f'{change:.1f}')
else:
    print('0.0')
")
            change_float=$(echo "$change" | tr -d '%')

            # Determine status
            if (( $(echo "$change_float > $THRESHOLD_PERCENT" | bc -l) )); then
                status="${RED}REGRESSION${NC}"
                has_regression=true
            elif (( $(echo "$change_float < -$THRESHOLD_PERCENT" | bc -l) )); then
                status="${GREEN}IMPROVEMENT${NC}"
            else
                status="${YELLOW}OK${NC}"
            fi

            # Format times
            baseline_fmt=$(python3 -c "print(f'{$baseline:.2f} ns')")
            compare_fmt=$(python3 -c "print(f'{$compare:.2f} ns')")

            printf "%-50s %15s %15s %9s%% %b\n" "$name" "$baseline_fmt" "$compare_fmt" "$change" "$status"
        fi
    done

    echo ""

    if [ "$has_regression" = true ]; then
        echo -e "${RED}WARNING: Performance regressions detected!${NC}"
        echo "Regressions greater than ${THRESHOLD_PERCENT}% may indicate a problem."
        return 1
    else
        echo -e "${GREEN}All benchmarks within acceptable threshold.${NC}"
        return 0
    fi
}

# Store current branch/commit
ORIGINAL_REF=$(git rev-parse --abbrev-ref HEAD 2>/dev/null || git rev-parse HEAD)

# Run benchmarks for baseline
BASELINE_OUTPUT="${OUTPUT_DIR}/baseline.json"
run_benchmarks "${BASELINE_REF}" "${BASELINE_OUTPUT}"

# Run benchmarks for comparison
COMPARE_OUTPUT="${OUTPUT_DIR}/compare.json"
run_benchmarks "${COMPARE_REF}" "${COMPARE_OUTPUT}"

# Return to original ref
git checkout --quiet "${ORIGINAL_REF}"
git stash pop --quiet 2>/dev/null || true

# Compare results
compare_results "${BASELINE_OUTPUT}" "${COMPARE_OUTPUT}"

echo ""
echo "Results saved to: ${OUTPUT_DIR}/"
echo "  - baseline.json"
echo "  - compare.json"
