// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Memory tracking for Protocol Buffers parsing.
// Tracks memory allocations during parsing to enforce limits.

#ifndef GOOGLE_PROTOBUF_MEMORY_TRACKER_H__
#define GOOGLE_PROTOBUF_MEMORY_TRACKER_H__

#include <cstddef>
#include <cstdint>

#include "absl/base/thread_annotations.h"

// Must be included last.
#include "google/protobuf/port_def.inc"

namespace google {
namespace protobuf {
namespace internal {

// Tracks memory allocations during parsing to enforce resource limits.
// This class is not thread-safe and should be used only within a single
// parsing operation.
class PROTOBUF_EXPORT MemoryTracker {
 public:
  // Create a tracker with the specified limit (in bytes).
  // A limit of 0 means no limit.
  explicit MemoryTracker(size_t limit = 0)
      : allocated_(0), limit_(limit) {}

  // Attempt to allocate the specified number of bytes.
  // Returns true if the allocation is within limits, false otherwise.
  bool Allocate(size_t bytes) {
    if (limit_ == 0) {
      // No limit
      allocated_ += bytes;
      return true;
    }

    size_t new_total = allocated_ + bytes;
    if (new_total > limit_) {
      return false;  // Would exceed limit
    }
    allocated_ = new_total;
    return true;
  }

  // Record deallocation of the specified number of bytes.
  void Deallocate(size_t bytes) {
    if (bytes > allocated_) {
      allocated_ = 0;
    } else {
      allocated_ -= bytes;
    }
  }

  // Check if an allocation would succeed without actually performing it.
  bool CanAllocate(size_t bytes) const {
    if (limit_ == 0) {
      return true;  // No limit
    }
    return (allocated_ + bytes) <= limit_;
  }

  // Get the current total allocated bytes.
  size_t allocated() const { return allocated_; }

  // Get the configured limit.
  size_t limit() const { return limit_; }

  // Get the remaining allocation budget.
  size_t remaining() const {
    if (limit_ == 0) {
      return SIZE_MAX;  // No limit
    }
    if (allocated_ >= limit_) {
      return 0;
    }
    return limit_ - allocated_;
  }

  // Reset the tracker to initial state.
  void Reset() { allocated_ = 0; }

  // Set a new limit.
  void SetLimit(size_t limit) { limit_ = limit; }

 private:
  size_t allocated_;
  size_t limit_;
};

// RAII helper for tracking allocations in a scope.
class PROTOBUF_EXPORT ScopedAllocation {
 public:
  ScopedAllocation(MemoryTracker* tracker, size_t bytes)
      : tracker_(tracker), bytes_(bytes), succeeded_(false) {
    if (tracker_ != nullptr) {
      succeeded_ = tracker_->Allocate(bytes_);
    } else {
      succeeded_ = true;  // No tracker means no limit
    }
  }

  ~ScopedAllocation() {
    if (tracker_ != nullptr && succeeded_) {
      tracker_->Deallocate(bytes_);
    }
  }

  // Check if the allocation succeeded.
  bool succeeded() const { return succeeded_; }

  // Prevent copying
  ScopedAllocation(const ScopedAllocation&) = delete;
  ScopedAllocation& operator=(const ScopedAllocation&) = delete;

 private:
  MemoryTracker* tracker_;
  size_t bytes_;
  bool succeeded_;
};

}  // namespace internal
}  // namespace protobuf
}  // namespace google

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_MEMORY_TRACKER_H__
