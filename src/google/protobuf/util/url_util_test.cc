// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/util/url_util.h"

#include <cstdint>

#include "google/protobuf/url.pb.h"
#include "google/protobuf/testing/googletest.h"
#include <gtest/gtest.h>

namespace google {
namespace protobuf {
namespace util {

using google::protobuf::Url;

namespace {

TEST(UrlUtilTest, BasicParsing) {
  Url url;

  // Simple URL
  EXPECT_TRUE(UrlUtil::FromString("https://example.com", &url));
  EXPECT_EQ("https", url.scheme());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ(0, url.port());
  EXPECT_EQ("", url.path());

  // URL with path
  EXPECT_TRUE(UrlUtil::FromString("https://example.com/path/to/resource", &url));
  EXPECT_EQ("/path/to/resource", url.path());

  // URL with port
  EXPECT_TRUE(UrlUtil::FromString("https://example.com:8080", &url));
  EXPECT_EQ(8080, url.port());

  // URL with query
  EXPECT_TRUE(UrlUtil::FromString("https://example.com?key=value", &url));
  EXPECT_EQ("key=value", url.query());

  // URL with fragment
  EXPECT_TRUE(UrlUtil::FromString("https://example.com#section", &url));
  EXPECT_EQ("section", url.fragment());

  // URL with userinfo
  EXPECT_TRUE(UrlUtil::FromString("https://user:pass@example.com", &url));
  EXPECT_EQ("user:pass", url.userinfo());
}

TEST(UrlUtilTest, CompleteUrl) {
  Url url;
  EXPECT_TRUE(UrlUtil::FromString(
      "https://user:pass@example.com:8080/path?query=value#fragment", &url));

  EXPECT_EQ("https", url.scheme());
  EXPECT_EQ("user:pass", url.userinfo());
  EXPECT_EQ("example.com", url.host());
  EXPECT_EQ(8080, url.port());
  EXPECT_EQ("/path", url.path());
  EXPECT_EQ("query=value", url.query());
  EXPECT_EQ("fragment", url.fragment());
}

TEST(UrlUtilTest, InvalidUrls) {
  Url url;

  EXPECT_FALSE(UrlUtil::FromString("", &url));
  EXPECT_FALSE(UrlUtil::FromString("not-a-url", &url));
  EXPECT_FALSE(UrlUtil::FromString("://missing-scheme.com", &url));
}

TEST(UrlUtilTest, Validation) {
  Url url;
  UrlUtil::FromString("https://example.com/path", &url);
  EXPECT_TRUE(UrlUtil::IsValid(url));

  EXPECT_TRUE(UrlUtil::IsValidString("https://example.com"));
  EXPECT_FALSE(UrlUtil::IsValidString("not-a-url"));
}

TEST(UrlUtilTest, BuildString) {
  Url url;
  UrlUtil::FromComponents("https", "example.com", 8080, "/path", "key=value",
                          "section", &url);

  std::string built = UrlUtil::BuildString(url);
  EXPECT_EQ("https://example.com:8080/path?key=value#section", built);
}

TEST(UrlUtilTest, EffectivePort) {
  Url url;

  UrlUtil::FromString("http://example.com", &url);
  EXPECT_EQ(80, UrlUtil::GetEffectivePort(url));

  UrlUtil::FromString("https://example.com", &url);
  EXPECT_EQ(443, UrlUtil::GetEffectivePort(url));

  UrlUtil::FromString("https://example.com:8080", &url);
  EXPECT_EQ(8080, UrlUtil::GetEffectivePort(url));
}

TEST(UrlUtilTest, Encoding) {
  EXPECT_EQ("hello%20world", UrlUtil::Encode("hello world"));
  EXPECT_EQ("hello+world", UrlUtil::Encode("hello+world"));
  EXPECT_EQ("a%26b%3Dc", UrlUtil::Encode("a&b=c"));

  EXPECT_EQ("hello world", UrlUtil::Decode("hello%20world"));
  EXPECT_EQ("hello world", UrlUtil::Decode("hello+world"));
  EXPECT_EQ("a&b=c", UrlUtil::Decode("a%26b%3Dc"));
}

TEST(UrlUtilTest, QueryParsing) {
  std::vector<std::pair<std::string, std::string>> params;

  EXPECT_TRUE(UrlUtil::ParseQuery("key1=value1&key2=value2", &params));
  EXPECT_EQ(2u, params.size());
  EXPECT_EQ("key1", params[0].first);
  EXPECT_EQ("value1", params[0].second);
  EXPECT_EQ("key2", params[1].first);
  EXPECT_EQ("value2", params[1].second);

  // Empty query
  EXPECT_TRUE(UrlUtil::ParseQuery("", &params));
  EXPECT_EQ(0u, params.size());

  // Key without value
  EXPECT_TRUE(UrlUtil::ParseQuery("key", &params));
  EXPECT_EQ(1u, params.size());
  EXPECT_EQ("key", params[0].first);
  EXPECT_EQ("", params[0].second);
}

TEST(UrlUtilTest, QueryBuilding) {
  std::vector<std::pair<std::string, std::string>> params = {
      {"key1", "value1"},
      {"key2", "value 2"},
  };

  std::string query = UrlUtil::BuildQuery(params);
  EXPECT_EQ("key1=value1&key2=value%202", query);
}

TEST(UrlUtilTest, Normalize) {
  Url url;

  // Lowercase scheme and host
  UrlUtil::FromString("HTTPS://EXAMPLE.COM/path", &url);
  UrlUtil::Normalize(&url);
  EXPECT_EQ("https", url.scheme());
  EXPECT_EQ("example.com", url.host());

  // Remove default port
  UrlUtil::FromString("https://example.com:443/path", &url);
  UrlUtil::Normalize(&url);
  EXPECT_EQ(0, url.port());

  // Ensure leading slash
  UrlUtil::FromString("https://example.com", &url);
  UrlUtil::Normalize(&url);
  EXPECT_EQ("/", url.path());
}

TEST(UrlUtilTest, AreEquivalent) {
  Url u1, u2;
  UrlUtil::FromString("HTTPS://EXAMPLE.COM:443/path", &u1);
  UrlUtil::FromString("https://example.com/path", &u2);
  EXPECT_TRUE(UrlUtil::AreEquivalent(u1, u2));

  UrlUtil::FromString("https://example.com/path", &u1);
  UrlUtil::FromString("https://example.com/other", &u2);
  EXPECT_FALSE(UrlUtil::AreEquivalent(u1, u2));
}

TEST(UrlUtilTest, Resolve) {
  Url base, result;
  UrlUtil::FromString("https://example.com/path/page", &base);

  // Absolute URL
  EXPECT_TRUE(UrlUtil::Resolve(base, "https://other.com/new", &result));
  EXPECT_EQ("https://other.com/new", result.value());

  // Absolute path
  EXPECT_TRUE(UrlUtil::Resolve(base, "/newpath", &result));
  EXPECT_EQ("/newpath", result.path());
  EXPECT_EQ("example.com", result.host());

  // Relative path
  EXPECT_TRUE(UrlUtil::Resolve(base, "sibling", &result));
  EXPECT_EQ("/path/sibling", result.path());

  // Query only
  EXPECT_TRUE(UrlUtil::Resolve(base, "?key=value", &result));
  EXPECT_EQ("key=value", result.query());

  // Fragment only
  EXPECT_TRUE(UrlUtil::Resolve(base, "#section", &result));
  EXPECT_EQ("section", result.fragment());
}

TEST(UrlUtilTest, IPv6) {
  Url url;
  EXPECT_TRUE(UrlUtil::FromString("https://[::1]:8080/path", &url));
  EXPECT_EQ("::1", url.host());
  EXPECT_EQ(8080, url.port());
}

TEST(UrlUtilTest, Comparison) {
  Url u1, u2;
  UrlUtil::FromString("https://example.com", &u1);
  UrlUtil::FromString("https://example.com", &u2);
  EXPECT_EQ(u1, u2);

  UrlUtil::FromString("https://other.com", &u2);
  EXPECT_NE(u1, u2);
}

TEST(UrlUtilTest, StreamOutput) {
  Url url;
  UrlUtil::FromString("https://example.com/path", &url);

  std::ostringstream oss;
  oss << url;
  EXPECT_EQ("https://example.com/path", oss.str());
}

}  // namespace

}  // namespace util
}  // namespace protobuf
}  // namespace google
