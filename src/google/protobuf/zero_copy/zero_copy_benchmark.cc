// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Performance benchmarks for zero-copy parsing functionality.
// These benchmarks compare zero-copy access vs traditional string copying.

#include <benchmark/benchmark.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/string_view.h"
#include "google/protobuf/zero_copy/zero_copy.h"

namespace google {
namespace protobuf {
namespace zero_copy {
namespace {

// =============================================================================
// Buffer Creation Benchmarks
// =============================================================================

static void BM_ParseBufferCreateOwned_Small(benchmark::State& state) {
  std::string data(64, 'X');
  for (auto _ : state) {
    auto buffer = ParseBuffer::Create(data);
    benchmark::DoNotOptimize(buffer);
  }
  state.SetBytesProcessed(state.iterations() * 64);
}
BENCHMARK(BM_ParseBufferCreateOwned_Small);

static void BM_ParseBufferCreateOwned_Large(benchmark::State& state) {
  size_t size = state.range(0);
  std::string data(size, 'X');
  for (auto _ : state) {
    auto buffer = ParseBuffer::Create(data);
    benchmark::DoNotOptimize(buffer);
  }
  state.SetBytesProcessed(state.iterations() * size);
}
BENCHMARK(BM_ParseBufferCreateOwned_Large)->Range(1024, 1024 * 1024);

static void BM_ParseBufferWrap(benchmark::State& state) {
  size_t size = state.range(0);
  std::string data(size, 'X');
  for (auto _ : state) {
    auto buffer = ParseBuffer::Wrap(data.data(), data.size());
    benchmark::DoNotOptimize(buffer);
  }
  state.SetBytesProcessed(state.iterations() * size);
}
BENCHMARK(BM_ParseBufferWrap)->Range(64, 1024 * 1024);

// =============================================================================
// Zero-Copy String Access vs Traditional Copy
// =============================================================================

// Benchmark zero-copy string view access
static void BM_ZeroCopyStringView(benchmark::State& state) {
  size_t size = state.range(0);
  std::string data(size, 'X');
  auto buffer = ParseBuffer::Create(std::move(data));
  ZeroCopyString str(buffer, 0, size);

  for (auto _ : state) {
    absl::string_view view = str.view();
    benchmark::DoNotOptimize(view.data());
    benchmark::DoNotOptimize(view.size());
  }
  state.SetBytesProcessed(state.iterations() * size);
}
BENCHMARK(BM_ZeroCopyStringView)->Range(64, 1024 * 1024);

// Benchmark traditional string copy
static void BM_TraditionalStringCopy(benchmark::State& state) {
  size_t size = state.range(0);
  std::string data(size, 'X');

  for (auto _ : state) {
    std::string copy = data;
    benchmark::DoNotOptimize(copy.data());
    benchmark::DoNotOptimize(copy.size());
  }
  state.SetBytesProcessed(state.iterations() * size);
}
BENCHMARK(BM_TraditionalStringCopy)->Range(64, 1024 * 1024);

// Benchmark zero-copy string materialization
static void BM_ZeroCopyStringMaterialize(benchmark::State& state) {
  size_t size = state.range(0);
  std::string data(size, 'X');
  auto buffer = ParseBuffer::Create(std::move(data));

  for (auto _ : state) {
    ZeroCopyString str(buffer, 0, size);
    std::string materialized = str.ToString();
    benchmark::DoNotOptimize(materialized.data());
  }
  state.SetBytesProcessed(state.iterations() * size);
}
BENCHMARK(BM_ZeroCopyStringMaterialize)->Range(64, 1024 * 1024);

// =============================================================================
// Buffer Substr vs String Substr
// =============================================================================

static void BM_BufferSubstr(benchmark::State& state) {
  size_t size = 1024 * 1024;  // 1MB buffer
  std::string data(size, 'X');
  auto buffer = ParseBuffer::Create(std::move(data));

  for (auto _ : state) {
    // Extract 1KB chunks from various positions
    for (size_t i = 0; i < 1000; i++) {
      absl::string_view view = buffer->Substr(i * 1000, 1000);
      benchmark::DoNotOptimize(view.data());
    }
  }
  state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(BM_BufferSubstr);

static void BM_StringSubstr(benchmark::State& state) {
  size_t size = 1024 * 1024;  // 1MB string
  std::string data(size, 'X');

  for (auto _ : state) {
    // Extract 1KB chunks from various positions (copies each time)
    for (size_t i = 0; i < 1000; i++) {
      std::string chunk = data.substr(i * 1000, 1000);
      benchmark::DoNotOptimize(chunk.data());
    }
  }
  state.SetItemsProcessed(state.iterations() * 1000);
}
BENCHMARK(BM_StringSubstr);

// =============================================================================
// Multiple String Field Access
// =============================================================================

// Simulate accessing multiple string fields with zero-copy
static void BM_MultipleFieldsZeroCopy(benchmark::State& state) {
  int num_fields = state.range(0);
  size_t field_size = 1024;  // 1KB per field
  size_t total_size = num_fields * field_size;

  std::string data(total_size, 'X');
  auto buffer = ParseBuffer::Create(std::move(data));

  // Pre-create ZeroCopyString for each field
  std::vector<ZeroCopyString> fields;
  for (int i = 0; i < num_fields; i++) {
    fields.emplace_back(buffer, i * field_size, field_size);
  }

  for (auto _ : state) {
    for (const auto& field : fields) {
      absl::string_view view = field.view();
      benchmark::DoNotOptimize(view.data());
    }
  }
  state.SetItemsProcessed(state.iterations() * num_fields);
}
BENCHMARK(BM_MultipleFieldsZeroCopy)->Range(1, 100);

// Simulate accessing multiple string fields with copying
static void BM_MultipleFieldsCopy(benchmark::State& state) {
  int num_fields = state.range(0);
  size_t field_size = 1024;  // 1KB per field

  // Pre-create separate strings for each field
  std::vector<std::string> fields;
  for (int i = 0; i < num_fields; i++) {
    fields.emplace_back(field_size, 'X');
  }

  for (auto _ : state) {
    for (const auto& field : fields) {
      std::string copy = field;  // Copy each field
      benchmark::DoNotOptimize(copy.data());
    }
  }
  state.SetItemsProcessed(state.iterations() * num_fields);
}
BENCHMARK(BM_MultipleFieldsCopy)->Range(1, 100);

// =============================================================================
// ZeroCopyParseResult Field Lookup
// =============================================================================

static void BM_ParseResultFieldLookup(benchmark::State& state) {
  int num_fields = state.range(0);
  std::string data(num_fields * 100, 'X');
  auto buffer = ParseBuffer::Create(std::move(data));

  ZeroCopyParseResult result(buffer);
  for (int i = 0; i < num_fields; i++) {
    result.AddFieldLocation(i + 1, i * 100, 100);
  }

  for (auto _ : state) {
    for (int i = 0; i < num_fields; i++) {
      absl::string_view view = result.GetStringView(i + 1);
      benchmark::DoNotOptimize(view.data());
    }
  }
  state.SetItemsProcessed(state.iterations() * num_fields);
}
BENCHMARK(BM_ParseResultFieldLookup)->Range(1, 100);

// =============================================================================
// Lazy String Benchmarks
// =============================================================================

static void BM_LazyStringView(benchmark::State& state) {
  size_t size = state.range(0);
  std::string data(size, 'X');
  auto buffer = ParseBuffer::Create(std::move(data));
  LazyString str(buffer, 0, size);

  for (auto _ : state) {
    absl::string_view view = str.view();
    benchmark::DoNotOptimize(view.data());
  }
  state.SetBytesProcessed(state.iterations() * size);
}
BENCHMARK(BM_LazyStringView)->Range(64, 1024 * 1024);

static void BM_LazyStringMaterialize(benchmark::State& state) {
  size_t size = state.range(0);
  std::string data(size, 'X');
  auto buffer = ParseBuffer::Create(std::move(data));

  for (auto _ : state) {
    state.PauseTiming();
    LazyString str(buffer, 0, size);
    state.ResumeTiming();

    const std::string& s = str.str();
    benchmark::DoNotOptimize(s.data());
  }
  state.SetBytesProcessed(state.iterations() * size);
}
BENCHMARK(BM_LazyStringMaterialize)->Range(64, 1024 * 1024);

// =============================================================================
// Memory Efficiency Benchmark
// =============================================================================

// Measure memory efficiency of shared buffer
static void BM_SharedBufferMemory(benchmark::State& state) {
  int num_refs = state.range(0);
  size_t buffer_size = 1024 * 1024;  // 1MB

  std::string data(buffer_size, 'X');

  for (auto _ : state) {
    auto buffer = ParseBuffer::Create(std::move(data));

    // Create multiple ZeroCopyStrings sharing the same buffer
    std::vector<ZeroCopyString> strings;
    strings.reserve(num_refs);
    for (int i = 0; i < num_refs; i++) {
      size_t offset = (i * 1000) % buffer_size;
      size_t len = 1000;
      if (offset + len > buffer_size) len = buffer_size - offset;
      strings.emplace_back(buffer, offset, len);
    }

    // Access all strings
    for (const auto& s : strings) {
      benchmark::DoNotOptimize(s.view().data());
    }

    // Prepare for next iteration
    data = std::string(buffer_size, 'X');
  }
  state.SetItemsProcessed(state.iterations() * num_refs);
}
BENCHMARK(BM_SharedBufferMemory)->Range(1, 1000);

// =============================================================================
// Bytes Field Benchmarks
// =============================================================================

static void BM_ZeroCopyBytesSpan(benchmark::State& state) {
  size_t size = state.range(0);
  std::string data(size, '\x00');
  auto buffer = ParseBuffer::Create(std::move(data));
  ZeroCopyBytes bytes(buffer, 0, size);

  for (auto _ : state) {
    auto span = bytes.span();
    benchmark::DoNotOptimize(span.data());
    benchmark::DoNotOptimize(span.size());
  }
  state.SetBytesProcessed(state.iterations() * size);
}
BENCHMARK(BM_ZeroCopyBytesSpan)->Range(64, 1024 * 1024);

}  // namespace
}  // namespace zero_copy
}  // namespace protobuf
}  // namespace google
