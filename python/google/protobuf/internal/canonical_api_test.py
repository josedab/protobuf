#!/usr/bin/env python
# Protocol Buffers - Google's data interchange format
# Copyright 2008 Google Inc.  All rights reserved.
#
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file or at
# https://developers.google.com/open-source/licenses/bsd

"""Tests for the canonical API aliases in Message."""

import unittest

from google.protobuf import unittest_pb2


class CanonicalApiTest(unittest.TestCase):
  """Tests for canonical API method aliases."""

  def test_serialize_equivalence(self):
    """Test that serialize() produces the same output as SerializeToString()."""
    message = unittest_pb2.TestAllTypes()
    message.optional_int32 = 123
    message.optional_string = 'test'

    canonical = message.serialize()
    original = message.SerializeToString()

    self.assertEqual(canonical, original)
    self.assertTrue(len(canonical) > 0)

  def test_parse_equivalence(self):
    """Test that parse() works the same as ParseFromString()."""
    original = unittest_pb2.TestAllTypes()
    original.optional_int32 = 456
    original.optional_string = 'hello'

    data = original.serialize()

    # Test canonical parse()
    parsed_canonical = unittest_pb2.TestAllTypes()
    parsed_canonical.parse(data)
    self.assertEqual(parsed_canonical.optional_int32, 456)
    self.assertEqual(parsed_canonical.optional_string, 'hello')

    # Test original ParseFromString()
    parsed_original = unittest_pb2.TestAllTypes()
    parsed_original.ParseFromString(data)
    self.assertEqual(parsed_original.optional_int32, 456)
    self.assertEqual(parsed_original.optional_string, 'hello')

  def test_clear_equivalence(self):
    """Test that clear() works the same as Clear()."""
    message = unittest_pb2.TestAllTypes()
    message.optional_int32 = 789
    message.optional_string = 'world'

    self.assertTrue(message.HasField('optional_string'))

    message.clear()

    # After clear, default values apply
    self.assertEqual(message.optional_int32, 0)
    self.assertEqual(message.optional_string, '')

  def test_is_initialized_equivalence(self):
    """Test that is_initialized() works the same as IsInitialized()."""
    message = unittest_pb2.TestAllTypes()

    self.assertEqual(message.is_initialized(), message.IsInitialized())
    self.assertTrue(message.is_initialized())

    message.optional_int32 = 100
    self.assertEqual(message.is_initialized(), message.IsInitialized())
    self.assertTrue(message.is_initialized())

  def test_get_serialized_size_equivalence(self):
    """Test that get_serialized_size() works the same as ByteSize()."""
    message = unittest_pb2.TestAllTypes()
    message.optional_int32 = 42
    message.optional_string = 'size test'

    canonical_size = message.get_serialized_size()
    original_size = message.ByteSize()

    self.assertEqual(canonical_size, original_size)
    self.assertTrue(canonical_size > 0)

  def test_merge_from_equivalence(self):
    """Test that merge_from() works the same as MergeFrom()."""
    source = unittest_pb2.TestAllTypes()
    source.optional_int32 = 111

    # Test canonical merge_from()
    target_canonical = unittest_pb2.TestAllTypes()
    target_canonical.optional_string = 'existing'
    target_canonical.merge_from(source)
    self.assertEqual(target_canonical.optional_int32, 111)
    self.assertEqual(target_canonical.optional_string, 'existing')

    # Test original MergeFrom()
    target_original = unittest_pb2.TestAllTypes()
    target_original.optional_string = 'existing'
    target_original.MergeFrom(source)
    self.assertEqual(target_original.optional_int32, 111)
    self.assertEqual(target_original.optional_string, 'existing')

  def test_clone(self):
    """Test that clone() creates a proper copy."""
    original = unittest_pb2.TestAllTypes()
    original.optional_int32 = 12345
    original.optional_string = 'clone test'
    original.repeated_int32.append(1)
    original.repeated_int32.append(2)
    original.repeated_int32.append(3)

    cloned = original.clone()

    # Verify contents
    self.assertEqual(cloned.optional_int32, 12345)
    self.assertEqual(cloned.optional_string, 'clone test')
    self.assertEqual(list(cloned.repeated_int32), [1, 2, 3])

    # Verify it's a separate copy
    cloned.optional_int32 = 99999
    self.assertEqual(original.optional_int32, 12345)

  def test_round_trip(self):
    """Test round-trip: serialize -> parse."""
    original = unittest_pb2.TestAllTypes()
    original.optional_int32 = 12345
    original.optional_int64 = 67890
    original.optional_float = 3.14
    original.optional_string = 'round trip test'
    original.repeated_int32.append(1)
    original.repeated_int32.append(2)
    original.repeated_int32.append(3)

    data = original.serialize()

    restored = unittest_pb2.TestAllTypes()
    restored.parse(data)

    self.assertEqual(restored.optional_int32, 12345)
    self.assertEqual(restored.optional_int64, 67890)
    self.assertAlmostEqual(restored.optional_float, 3.14, places=5)
    self.assertEqual(restored.optional_string, 'round trip test')
    self.assertEqual(list(restored.repeated_int32), [1, 2, 3])

  def test_empty_message(self):
    """Test empty message."""
    empty = unittest_pb2.TestAllTypes()

    data = empty.serialize()
    self.assertEqual(len(data), 0)

    parsed = unittest_pb2.TestAllTypes()
    parsed.parse(data)
    self.assertTrue(parsed.is_initialized())
    self.assertEqual(parsed.get_serialized_size(), 0)

  def test_deterministic_serialization(self):
    """Test deterministic serialization parameter."""
    message = unittest_pb2.TestAllTypes()
    message.optional_int32 = 42

    # Test with deterministic=True
    data1 = message.serialize(deterministic=True)
    data2 = message.serialize(deterministic=True)
    self.assertEqual(data1, data2)


if __name__ == '__main__':
  unittest.main()
