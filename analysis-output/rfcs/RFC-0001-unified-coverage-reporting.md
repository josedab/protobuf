# RFC-0001: Unified Code Coverage Reporting

**Status:** Draft
**Author:** Codebase Analysis
**Created:** November 19, 2025
**Effort:** 3 dev-days
**Category:** Quick Win

---

## Summary

Implement a centralized code coverage reporting system that aggregates coverage data from all supported languages (C++, Java, Python, etc.) into a unified dashboard, providing visibility into test coverage gaps across the entire codebase.

## Motivation

### Problem Statement

The Protocol Buffers codebase currently lacks unified test coverage measurement:

1. **No centralized metrics** - Each language has separate testing but no aggregated view
2. **Coverage variation** - Python has 54% test ratio, C++ has 9.5%
3. **Invisible gaps** - Hard to identify under-tested code paths
4. **No trend tracking** - Can't detect coverage regression over time

### Evidence from Analysis

| Language | Test Files | Test LOC | Test Ratio | Coverage Tool |
|----------|------------|----------|------------|---------------|
| Python | 35 | 19,600 | 54% | None configured |
| Java | 91 | 44,300 | 37% | None configured |
| C++ | 215 | 33,000 | 9.5% | None configured |

### Benefits

1. **Identify gaps** - Discover untested code paths
2. **Track trends** - Detect coverage regression
3. **Guide effort** - Prioritize testing where needed
4. **Quality signal** - Provide confidence metric for releases

## Detailed Design

### Architecture

```
┌─────────────────────────────────────────────────────┐
│                Coverage Dashboard                    │
│        (GitHub Pages / Codecov / Similar)           │
└────────────────────────┬────────────────────────────┘
                         │
           ┌─────────────┼─────────────┐
           │             │             │
           ▼             ▼             ▼
    ┌───────────┐ ┌───────────┐ ┌───────────┐
    │   LCOV    │ │  JaCoCo   │ │Coverage.py│
    │   (C++)   │ │  (Java)   │ │  (Python) │
    └─────┬─────┘ └─────┬─────┘ └─────┬─────┘
          │             │             │
          ▼             ▼             ▼
    ┌───────────┐ ┌───────────┐ ┌───────────┐
    │  Bazel    │ │  Bazel    │ │  Bazel    │
    │ C++ Tests │ │Java Tests │ │  Pytest   │
    └───────────┘ └───────────┘ └───────────┘
```

### Implementation Steps

#### 1. C++ Coverage (LLVM Source-Based)

Add to `.bazelrc`:
```
build:coverage --instrumentation_filter="//src/google/protobuf[/:]"
build:coverage --collect_code_coverage
build:coverage --combined_report=lcov
```

Create coverage script:
```bash
#!/bin/bash
# scripts/coverage_cpp.sh

bazel coverage --config=coverage //src/google/protobuf:all
genhtml bazel-out/_coverage/_coverage_report.dat \
  --output-directory coverage/cpp
```

#### 2. Java Coverage (JaCoCo)

Add to `java/BUILD.bazel`:
```python
java_test(
    name = "core_test",
    # ... existing config ...
    tags = ["coverage"],
)
```

Configure JaCoCo:
```python
# MODULE.bazel
bazel_dep(name = "rules_jvm_external", version = "5.3")

java_library(
    name = "jacoco_agent",
    neverlink = True,
    exports = ["@maven//:org_jacoco_org_jacoco_agent"],
)
```

#### 3. Python Coverage (coverage.py)

Add to `python/BUILD.bazel`:
```python
py_test(
    name = "all_tests",
    srcs = glob(["**/test_*.py"]),
    deps = [
        "//python/google/protobuf",
        requirement("coverage"),
    ],
)
```

Create coverage script:
```bash
#!/bin/bash
# scripts/coverage_python.sh

cd python
coverage run -m pytest google/protobuf/
coverage xml -o ../coverage/python/coverage.xml
coverage html -d ../coverage/python/html
```

#### 4. GitHub Actions Workflow

Create `.github/workflows/coverage.yml`:
```yaml
name: Coverage Report

on:
  push:
    branches: [main]
  pull_request:
    branches: [main]

jobs:
  coverage:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: C++ Coverage
        run: ./scripts/coverage_cpp.sh

      - name: Java Coverage
        run: bazel coverage //java:all --combined_report=lcov

      - name: Python Coverage
        run: ./scripts/coverage_python.sh

      - name: Upload to Codecov
        uses: codecov/codecov-action@v3
        with:
          files: |
            coverage/cpp/lcov.info,
            coverage/java/jacoco.xml,
            coverage/python/coverage.xml
          flags: cpp,java,python
          fail_ci_if_error: true

      - name: Comment on PR
        if: github.event_name == 'pull_request'
        uses: codecov/codecov-action@v3
        with:
          comment: true
```

#### 5. Codecov Configuration

Create `codecov.yml`:
```yaml
codecov:
  require_ci_to_pass: yes

coverage:
  precision: 2
  round: down
  range: "60...100"

  status:
    project:
      default:
        target: auto
        threshold: 1%
    patch:
      default:
        target: 80%

flags:
  cpp:
    paths:
      - src/google/protobuf/
    carryforward: true
  java:
    paths:
      - java/
    carryforward: true
  python:
    paths:
      - python/google/protobuf/
    carryforward: true

comment:
  layout: "reach,diff,flags,files"
  behavior: default
  require_changes: true
```

### Dashboard Features

1. **Per-language breakdown** - Coverage % for each language
2. **Trend graphs** - Coverage over time
3. **PR comments** - Coverage diff on each PR
4. **File-level detail** - Drill down to uncovered lines
5. **Threshold enforcement** - Fail CI if coverage drops

## Example Usage

### Viewing Coverage

After implementation, coverage will be visible:

1. **PR Comments:**
   ```
   Coverage Report
   ───────────────
   Project coverage: 72.4% (+0.3%)

   | Flag   | Coverage | Δ     |
   |--------|----------|-------|
   | cpp    | 65.2%    | +0.5% |
   | java   | 78.9%    | +0.2% |
   | python | 82.1%    | +0.1% |
   ```

2. **Dashboard URL:** `https://codecov.io/gh/protocolbuffers/protobuf`

### Local Coverage Generation

```bash
# Generate C++ coverage locally
./scripts/coverage_cpp.sh
open coverage/cpp/index.html

# Generate Python coverage locally
./scripts/coverage_python.sh
open coverage/python/html/index.html
```

## Implementation Plan

### Phase 1: Infrastructure (Day 1)
- [ ] Create coverage scripts for each language
- [ ] Configure Codecov account
- [ ] Add `codecov.yml` configuration

### Phase 2: CI Integration (Day 2)
- [ ] Create GitHub Actions workflow
- [ ] Test on a feature branch
- [ ] Configure PR comments

### Phase 3: Rollout (Day 3)
- [ ] Merge to main branch
- [ ] Document in CONTRIBUTING.md
- [ ] Announce to maintainers

## Backwards Compatibility

This is purely additive - no impact on existing functionality:
- No code changes to protobuf itself
- Only adds CI infrastructure
- Existing tests unchanged

## Alternatives Considered

### Alternative 1: Language-Specific Dashboards
- **Pro:** Simpler implementation
- **Con:** No unified view, harder to compare
- **Decision:** Rejected - unified view is the main value

### Alternative 2: Self-Hosted Coverage Tool
- **Pro:** Full control
- **Con:** Maintenance burden, cost
- **Decision:** Rejected - Codecov is industry standard

### Alternative 3: Manual Coverage Checks
- **Pro:** No infrastructure needed
- **Con:** Won't be done regularly
- **Decision:** Rejected - automation is essential

## Open Questions

1. **Coverage thresholds** - What minimum coverage should we enforce?
   - Suggestion: Start with current levels, increase over time

2. **Carryforward** - Should we carry forward coverage for unchanged files?
   - Suggestion: Yes, to avoid noise from language-specific changes

3. **Cost** - Codecov pricing for open source?
   - Note: Free for public repositories

## Success Criteria

- [ ] Coverage reports generated for C++, Java, Python
- [ ] PR comments show coverage diff
- [ ] Dashboard accessible to all maintainers
- [ ] Coverage trend visible over 30+ days
- [ ] No false negatives (no incorrectly flagged regressions)

## Rollback Strategy

If issues arise:
1. Disable coverage workflow in GitHub Actions
2. Remove Codecov integration
3. Delete configuration files

No impact on actual protobuf code.

---

## References

- [Codecov Documentation](https://docs.codecov.com/)
- [LLVM Source-Based Coverage](https://clang.llvm.org/docs/SourceBasedCodeCoverage.html)
- [JaCoCo Documentation](https://www.jacoco.org/jacoco/)
- [Coverage.py Documentation](https://coverage.readthedocs.io/)
