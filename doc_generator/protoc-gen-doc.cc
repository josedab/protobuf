// Protocol Buffers - Google's data interchange format
// Copyright 2024 Google LLC.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Main entry point for the protoc documentation generator plugin.

#include "google/protobuf/compiler/plugin.h"
#include "doc_generator/doc_generator.h"

int main(int argc, char* argv[]) {
  google::protobuf::doc_generator::DocGenerator generator;
  return google::protobuf::compiler::PluginMain(argc, argv, &generator);
}
