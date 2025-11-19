# Protocol Buffers Performance Benchmarks

This directory contains performance benchmarks for Protocol Buffers. These benchmarks help detect performance regressions and track performance over time.

## Overview

The benchmark suite includes:

- **Message operation benchmarks** - Parse, serialize, copy, merge, and clear operations
- **Arena allocation benchmarks** - Memory allocation performance with arenas
- **Size benchmarks** - Binary size measurements for different configurations

## Running Benchmarks Locally

### C++ Benchmarks

Run the main message benchmarks:

```bash
# Build benchmarks
bazel build //benchmarks:benchmark_messages -c opt

# Run all benchmarks
./bazel-bin/benchmarks/benchmark_messages

# Run specific benchmarks by filter
./bazel-bin/benchmarks/benchmark_messages --benchmark_filter=BM_Parse

# Run with JSON output
./bazel-bin/benchmarks/benchmark_messages \
  --benchmark_format=json \
  --benchmark_out=results.json
```

Run the extended benchmarks (includes arena, JSON, etc.):

```bash
bazel build //benchmarks:benchmark -c opt
./bazel-bin/benchmarks/benchmark
```

### Comparing Results Between Commits

Use the comparison script to measure performance changes:

```bash
# Compare current HEAD against main
./scripts/benchmark_compare.sh main HEAD

# Compare between specific commits or tags
./scripts/benchmark_compare.sh v3.21.0 v3.22.0

# Set custom threshold (default is 5%)
BENCHMARK_THRESHOLD=10 ./scripts/benchmark_compare.sh main HEAD
```

## CI Integration

Performance benchmarks run automatically on:

- Every push to `main` branch
- Every pull request to `main` branch

### Regression Detection

The CI system will:

1. Run benchmarks on the PR branch
2. Compare results against baseline data
3. Comment on the PR with performance report
4. **Fail the check** if any benchmark regresses >5%

### Dashboard

View performance trends at:
https://protocolbuffers.github.io/protobuf/dev/bench/

## Benchmark Suite

### Message Benchmarks (`benchmark_messages`)

| Benchmark | Description |
|-----------|-------------|
| `BM_ParseSmallMessage` | Parse small message (~50 bytes) |
| `BM_ParseMediumMessage` | Parse medium message (~500 bytes) |
| `BM_ParseLargeMessage` | Parse large message (~2KB) |
| `BM_ParseDeeplyNestedMessage` | Parse 5-level nested message |
| `BM_ParseRepeatedFields` | Parse message with repeated fields |
| `BM_SerializeSmallMessage` | Serialize small message |
| `BM_SerializeMediumMessage` | Serialize medium message |
| `BM_SerializeLargeMessage` | Serialize large message |
| `BM_SerializeDeeplyNestedMessage` | Serialize nested message |
| `BM_SerializeRepeatedFields` | Serialize repeated fields |
| `BM_ParseSmallMessageArena` | Parse with arena allocation |
| `BM_ParseLargeMessageArena` | Parse large with arena |
| `BM_ByteSizeSmallMessage` | Calculate byte size (small) |
| `BM_ByteSizeLargeMessage` | Calculate byte size (large) |
| `BM_CopySmallMessage` | Copy small message |
| `BM_CopyLargeMessage` | Copy large message |
| `BM_MergeSmallMessage` | Merge small message |
| `BM_MergeLargeMessage` | Merge large message |
| `BM_ClearSmallMessage` | Clear small message |
| `BM_ClearLargeMessage` | Clear large message |

### Extended Benchmarks (`benchmark`)

| Benchmark | Description |
|-----------|-------------|
| `BM_ArenaOneAlloc` | Single arena allocation |
| `BM_ArenaInitialBlockOneAlloc` | Arena with initial block |
| `BM_ArenaFuseUnbalanced` | Arena fusion (unbalanced) |
| `BM_ArenaFuseBalanced` | Arena fusion (balanced) |
| `BM_LoadAdsDescriptor_*` | Load large descriptor |
| `BM_Parse_Upb_FileDesc` | UPB parse benchmark |
| `BM_Parse_Proto2` | Proto2 parse benchmark |
| `BM_SerializeDescriptor_*` | Descriptor serialization |
| `BM_JsonParse_*` | JSON parsing |
| `BM_JsonSerialize_*` | JSON serialization |

## Configuration

Benchmark configuration is in `benchmarks/config.yaml`:

```yaml
thresholds:
  regression_percent: 5   # Alert threshold
  noise_percent: 2        # Ignore threshold

ci:
  repetitions: 3          # Statistical repetitions
  min_time_seconds: 1     # Minimum benchmark time
```

## Contributing

When adding new benchmarks:

1. Add benchmark to appropriate `.cc` file
2. Update `BUILD` file if needed
3. Add to `config.yaml` with threshold
4. Update this README

### Best Practices

- Use `benchmark::DoNotOptimize()` to prevent compiler optimizations
- Call `state.SetBytesProcessed()` for throughput metrics
- Use realistic data sizes and patterns
- Include warmup iterations for cache effects
- Test with multiple message sizes

## Troubleshooting

### High Variance

If benchmarks show high variance:

1. Increase repetitions: `--benchmark_repetitions=5`
2. Increase min time: `--benchmark_min_time=2s`
3. Run on dedicated hardware (not shared CI runners)
4. Disable CPU frequency scaling

### False Positives

If CI reports false regressions:

1. Check the noise threshold in `config.yaml`
2. Review the comparison manually with `benchmark_compare.sh`
3. Re-run benchmarks with more iterations

## References

- [Google Benchmark](https://github.com/google/benchmark)
- [github-action-benchmark](https://github.com/benchmark-action/github-action-benchmark)
- [Protocol Buffers Performance Tips](https://developers.google.com/protocol-buffers/docs/cpptutorial)
