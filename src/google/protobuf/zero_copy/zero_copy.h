// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Main header for zero-copy parsing functionality.
// Include this header to access all zero-copy classes.
//
// Example usage:
//
//   #include "google/protobuf/zero_copy/zero_copy.h"
//
//   // Parse with zero-copy string access
//   auto buffer = zero_copy::ParseBuffer::Create(std::move(data));
//   MyMessage msg;
//   auto result = zero_copy::ParseZeroCopy(std::move(data), &msg);
//
//   // Access strings without copying
//   absl::string_view name = msg.name();  // With modified generated code
//
//   // Or use ZeroCopyString directly
//   zero_copy::ZeroCopyString str(buffer, offset, length);
//   absl::string_view view = str.view();  // Zero-copy access
//

#ifndef GOOGLE_PROTOBUF_ZERO_COPY_ZERO_COPY_H__
#define GOOGLE_PROTOBUF_ZERO_COPY_ZERO_COPY_H__

#include "google/protobuf/zero_copy/parse_buffer.h"
#include "google/protobuf/zero_copy/zero_copy_parser.h"
#include "google/protobuf/zero_copy/zero_copy_string.h"

#endif  // GOOGLE_PROTOBUF_ZERO_COPY_ZERO_COPY_H__
