// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

// Defines utilities for the Url well known type.

#ifndef GOOGLE_PROTOBUF_UTIL_URL_UTIL_H__
#define GOOGLE_PROTOBUF_UTIL_URL_UTIL_H__

#include <cstdint>
#include <ostream>
#include <string>

#include "google/protobuf/url.pb.h"
#include "absl/strings/string_view.h"

// Must be included last.
#include "google/protobuf/port_def.inc"

namespace google {
namespace protobuf {
namespace util {

// Utility functions for Url.
class PROTOBUF_EXPORT UrlUtil {
  typedef google::protobuf::Url Url;

 public:
  // Common schemes.
  static constexpr absl::string_view kSchemeHttp = "http";
  static constexpr absl::string_view kSchemeHttps = "https";
  static constexpr absl::string_view kSchemeFtp = "ftp";
  static constexpr absl::string_view kSchemeFile = "file";

  // Default ports for common schemes.
  static constexpr int32_t kDefaultPortHttp = 80;
  static constexpr int32_t kDefaultPortHttps = 443;
  static constexpr int32_t kDefaultPortFtp = 21;

  // Validates the URL according to RFC 3986.
  static bool IsValid(const Url& url);

  // Validates a URL string without parsing.
  static bool IsValidString(absl::string_view url_string);

  // Converts Url to/from string format.
  // Parses the URL and populates all component fields.
  static std::string ToString(const Url& url);
  static bool FromString(absl::string_view value, Url* url);

  // Rebuilds the URL string from components.
  // Useful after modifying individual components.
  static std::string BuildString(const Url& url);

  // Gets the effective port (default port if not specified).
  static int32_t GetEffectivePort(const Url& url);

  // URL encoding/decoding.
  static std::string Encode(absl::string_view value);
  static std::string Decode(absl::string_view value);

  // Query string parsing.
  // Returns false if the query string is malformed.
  static bool ParseQuery(absl::string_view query,
                         std::vector<std::pair<std::string, std::string>>* params);

  // Builds a query string from parameters.
  static std::string BuildQuery(
      const std::vector<std::pair<std::string, std::string>>& params);

  // Resolves a relative URL against a base URL.
  static bool Resolve(const Url& base, absl::string_view relative, Url* result);

  // Normalizes a URL (lowercase scheme/host, remove default port, etc.).
  static void Normalize(Url* url);

  // Checks if two URLs are equivalent after normalization.
  static bool AreEquivalent(const Url& u1, const Url& u2);

  // Creates URL from components.
  static void FromComponents(absl::string_view scheme,
                             absl::string_view host,
                             int32_t port,
                             absl::string_view path,
                             absl::string_view query,
                             absl::string_view fragment,
                             Url* url);
};

}  // namespace util
}  // namespace protobuf
}  // namespace google

namespace google {
namespace protobuf {

// Equality operators for Url (exact match).
inline bool operator==(const Url& u1, const Url& u2) {
  return u1.value() == u2.value();
}

inline bool operator!=(const Url& u1, const Url& u2) {
  return u1.value() != u2.value();
}

inline std::ostream& operator<<(std::ostream& out, const Url& url) {
  out << google::protobuf::util::UrlUtil::ToString(url);
  return out;
}

}  // namespace protobuf
}  // namespace google

#include "google/protobuf/port_undef.inc"

#endif  // GOOGLE_PROTOBUF_UTIL_URL_UTIL_H__
