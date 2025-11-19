# RFC-0003: Performance Regression CI

**Status:** Draft
**Author:** Codebase Analysis
**Created:** November 19, 2025
**Effort:** 4 dev-days
**Category:** Quick Win

---

## Summary

Add continuous performance benchmarking to the CI pipeline that automatically detects performance regressions before they are merged, preventing gradual performance degradation.

## Motivation

### Problem Statement

Protocol Buffers is used in performance-critical applications, but:

1. **No automated regression detection** - Performance issues discovered after release
2. **Manual benchmarking** - Developers must remember to run benchmarks
3. **No trend tracking** - Can't see performance over time
4. **Silent degradation** - Small regressions compound

### Business Impact

A 5% regression per release compounds:
- After 5 releases: 23% slower
- After 10 releases: 40% slower

### Benefits

1. **Early detection** - Catch regressions before merge
2. **Trend visibility** - Track performance over time
3. **Developer awareness** - Automatic feedback on PRs
4. **Release confidence** - Performance guarantees

## Detailed Design

### Architecture

```
┌─────────────────────────────────────────────────────┐
│               Performance Dashboard                  │
│            (GitHub Pages + Bencher.dev)             │
└────────────────────────┬────────────────────────────┘
                         │
                    ┌────┴────┐
                    │ Results │
                    │  Store  │
                    └────┬────┘
                         │
         ┌───────────────┼───────────────┐
         │               │               │
         ▼               ▼               ▼
   ┌───────────┐   ┌───────────┐   ┌───────────┐
   │  C++ Bench│   │ Java Bench│   │ Py Bench  │
   │ (Google   │   │  (JMH)    │   │ (pytest-  │
   │ Benchmark)│   │           │   │ benchmark)│
   └───────────┘   └───────────┘   └───────────┘
```

### Implementation Steps

#### 1. Benchmark Suite Enhancement

Ensure consistent benchmarks exist for key operations:

```cpp
// benchmarks/cpp/benchmark_messages.cc

// Core operations to benchmark
static void BM_ParseSmallMessage(benchmark::State& state) {
  std::string data = CreateSmallMessage().SerializeAsString();
  for (auto _ : state) {
    SmallMessage msg;
    msg.ParseFromString(data);
    benchmark::DoNotOptimize(msg);
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_ParseSmallMessage);

static void BM_SerializeSmallMessage(benchmark::State& state) {
  SmallMessage msg = CreateSmallMessage();
  for (auto _ : state) {
    std::string output;
    msg.SerializeToString(&output);
    benchmark::DoNotOptimize(output);
  }
  state.SetBytesProcessed(state.iterations() * msg.ByteSizeLong());
}
BENCHMARK(BM_SerializeSmallMessage);

// More benchmarks: large messages, arena, reflection, etc.
```

#### 2. GitHub Actions Workflow

Create `.github/workflows/benchmarks.yml`:

```yaml
name: Performance Benchmarks

on:
  push:
    branches: [main]
  pull_request:
    branches: [main]

jobs:
  benchmark-cpp:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Build benchmarks
        run: bazel build //benchmarks/cpp:all -c opt

      - name: Run benchmarks
        run: |
          ./bazel-bin/benchmarks/cpp/benchmark_messages \
            --benchmark_format=json \
            --benchmark_out=benchmark_results.json

      - name: Store benchmark result
        uses: benchmark-action/github-action-benchmark@v1
        with:
          tool: 'googlecpp'
          output-file-path: benchmark_results.json
          github-token: ${{ secrets.GITHUB_TOKEN }}
          auto-push: true
          alert-threshold: '105%'
          comment-on-alert: true
          fail-on-alert: true
          alert-comment-cc-users: '@protobuf/performance'

  benchmark-java:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Run JMH benchmarks
        run: |
          bazel run //benchmarks/java:jmh -- \
            -rf json -rff benchmark_results.json

      - name: Store benchmark result
        uses: benchmark-action/github-action-benchmark@v1
        with:
          tool: 'jmh'
          output-file-path: benchmark_results.json
          github-token: ${{ secrets.GITHUB_TOKEN }}
          auto-push: true
          alert-threshold: '105%'
          comment-on-alert: true

  compare-baseline:
    needs: [benchmark-cpp, benchmark-java]
    runs-on: ubuntu-latest
    if: github.event_name == 'pull_request'
    steps:
      - name: Compare with baseline
        uses: benchmark-action/github-action-benchmark@v1
        with:
          tool: 'googlecpp'
          external-data-json-path: ./cache/benchmark-data.json
          github-token: ${{ secrets.GITHUB_TOKEN }}
          comment-always: true
```

#### 3. Benchmark Configurations

Create standardized benchmark configurations:

```yaml
# benchmarks/config.yaml
benchmarks:
  cpp:
    parse:
      - name: small_message
        iterations: 10000
        warmup: 100
      - name: large_message
        iterations: 1000
        warmup: 10
      - name: deeply_nested
        iterations: 5000
        warmup: 50
    serialize:
      - name: small_message
        iterations: 10000
        warmup: 100
      - name: large_message
        iterations: 1000
        warmup: 10

  java:
    parse:
      - name: SmallMessageBenchmark
        fork: 2
        warmup_iterations: 3
        measurement_iterations: 5

thresholds:
  regression_percent: 5  # Alert if >5% slower
  noise_percent: 2       # Ignore <2% variations
```

#### 4. Dashboard Configuration

GitHub Pages displays results:

```yaml
# .github/workflows/pages.yml
name: Deploy Benchmark Dashboard

on:
  push:
    branches: [main]

jobs:
  deploy:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
        with:
          ref: gh-pages

      - name: Setup Pages
        uses: actions/configure-pages@v3

      - name: Deploy
        uses: actions/deploy-pages@v2
```

#### 5. PR Comments

Automated comments on PRs:

```markdown
## Performance Report

### C++ Benchmarks

| Benchmark | Base | PR | Change |
|-----------|------|----|----|
| BM_ParseSmallMessage | 125 ns | 128 ns | ⚠️ +2.4% |
| BM_SerializeSmallMessage | 89 ns | 87 ns | ✅ -2.2% |
| BM_ParseLargeMessage | 4.2 µs | 4.1 µs | ✅ -2.4% |

### Summary
- 1 potential regression detected
- Overall: -0.7% (within noise threshold)

[View full results](https://protocolbuffers.github.io/protobuf/dev/bench/)
```

## Example Usage

### Viewing Results

1. **Dashboard:** https://protocolbuffers.github.io/protobuf/dev/bench/
2. **PR comments:** Automatic on each PR
3. **Alerts:** Slack/email when regression detected

### Running Locally

```bash
# Run C++ benchmarks
bazel run //benchmarks/cpp:benchmark_messages -c opt -- \
  --benchmark_filter=BM_Parse

# Run Java benchmarks
bazel run //benchmarks/java:jmh -- -f 1 -wi 3 -i 5

# Compare with baseline
./scripts/benchmark_compare.sh main HEAD
```

## Implementation Plan

### Day 1: Benchmark Suite
- [ ] Review existing benchmarks
- [ ] Add missing core benchmarks
- [ ] Standardize output format

### Day 2: CI Integration
- [ ] Create GitHub Actions workflow
- [ ] Configure benchmark-action
- [ ] Set up data storage

### Day 3: Dashboard
- [ ] Deploy GitHub Pages dashboard
- [ ] Configure charts and trends
- [ ] Set up alerts

### Day 4: Testing and Documentation
- [ ] Test on feature branch
- [ ] Document usage
- [ ] Configure notifications

## Backwards Compatibility

This is purely additive:
- No changes to protobuf code
- Only adds CI infrastructure
- Existing benchmarks unchanged

## Alternatives Considered

### Alternative 1: External Service (Bencher.dev)
- **Pro:** More features
- **Con:** Another service to manage
- **Decision:** Start with GitHub-native, migrate if needed

### Alternative 2: Nightly-only Benchmarks
- **Pro:** Less CI load
- **Con:** Regressions detected late
- **Decision:** Rejected - want immediate feedback

### Alternative 3: Manual Benchmark Reviews
- **Pro:** Human judgment
- **Con:** Inconsistent, time-consuming
- **Decision:** Rejected - automation essential

## Open Questions

1. **Threshold tuning** - What regression % triggers alert?
   - Suggestion: 5% initially, tune based on noise

2. **Benchmark selection** - Which benchmarks are CI-critical?
   - Suggestion: Core parse/serialize for each language

3. **Hardware consistency** - How to ensure consistent results?
   - Suggestion: Use GitHub-hosted runners, accept some variance

## Success Criteria

- [ ] Benchmarks run on every PR
- [ ] Dashboard shows trends over 30+ days
- [ ] <5% regression detected before merge
- [ ] False positive rate <10%
- [ ] Results within 2% variance between runs

## Effort Estimation

| Task | Hours |
|------|-------|
| Benchmark suite review | 4 |
| CI workflow creation | 8 |
| Dashboard setup | 6 |
| Testing | 6 |
| Documentation | 4 |
| **Total** | **28** (4 days) |

## Rollback Strategy

1. Disable benchmark workflow
2. Remove benchmark-action configuration
3. Optionally keep dashboard with historical data

No impact on protobuf functionality.

---

## References

- [Google Benchmark](https://github.com/google/benchmark)
- [github-action-benchmark](https://github.com/benchmark-action/github-action-benchmark)
- [JMH (Java Microbenchmark Harness)](https://openjdk.org/projects/code-tools/jmh/)
