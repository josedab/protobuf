// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/util/url_util.h"

#include <algorithm>
#include <cctype>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_split.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"

namespace google {
namespace protobuf {
namespace util {

namespace {

// Characters that don't need encoding in URLs
bool IsUnreservedChar(char c) {
  return std::isalnum(static_cast<unsigned char>(c)) ||
         c == '-' || c == '_' || c == '.' || c == '~';
}

// Hex character to int
int HexToInt(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'a' && c <= 'f') return c - 'a' + 10;
  if (c >= 'A' && c <= 'F') return c - 'A' + 10;
  return -1;
}

// Int to hex character
char IntToHex(int value) {
  return "0123456789ABCDEF"[value & 0xF];
}

}  // namespace

bool UrlUtil::IsValid(const Url& url) {
  return !url.value().empty() && !url.scheme().empty() && !url.host().empty();
}

bool UrlUtil::IsValidString(absl::string_view url_string) {
  Url url;
  return FromString(url_string, &url);
}

std::string UrlUtil::ToString(const Url& url) {
  return url.value();
}

bool UrlUtil::FromString(absl::string_view value, Url* url) {
  if (value.empty()) {
    return false;
  }

  url->set_value(std::string(value));

  // Parse scheme
  size_t scheme_end = value.find("://");
  if (scheme_end == absl::string_view::npos) {
    return false;
  }

  url->set_scheme(std::string(value.substr(0, scheme_end)));
  value = value.substr(scheme_end + 3);

  // Parse userinfo and host
  size_t path_start = value.find('/');
  size_t query_start = value.find('?');
  size_t fragment_start = value.find('#');

  size_t authority_end = std::min({path_start, query_start, fragment_start});
  if (authority_end == absl::string_view::npos) {
    authority_end = value.size();
  }

  absl::string_view authority = value.substr(0, authority_end);
  value = value.substr(authority_end);

  // Check for userinfo
  size_t at_pos = authority.find('@');
  if (at_pos != absl::string_view::npos) {
    url->set_userinfo(std::string(authority.substr(0, at_pos)));
    authority = authority.substr(at_pos + 1);
  }

  // Parse host and port
  size_t port_start = authority.rfind(':');

  // Check if it's an IPv6 address
  if (authority[0] == '[') {
    size_t bracket_end = authority.find(']');
    if (bracket_end == absl::string_view::npos) {
      return false;
    }
    url->set_host(std::string(authority.substr(1, bracket_end - 1)));
    if (bracket_end + 1 < authority.size() && authority[bracket_end + 1] == ':') {
      port_start = bracket_end + 1;
    } else {
      port_start = absl::string_view::npos;
    }
  } else if (port_start != absl::string_view::npos) {
    url->set_host(std::string(authority.substr(0, port_start)));
  } else {
    url->set_host(std::string(authority));
  }

  // Parse port
  if (port_start != absl::string_view::npos && port_start + 1 < authority.size()) {
    absl::string_view port_str = authority.substr(port_start + 1);
    int port = 0;
    for (char c : port_str) {
      if (c < '0' || c > '9') {
        return false;
      }
      port = port * 10 + (c - '0');
      if (port > 65535) {
        return false;
      }
    }
    url->set_port(port);
  } else {
    url->set_port(0);
  }

  // Parse path
  if (!value.empty() && value[0] == '/') {
    size_t path_end = value.find_first_of("?#");
    if (path_end == absl::string_view::npos) {
      url->set_path(std::string(value));
      value = "";
    } else {
      url->set_path(std::string(value.substr(0, path_end)));
      value = value.substr(path_end);
    }
  }

  // Parse query
  if (!value.empty() && value[0] == '?') {
    size_t query_end = value.find('#');
    if (query_end == absl::string_view::npos) {
      url->set_query(std::string(value.substr(1)));
      value = "";
    } else {
      url->set_query(std::string(value.substr(1, query_end - 1)));
      value = value.substr(query_end);
    }
  }

  // Parse fragment
  if (!value.empty() && value[0] == '#') {
    url->set_fragment(std::string(value.substr(1)));
  }

  return true;
}

std::string UrlUtil::BuildString(const Url& url) {
  std::string result;

  // Scheme
  result += url.scheme();
  result += "://";

  // Userinfo
  if (!url.userinfo().empty()) {
    result += url.userinfo();
    result += "@";
  }

  // Host
  result += url.host();

  // Port (only if non-default)
  if (url.port() > 0) {
    result += ":";
    result += std::to_string(url.port());
  }

  // Path
  result += url.path();

  // Query
  if (!url.query().empty()) {
    result += "?";
    result += url.query();
  }

  // Fragment
  if (!url.fragment().empty()) {
    result += "#";
    result += url.fragment();
  }

  return result;
}

int32_t UrlUtil::GetEffectivePort(const Url& url) {
  if (url.port() > 0) {
    return url.port();
  }

  if (url.scheme() == kSchemeHttp) return kDefaultPortHttp;
  if (url.scheme() == kSchemeHttps) return kDefaultPortHttps;
  if (url.scheme() == kSchemeFtp) return kDefaultPortFtp;

  return 0;
}

std::string UrlUtil::Encode(absl::string_view value) {
  std::string result;
  result.reserve(value.size() * 3);  // Worst case

  for (char c : value) {
    if (IsUnreservedChar(c)) {
      result += c;
    } else {
      result += '%';
      result += IntToHex((static_cast<unsigned char>(c) >> 4) & 0xF);
      result += IntToHex(static_cast<unsigned char>(c) & 0xF);
    }
  }

  return result;
}

std::string UrlUtil::Decode(absl::string_view value) {
  std::string result;
  result.reserve(value.size());

  for (size_t i = 0; i < value.size(); ++i) {
    if (value[i] == '%' && i + 2 < value.size()) {
      int hi = HexToInt(value[i + 1]);
      int lo = HexToInt(value[i + 2]);
      if (hi >= 0 && lo >= 0) {
        result += static_cast<char>((hi << 4) | lo);
        i += 2;
        continue;
      }
    }
    if (value[i] == '+') {
      result += ' ';
    } else {
      result += value[i];
    }
  }

  return result;
}

bool UrlUtil::ParseQuery(
    absl::string_view query,
    std::vector<std::pair<std::string, std::string>>* params) {
  params->clear();

  if (query.empty()) {
    return true;
  }

  std::vector<absl::string_view> pairs = absl::StrSplit(query, '&');

  for (const auto& pair : pairs) {
    size_t eq_pos = pair.find('=');
    if (eq_pos == absl::string_view::npos) {
      params->emplace_back(Decode(pair), "");
    } else {
      params->emplace_back(Decode(pair.substr(0, eq_pos)),
                          Decode(pair.substr(eq_pos + 1)));
    }
  }

  return true;
}

std::string UrlUtil::BuildQuery(
    const std::vector<std::pair<std::string, std::string>>& params) {
  std::string result;

  for (size_t i = 0; i < params.size(); ++i) {
    if (i > 0) {
      result += "&";
    }
    result += Encode(params[i].first);
    result += "=";
    result += Encode(params[i].second);
  }

  return result;
}

bool UrlUtil::Resolve(const Url& base, absl::string_view relative, Url* result) {
  // If relative is absolute, just parse it
  if (relative.find("://") != absl::string_view::npos) {
    return FromString(relative, result);
  }

  *result = base;

  if (relative.empty()) {
    return true;
  }

  if (relative[0] == '/') {
    if (relative.size() > 1 && relative[1] == '/') {
      // Protocol-relative URL
      return FromString(absl::StrCat(base.scheme(), ":", relative), result);
    }
    // Absolute path
    result->set_path(std::string(relative));
    result->set_query("");
    result->set_fragment("");
  } else if (relative[0] == '?') {
    result->set_query(std::string(relative.substr(1)));
    result->set_fragment("");
  } else if (relative[0] == '#') {
    result->set_fragment(std::string(relative.substr(1)));
  } else {
    // Relative path
    std::string base_path = base.path();
    size_t last_slash = base_path.rfind('/');
    if (last_slash != std::string::npos) {
      base_path = base_path.substr(0, last_slash + 1);
    } else {
      base_path = "/";
    }
    result->set_path(base_path + std::string(relative));
  }

  result->set_value(BuildString(*result));
  return true;
}

void UrlUtil::Normalize(Url* url) {
  // Lowercase scheme
  std::string scheme = url->scheme();
  std::transform(scheme.begin(), scheme.end(), scheme.begin(),
                 [](char c) { return std::tolower(c); });
  url->set_scheme(scheme);

  // Lowercase host
  std::string host = url->host();
  std::transform(host.begin(), host.end(), host.begin(),
                 [](char c) { return std::tolower(c); });
  url->set_host(host);

  // Remove default port
  int32_t default_port = 0;
  if (scheme == "http") default_port = 80;
  else if (scheme == "https") default_port = 443;
  else if (scheme == "ftp") default_port = 21;

  if (url->port() == default_port) {
    url->set_port(0);
  }

  // Ensure path has leading slash
  if (url->path().empty()) {
    url->set_path("/");
  }

  url->set_value(BuildString(*url));
}

bool UrlUtil::AreEquivalent(const Url& u1, const Url& u2) {
  Url n1 = u1, n2 = u2;
  Normalize(&n1);
  Normalize(&n2);
  return n1.value() == n2.value();
}

void UrlUtil::FromComponents(absl::string_view scheme,
                              absl::string_view host,
                              int32_t port,
                              absl::string_view path,
                              absl::string_view query,
                              absl::string_view fragment,
                              Url* url) {
  url->set_scheme(std::string(scheme));
  url->set_host(std::string(host));
  url->set_port(port);
  url->set_path(std::string(path));
  url->set_query(std::string(query));
  url->set_fragment(std::string(fragment));
  url->set_value(BuildString(*url));
}

}  // namespace util
}  // namespace protobuf
}  // namespace google
