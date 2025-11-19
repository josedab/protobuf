# Protocol Buffers - Google's data interchange format
# Copyright 2008 Google Inc.  All rights reserved.
#
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file or at
# https://developers.google.com/open-source/licenses/bsd

"""Tests for async_parser module."""

import asyncio
import io
import struct
import unittest

from google.protobuf import async_parser
from google.protobuf import unittest_pb2


def create_delimited_message(message):
    """Create a length-delimited message."""
    serialized = message.SerializeToString()
    size = len(serialized)

    # Encode size as varint
    varint_bytes = []
    while size >= 0x80:
        varint_bytes.append((size & 0x7F) | 0x80)
        size >>= 7
    varint_bytes.append(size)

    return bytes(varint_bytes) + serialized


class IncrementalParserTest(unittest.TestCase):
    """Tests for IncrementalParser."""

    def test_parse_single_message(self):
        """Test parsing a single message."""
        message = unittest_pb2.TestAllTypes()
        message.optional_int32 = 42
        message.optional_string = "hello"

        data = create_delimited_message(message)

        parser = async_parser.IncrementalParser()
        state = parser.feed(data)

        self.assertEqual(state, async_parser.IncrementalParser.MESSAGE_READY)
        self.assertTrue(parser.has_message())

        parsed = parser.take_message(unittest_pb2.TestAllTypes)
        self.assertIsNotNone(parsed)
        self.assertEqual(parsed.optional_int32, 42)
        self.assertEqual(parsed.optional_string, "hello")

    def test_parse_in_chunks(self):
        """Test parsing a message byte by byte."""
        message = unittest_pb2.TestAllTypes()
        message.optional_int32 = 123
        message.optional_string = "chunked"

        data = create_delimited_message(message)

        parser = async_parser.IncrementalParser()

        # Feed one byte at a time
        for i in range(len(data) - 1):
            state = parser.feed(data[i:i+1])
            self.assertEqual(state, async_parser.IncrementalParser.NEED_MORE_DATA)
            self.assertFalse(parser.has_message())

        # Feed the last byte
        state = parser.feed(data[-1:])
        self.assertEqual(state, async_parser.IncrementalParser.MESSAGE_READY)

        parsed = parser.take_message(unittest_pb2.TestAllTypes)
        self.assertIsNotNone(parsed)
        self.assertEqual(parsed.optional_int32, 123)
        self.assertEqual(parsed.optional_string, "chunked")

    def test_parse_multiple_messages(self):
        """Test parsing multiple messages from one feed."""
        data = b""

        for i in range(5):
            message = unittest_pb2.TestAllTypes()
            message.optional_int32 = i * 10
            data += create_delimited_message(message)

        parser = async_parser.IncrementalParser()

        # Feed all data at once
        parser.feed(data)

        # Parse all messages
        for i in range(5):
            self.assertTrue(parser.has_message())
            parsed = parser.take_message(unittest_pb2.TestAllTypes)
            self.assertIsNotNone(parsed)
            self.assertEqual(parsed.optional_int32, i * 10)

        self.assertFalse(parser.has_message())

    def test_max_message_size_enforced(self):
        """Test that maximum message size is enforced."""
        parser = async_parser.IncrementalParser(max_message_size=100)

        # Create a message larger than the limit
        message = unittest_pb2.TestAllTypes()
        message.optional_string = "x" * 200

        data = create_delimited_message(message)

        state = parser.feed(data)
        self.assertEqual(state, async_parser.IncrementalParser.ERROR)
        self.assertIsNotNone(parser.error_message)

    def test_reset(self):
        """Test resetting the parser."""
        message = unittest_pb2.TestAllTypes()
        message.optional_int32 = 42

        data = create_delimited_message(message)

        parser = async_parser.IncrementalParser()

        # Parse partial data
        parser.feed(data[:len(data)//2])

        # Reset
        parser.reset()
        self.assertEqual(parser.state, async_parser.IncrementalParser.NEED_MORE_DATA)
        self.assertEqual(parser.bytes_consumed, 0)

        # Parse complete message
        parser.feed(data)
        self.assertTrue(parser.has_message())

    def test_take_message_when_not_ready(self):
        """Test that take_message returns None when no message is ready."""
        parser = async_parser.IncrementalParser()
        self.assertIsNone(parser.take_message(unittest_pb2.TestAllTypes))

    def test_empty_feed(self):
        """Test feeding empty data."""
        parser = async_parser.IncrementalParser()
        state = parser.feed(b"")
        self.assertEqual(state, async_parser.IncrementalParser.NEED_MORE_DATA)


class AsyncParserTest(unittest.TestCase):
    """Tests for async parser functions."""

    def test_parse_async(self):
        """Test async message parsing."""
        message = unittest_pb2.TestAllTypes()
        message.optional_int32 = 42
        message.optional_string = "async"

        data = message.SerializeToString()

        async def run_test():
            parsed = await async_parser.parse_async(data, unittest_pb2.TestAllTypes)
            self.assertEqual(parsed.optional_int32, 42)
            self.assertEqual(parsed.optional_string, "async")

        asyncio.run(run_test())

    def test_serialize_async(self):
        """Test async message serialization."""
        message = unittest_pb2.TestAllTypes()
        message.optional_int32 = 123

        async def run_test():
            data = await async_parser.serialize_async(message)
            self.assertIsNotNone(data)

            parsed = unittest_pb2.TestAllTypes()
            parsed.ParseFromString(data)
            self.assertEqual(parsed.optional_int32, 123)

        asyncio.run(run_test())

    def test_parse_stream(self):
        """Test parsing a stream of messages."""
        # Create a stream of messages
        messages_data = b""
        for i in range(5):
            message = unittest_pb2.TestAllTypes()
            message.optional_int32 = i * 10
            messages_data += create_delimited_message(message)

        async def async_gen():
            # Yield data in chunks
            chunk_size = 10
            for i in range(0, len(messages_data), chunk_size):
                yield messages_data[i:i+chunk_size]

        async def run_test():
            count = 0
            async for message in async_parser.parse_stream(async_gen(), unittest_pb2.TestAllTypes):
                self.assertEqual(message.optional_int32, count * 10)
                count += 1
            self.assertEqual(count, 5)

        asyncio.run(run_test())


class AsyncSerializerTest(unittest.TestCase):
    """Tests for AsyncSerializer."""

    def test_serialize(self):
        """Test basic async serialization."""
        message = unittest_pb2.TestAllTypes()
        message.optional_int32 = 456

        async def run_test():
            data = await async_parser.AsyncSerializer.serialize(message)
            self.assertIsNotNone(data)

            parsed = unittest_pb2.TestAllTypes()
            parsed.ParseFromString(data)
            self.assertEqual(parsed.optional_int32, 456)

        asyncio.run(run_test())

    def test_serialize_delimited(self):
        """Test serialization with length prefix."""
        message = unittest_pb2.TestAllTypes()
        message.optional_int32 = 789

        async def run_test():
            data = await async_parser.AsyncSerializer.serialize_delimited(message)
            self.assertIsNotNone(data)

            # Parse the length prefix
            pos = 0
            size = 0
            shift = 0
            while pos < len(data):
                byte = data[pos]
                size |= (byte & 0x7F) << shift
                pos += 1
                if (byte & 0x80) == 0:
                    break
                shift += 7

            # Parse the message
            message_bytes = data[pos:pos+size]
            parsed = unittest_pb2.TestAllTypes()
            parsed.ParseFromString(message_bytes)
            self.assertEqual(parsed.optional_int32, 789)

        asyncio.run(run_test())


if __name__ == "__main__":
    unittest.main()
