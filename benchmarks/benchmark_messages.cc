// Protocol Buffers - Google's data interchange format
// Copyright 2023 Google LLC.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Performance benchmarks for core protobuf message operations.
// These benchmarks measure parse and serialize performance for
// various message sizes and structures.

#include <benchmark/benchmark.h>

#include <cstdint>
#include <string>
#include <vector>

#include "benchmarks/benchmark_messages.pb.h"
#include "google/protobuf/arena.h"

namespace {

// Helper functions to create test messages with realistic data

benchmarks::SmallMessage CreateSmallMessage() {
  benchmarks::SmallMessage msg;
  msg.set_id(12345);
  msg.set_name("benchmark_test_message");
  msg.set_active(true);
  msg.set_timestamp(1700000000000);
  msg.set_value(3.14159265359);
  return msg;
}

benchmarks::MediumMessage CreateMediumMessage() {
  benchmarks::MediumMessage msg;
  msg.set_id(67890);
  msg.set_name("medium_benchmark_message");

  for (int i = 0; i < 10; i++) {
    msg.add_tags("tag_" + std::to_string(i));
  }

  for (int i = 0; i < 5; i++) {
    auto* metadata = msg.add_metadata();
    metadata->set_key("key_" + std::to_string(i));
    metadata->set_value("value_" + std::to_string(i));
    metadata->set_timestamp(1700000000000 + i * 1000);
  }

  msg.set_payload(std::string(256, 'x'));
  msg.set_status(benchmarks::MediumMessage::ACTIVE);

  return msg;
}

benchmarks::LargeMessage CreateLargeMessage() {
  benchmarks::LargeMessage msg;
  msg.set_id(111213);
  msg.set_name("large_benchmark_message");
  msg.set_description(
      "This is a large message with many fields used for performance "
      "benchmarking of protobuf serialization and parsing operations.");
  msg.set_created_at(1700000000000);
  msg.set_updated_at(1700000001000);

  auto* author = msg.mutable_author();
  author->set_id(1);
  author->set_name("Test Author");
  author->set_email("test@example.com");

  for (int i = 0; i < 20; i++) {
    msg.add_tags("tag_" + std::to_string(i));
  }

  for (int i = 0; i < 10; i++) {
    auto* comment = msg.add_comments();
    comment->set_id(i);
    comment->set_text("This is comment number " + std::to_string(i) +
                      " with some additional text.");
    auto* comment_author = comment->mutable_author();
    comment_author->set_id(i + 100);
    comment_author->set_name("Commenter " + std::to_string(i));
    comment_author->set_email("commenter" + std::to_string(i) + "@example.com");
    comment->set_timestamp(1700000000000 + i * 60000);
  }

  (*msg.mutable_attributes())["attr1"] = "value1";
  (*msg.mutable_attributes())["attr2"] = "value2";
  (*msg.mutable_attributes())["attr3"] = "value3";

  msg.set_content(std::string(1024, 'y'));
  msg.set_priority(benchmarks::LargeMessage::HIGH);

  for (int i = 0; i < 50; i++) {
    msg.add_related_ids(1000 + i);
  }

  return msg;
}

benchmarks::DeeplyNestedMessage CreateDeeplyNestedMessage() {
  benchmarks::DeeplyNestedMessage msg;
  msg.set_root_value(42);

  auto* l1 = msg.mutable_nested();
  l1->set_flag(true);

  auto* l2 = l1->mutable_nested();
  l2->set_id(12345);

  auto* l3 = l2->mutable_nested();
  l3->set_name("deeply_nested");

  auto* l4 = l3->mutable_nested();
  for (int i = 0; i < 100; i++) {
    l4->add_values(i);
  }

  auto* l5 = l4->mutable_nested();
  l5->set_value(999);
  l5->set_data("deep_data_value");

  return msg;
}

benchmarks::RepeatedFieldsMessage CreateRepeatedFieldsMessage(int size) {
  benchmarks::RepeatedFieldsMessage msg;

  for (int i = 0; i < size; i++) {
    msg.add_int_values(i);
    msg.add_long_values(i * 1000000LL);
    msg.add_float_values(static_cast<float>(i) * 0.1f);
    msg.add_double_values(static_cast<double>(i) * 0.01);
    msg.add_bool_values(i % 2 == 0);
    msg.add_string_values("string_" + std::to_string(i));
    msg.add_bytes_values(std::string(10, static_cast<char>('a' + (i % 26))));

    auto* small = msg.add_messages();
    small->set_id(i);
    small->set_name("nested_" + std::to_string(i));
    small->set_active(true);
    small->set_timestamp(1700000000000 + i);
    small->set_value(static_cast<double>(i));
  }

  return msg;
}

// Parse Benchmarks

static void BM_ParseSmallMessage(benchmark::State& state) {
  std::string data = CreateSmallMessage().SerializeAsString();
  for (auto _ : state) {
    benchmarks::SmallMessage msg;
    msg.ParseFromString(data);
    benchmark::DoNotOptimize(msg);
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_ParseSmallMessage);

static void BM_ParseMediumMessage(benchmark::State& state) {
  std::string data = CreateMediumMessage().SerializeAsString();
  for (auto _ : state) {
    benchmarks::MediumMessage msg;
    msg.ParseFromString(data);
    benchmark::DoNotOptimize(msg);
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_ParseMediumMessage);

static void BM_ParseLargeMessage(benchmark::State& state) {
  std::string data = CreateLargeMessage().SerializeAsString();
  for (auto _ : state) {
    benchmarks::LargeMessage msg;
    msg.ParseFromString(data);
    benchmark::DoNotOptimize(msg);
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_ParseLargeMessage);

static void BM_ParseDeeplyNestedMessage(benchmark::State& state) {
  std::string data = CreateDeeplyNestedMessage().SerializeAsString();
  for (auto _ : state) {
    benchmarks::DeeplyNestedMessage msg;
    msg.ParseFromString(data);
    benchmark::DoNotOptimize(msg);
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_ParseDeeplyNestedMessage);

static void BM_ParseRepeatedFields(benchmark::State& state) {
  std::string data = CreateRepeatedFieldsMessage(state.range(0)).SerializeAsString();
  for (auto _ : state) {
    benchmarks::RepeatedFieldsMessage msg;
    msg.ParseFromString(data);
    benchmark::DoNotOptimize(msg);
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_ParseRepeatedFields)->Range(10, 1000);

// Serialize Benchmarks

static void BM_SerializeSmallMessage(benchmark::State& state) {
  benchmarks::SmallMessage msg = CreateSmallMessage();
  for (auto _ : state) {
    std::string output;
    msg.SerializeToString(&output);
    benchmark::DoNotOptimize(output);
  }
  state.SetBytesProcessed(state.iterations() * msg.ByteSizeLong());
}
BENCHMARK(BM_SerializeSmallMessage);

static void BM_SerializeMediumMessage(benchmark::State& state) {
  benchmarks::MediumMessage msg = CreateMediumMessage();
  for (auto _ : state) {
    std::string output;
    msg.SerializeToString(&output);
    benchmark::DoNotOptimize(output);
  }
  state.SetBytesProcessed(state.iterations() * msg.ByteSizeLong());
}
BENCHMARK(BM_SerializeMediumMessage);

static void BM_SerializeLargeMessage(benchmark::State& state) {
  benchmarks::LargeMessage msg = CreateLargeMessage();
  for (auto _ : state) {
    std::string output;
    msg.SerializeToString(&output);
    benchmark::DoNotOptimize(output);
  }
  state.SetBytesProcessed(state.iterations() * msg.ByteSizeLong());
}
BENCHMARK(BM_SerializeLargeMessage);

static void BM_SerializeDeeplyNestedMessage(benchmark::State& state) {
  benchmarks::DeeplyNestedMessage msg = CreateDeeplyNestedMessage();
  for (auto _ : state) {
    std::string output;
    msg.SerializeToString(&output);
    benchmark::DoNotOptimize(output);
  }
  state.SetBytesProcessed(state.iterations() * msg.ByteSizeLong());
}
BENCHMARK(BM_SerializeDeeplyNestedMessage);

static void BM_SerializeRepeatedFields(benchmark::State& state) {
  benchmarks::RepeatedFieldsMessage msg = CreateRepeatedFieldsMessage(state.range(0));
  for (auto _ : state) {
    std::string output;
    msg.SerializeToString(&output);
    benchmark::DoNotOptimize(output);
  }
  state.SetBytesProcessed(state.iterations() * msg.ByteSizeLong());
}
BENCHMARK(BM_SerializeRepeatedFields)->Range(10, 1000);

// Arena Benchmarks

static void BM_ParseSmallMessageArena(benchmark::State& state) {
  std::string data = CreateSmallMessage().SerializeAsString();
  for (auto _ : state) {
    google::protobuf::Arena arena;
    auto* msg = google::protobuf::Arena::Create<benchmarks::SmallMessage>(&arena);
    msg->ParseFromString(data);
    benchmark::DoNotOptimize(msg);
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_ParseSmallMessageArena);

static void BM_ParseLargeMessageArena(benchmark::State& state) {
  std::string data = CreateLargeMessage().SerializeAsString();
  for (auto _ : state) {
    google::protobuf::Arena arena;
    auto* msg = google::protobuf::Arena::Create<benchmarks::LargeMessage>(&arena);
    msg->ParseFromString(data);
    benchmark::DoNotOptimize(msg);
  }
  state.SetBytesProcessed(state.iterations() * data.size());
}
BENCHMARK(BM_ParseLargeMessageArena);

// ByteSize Benchmarks

static void BM_ByteSizeSmallMessage(benchmark::State& state) {
  benchmarks::SmallMessage msg = CreateSmallMessage();
  for (auto _ : state) {
    benchmark::DoNotOptimize(msg.ByteSizeLong());
  }
}
BENCHMARK(BM_ByteSizeSmallMessage);

static void BM_ByteSizeLargeMessage(benchmark::State& state) {
  benchmarks::LargeMessage msg = CreateLargeMessage();
  for (auto _ : state) {
    benchmark::DoNotOptimize(msg.ByteSizeLong());
  }
}
BENCHMARK(BM_ByteSizeLargeMessage);

// Copy Benchmarks

static void BM_CopySmallMessage(benchmark::State& state) {
  benchmarks::SmallMessage src = CreateSmallMessage();
  for (auto _ : state) {
    benchmarks::SmallMessage dst;
    dst.CopyFrom(src);
    benchmark::DoNotOptimize(dst);
  }
}
BENCHMARK(BM_CopySmallMessage);

static void BM_CopyLargeMessage(benchmark::State& state) {
  benchmarks::LargeMessage src = CreateLargeMessage();
  for (auto _ : state) {
    benchmarks::LargeMessage dst;
    dst.CopyFrom(src);
    benchmark::DoNotOptimize(dst);
  }
}
BENCHMARK(BM_CopyLargeMessage);

// Merge Benchmarks

static void BM_MergeSmallMessage(benchmark::State& state) {
  benchmarks::SmallMessage src = CreateSmallMessage();
  for (auto _ : state) {
    benchmarks::SmallMessage dst;
    dst.MergeFrom(src);
    benchmark::DoNotOptimize(dst);
  }
}
BENCHMARK(BM_MergeSmallMessage);

static void BM_MergeLargeMessage(benchmark::State& state) {
  benchmarks::LargeMessage src = CreateLargeMessage();
  for (auto _ : state) {
    benchmarks::LargeMessage dst;
    dst.MergeFrom(src);
    benchmark::DoNotOptimize(dst);
  }
}
BENCHMARK(BM_MergeLargeMessage);

// Clear Benchmarks

static void BM_ClearSmallMessage(benchmark::State& state) {
  benchmarks::SmallMessage msg = CreateSmallMessage();
  for (auto _ : state) {
    msg.Clear();
    benchmark::DoNotOptimize(msg);
    // Re-populate for next iteration
    state.PauseTiming();
    msg = CreateSmallMessage();
    state.ResumeTiming();
  }
}
BENCHMARK(BM_ClearSmallMessage);

static void BM_ClearLargeMessage(benchmark::State& state) {
  benchmarks::LargeMessage msg = CreateLargeMessage();
  for (auto _ : state) {
    msg.Clear();
    benchmark::DoNotOptimize(msg);
    // Re-populate for next iteration
    state.PauseTiming();
    msg = CreateLargeMessage();
    state.ResumeTiming();
  }
}
BENCHMARK(BM_ClearLargeMessage);

}  // namespace

BENCHMARK_MAIN();
