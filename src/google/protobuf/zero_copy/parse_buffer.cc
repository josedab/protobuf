// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/zero_copy/parse_buffer.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <string>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "absl/log/absl_log.h"
#include "absl/strings/string_view.h"

namespace google {
namespace protobuf {
namespace zero_copy {

std::shared_ptr<ParseBuffer> ParseBuffer::Create(std::string data) {
  return std::make_shared<OwnedParseBuffer>(std::move(data));
}

std::shared_ptr<ParseBuffer> ParseBuffer::Wrap(const char* data, size_t size) {
  return std::make_shared<WrappedParseBuffer>(data, size);
}

MmapBuffer::~MmapBuffer() {
  if (mapped_data_ != nullptr && mapped_data_ != MAP_FAILED) {
    munmap(mapped_data_, mapped_size_);
  }
  if (fd_ >= 0) {
    close(fd_);
  }
}

std::shared_ptr<MmapBuffer> MmapBuffer::Open(const std::string& path) {
  // Open the file
  int fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) {
    ABSL_LOG(ERROR) << "Failed to open file: " << path
                    << " (" << strerror(errno) << ")";
    return nullptr;
  }

  // Get file size
  struct stat st;
  if (fstat(fd, &st) < 0) {
    ABSL_LOG(ERROR) << "Failed to stat file: " << path
                    << " (" << strerror(errno) << ")";
    close(fd);
    return nullptr;
  }

  size_t file_size = static_cast<size_t>(st.st_size);
  if (file_size == 0) {
    // Empty file - return valid but empty buffer
    auto buffer = std::shared_ptr<MmapBuffer>(new MmapBuffer());
    buffer->fd_ = fd;
    buffer->mapped_data_ = nullptr;
    buffer->mapped_size_ = 0;
    buffer->data_ = absl::string_view();
    return buffer;
  }

  // Memory map the file
  void* mapped = mmap(nullptr, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (mapped == MAP_FAILED) {
    ABSL_LOG(ERROR) << "Failed to mmap file: " << path
                    << " (" << strerror(errno) << ")";
    close(fd);
    return nullptr;
  }

  // Create the buffer
  auto buffer = std::shared_ptr<MmapBuffer>(new MmapBuffer());
  buffer->fd_ = fd;
  buffer->mapped_data_ = mapped;
  buffer->mapped_size_ = file_size;
  buffer->data_ = absl::string_view(static_cast<const char*>(mapped), file_size);

  return buffer;
}

}  // namespace zero_copy
}  // namespace protobuf
}  // namespace google
