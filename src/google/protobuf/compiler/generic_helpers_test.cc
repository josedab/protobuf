// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

#include "google/protobuf/compiler/generic_helpers.h"

#include <string>
#include <vector>

#include "google/protobuf/descriptor.pb.h"
#include <gtest/gtest.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace {

// Test helper to create a simple generic message
DescriptorProto CreateGenericMessage(const std::string& name,
                                     const std::vector<std::string>& params) {
  DescriptorProto message;
  message.set_name(name);
  for (const auto& param : params) {
    auto* type_param = message.add_type_parameter();
    type_param->set_name(param);
  }
  return message;
}

// Test helper to create a generic message with defaults
DescriptorProto CreateGenericMessageWithDefaults(
    const std::string& name,
    const std::vector<std::pair<std::string, std::string>>& params) {
  DescriptorProto message;
  message.set_name(name);
  for (const auto& param : params) {
    auto* type_param = message.add_type_parameter();
    type_param->set_name(param.first);
    if (!param.second.empty()) {
      type_param->set_default_type(param.second);
    }
  }
  return message;
}

TEST(GenericHelpersTest, IsGenericMessage) {
  DescriptorProto generic = CreateGenericMessage("Result", {"T"});
  EXPECT_TRUE(IsGenericMessage(generic));

  DescriptorProto non_generic;
  non_generic.set_name("User");
  EXPECT_FALSE(IsGenericMessage(non_generic));
}

TEST(GenericHelpersTest, GenerateMonomorphizedNameSingleParam) {
  std::vector<std::string> args = {"User"};
  std::string name = GenerateMonomorphizedName("Result", args);
  EXPECT_EQ(name, "Result_User");
}

TEST(GenericHelpersTest, GenerateMonomorphizedNameMultipleParams) {
  std::vector<std::string> args = {"User", "ErrorCode"};
  std::string name = GenerateMonomorphizedName("Result", args);
  EXPECT_EQ(name, "Result_User_ErrorCode");
}

TEST(GenericHelpersTest, GenerateMonomorphizedNameQualifiedType) {
  std::vector<std::string> args = {"google.protobuf.Empty"};
  std::string name = GenerateMonomorphizedName("Result", args);
  EXPECT_EQ(name, "Result_google_protobuf_Empty");
}

TEST(GenericHelpersTest, GenerateMonomorphizedNameEmptyArgs) {
  std::vector<std::string> args;
  std::string name = GenerateMonomorphizedName("Result", args);
  EXPECT_EQ(name, "Result");
}

TEST(GenericHelpersTest, BuildTypeParameterMap) {
  DescriptorProto message = CreateGenericMessage("Result", {"T", "E"});
  std::vector<std::string> args = {"User", "ErrorCode"};

  auto map = BuildTypeParameterMap(message, args);

  EXPECT_EQ(map.size(), 2);
  EXPECT_EQ(map["T"], "User");
  EXPECT_EQ(map["E"], "ErrorCode");
}

TEST(GenericHelpersTest, BuildTypeParameterMapWithDefaults) {
  auto message = CreateGenericMessageWithDefaults(
      "Result", {{"T", ""}, {"E", "string"}});
  std::vector<std::string> args = {"User"};  // Only provide T

  auto map = BuildTypeParameterMap(message, args);

  EXPECT_EQ(map.size(), 2);
  EXPECT_EQ(map["T"], "User");
  EXPECT_EQ(map["E"], "string");  // Uses default
}

TEST(GenericHelpersTest, ResolveTypeParameter) {
  std::map<std::string, std::string> type_map = {
      {"T", "User"},
      {"E", "ErrorCode"}};

  // Type parameter resolves to concrete type
  EXPECT_EQ(ResolveTypeParameter("T", type_map), "User");
  EXPECT_EQ(ResolveTypeParameter("E", type_map), "ErrorCode");

  // Non-parameter type passes through
  EXPECT_EQ(ResolveTypeParameter("SomeOtherType", type_map), "SomeOtherType");
}

TEST(GenericHelpersTest, GetTypeParameterNames) {
  DescriptorProto message = CreateGenericMessage("Result", {"T", "E", "F"});

  auto names = GetTypeParameterNames(message);

  EXPECT_EQ(names.size(), 3);
  EXPECT_EQ(names[0], "T");
  EXPECT_EQ(names[1], "E");
  EXPECT_EQ(names[2], "F");
}

TEST(GenericHelpersTest, FormatTypeArgumentList) {
  std::vector<std::string> args = {"User", "ErrorCode"};
  std::string formatted = FormatTypeArgumentList(args);
  EXPECT_EQ(formatted, "<User, ErrorCode>");
}

TEST(GenericHelpersTest, FormatTypeArgumentListEmpty) {
  std::vector<std::string> args;
  std::string formatted = FormatTypeArgumentList(args);
  EXPECT_EQ(formatted, "");
}

TEST(GenericHelpersTest, MonomorphizeMessage) {
  DescriptorProto generic = CreateGenericMessage("Result", {"T"});

  // Add a field that uses the type parameter
  auto* field = generic.add_field();
  field->set_name("value");
  field->set_number(1);
  field->set_generic_type_parameter("T");

  DescriptorProto result;
  std::vector<std::string> args = {"User"};
  MonomorphizeMessage(generic, args, &result);

  EXPECT_EQ(result.name(), "Result_User");
  EXPECT_EQ(result.generic_source_name(), "Result");
  EXPECT_EQ(result.type_parameter_size(), 0);  // No longer generic
  EXPECT_EQ(result.type_argument_size(), 1);
  EXPECT_EQ(result.type_argument(0).parameter_name(), "T");
  EXPECT_EQ(result.type_argument(0).type_name(), "User");

  // Field should have resolved type
  EXPECT_EQ(result.field(0).type_name(), "User");
  EXPECT_FALSE(result.field(0).has_generic_type_parameter());
}

TEST(GenericTypeRegistryTest, RegisterAndRetrieve) {
  GenericTypeRegistry registry;

  DescriptorProto message = CreateGenericMessage("Result", {"T"});
  registry.RegisterGenericMessage(message);

  EXPECT_TRUE(registry.IsGenericMessage("Result"));
  EXPECT_FALSE(registry.IsGenericMessage("Unknown"));

  const DescriptorProto* retrieved = registry.GetGenericMessage("Result");
  EXPECT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->name(), "Result");
}

TEST(GenericTypeRegistryTest, RegisterInstantiation) {
  GenericTypeRegistry registry;

  std::vector<std::string> args = {"User"};
  std::string name = registry.RegisterInstantiation("Result", args);

  EXPECT_EQ(name, "Result_User");

  const auto& instantiations = registry.GetInstantiations();
  EXPECT_EQ(instantiations.size(), 1);

  auto it = instantiations.find("Result_User");
  EXPECT_NE(it, instantiations.end());
  EXPECT_EQ(it->second.first, "Result");
  EXPECT_EQ(it->second.second.size(), 1);
  EXPECT_EQ(it->second.second[0], "User");
}

TEST(GenericHelpersTest, ParseMonomorphizedName) {
  std::string generic_name;
  std::vector<std::string> type_args;

  EXPECT_TRUE(ParseMonomorphizedName("Result_User", &generic_name, &type_args));
  EXPECT_EQ(generic_name, "Result");
  EXPECT_EQ(type_args.size(), 1);
  EXPECT_EQ(type_args[0], "User");
}

TEST(GenericHelpersTest, ParseMonomorphizedNameNoArgs) {
  std::string generic_name;
  std::vector<std::string> type_args;

  EXPECT_TRUE(ParseMonomorphizedName("SimpleMessage", &generic_name, &type_args));
  EXPECT_EQ(generic_name, "SimpleMessage");
  EXPECT_TRUE(type_args.empty());
}

}  // namespace
}  // namespace compiler
}  // namespace protobuf
}  // namespace google
