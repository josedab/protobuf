# Protocol Buffers - Google's data interchange format
# Copyright 2008 Google Inc.  All rights reserved.
#
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file or at
# https://developers.google.com/open-source/licenses/bsd

"""Async/streaming API for Protocol Buffers.

This module provides asynchronous and streaming interfaces for parsing
and serializing protocol buffer messages. It supports:
- Async/await parsing
- Async generators for streaming
- Incremental parsing for network streams

Example usage:

    import asyncio
    from google.protobuf import async_parser
    from myproto_pb2 import Person

    async def main():
        # Parse a single message
        data = get_message_bytes()
        person = await async_parser.parse_async(data, Person)
        print(person.name)

        # Stream multiple messages
        async for entry in async_parser.parse_stream(network_stream, LogEntry):
            await process_entry(entry)

    asyncio.run(main())
"""

import asyncio
import struct
from typing import AsyncIterator, Type, TypeVar, Optional, Callable, Any

from google.protobuf import message as message_module

# Type variable for message types
MessageType = TypeVar('MessageType', bound=message_module.Message)


class IncrementalParser:
    """State machine for incremental/streaming message parsing.

    This class allows parsing protocol buffer messages as bytes arrive
    incrementally, without requiring the entire message to be available
    in memory.

    Example:
        parser = IncrementalParser()
        while True:
            data = network.recv()
            parser.feed(data)
            while parser.has_message():
                msg = parser.take_message(Person)
                process(msg)
    """

    # Parser states
    NEED_MORE_DATA = 'need_more_data'
    MESSAGE_READY = 'message_ready'
    ERROR = 'error'
    DONE = 'done'

    def __init__(self, max_message_size: int = 64 * 1024 * 1024):
        """Initialize the parser.

        Args:
            max_message_size: Maximum allowed message size in bytes.
        """
        self._max_message_size = max_message_size
        self._buffer = bytearray()
        self._state = self.NEED_MORE_DATA
        self._message_size: Optional[int] = None
        self._bytes_consumed = 0
        self._error_message: Optional[str] = None
        self._message_bytes: Optional[bytes] = None

        # Varint parsing state
        self._varint_value = 0
        self._varint_shift = 0

    def feed(self, data: bytes) -> str:
        """Feed bytes to the parser incrementally.

        Args:
            data: Bytes to feed to the parser.

        Returns:
            The current parser state after processing.
        """
        if self._state in (self.ERROR, self.DONE):
            return self._state

        self._buffer.extend(data)
        self._try_parse()
        return self._state

    def _try_parse(self):
        """Try to parse a message from the buffer."""
        pos = 0

        # Parse message size if we don't have it
        if self._message_size is None:
            while pos < len(self._buffer):
                byte = self._buffer[pos]
                self._varint_value |= (byte & 0x7F) << self._varint_shift
                pos += 1

                if (byte & 0x80) == 0:
                    # Varint complete
                    self._message_size = self._varint_value
                    self._varint_value = 0
                    self._varint_shift = 0

                    if self._message_size > self._max_message_size:
                        self._state = self.ERROR
                        self._error_message = f"Message size {self._message_size} exceeds maximum {self._max_message_size}"
                        return

                    # Remove consumed varint bytes
                    del self._buffer[:pos]
                    break

                self._varint_shift += 7
                if self._varint_shift >= 35:
                    self._state = self.ERROR
                    self._error_message = "Varint too long"
                    return

        # Check if we have enough bytes for the message
        if self._message_size is not None:
            if len(self._buffer) >= self._message_size:
                # Extract message bytes
                self._message_bytes = bytes(self._buffer[:self._message_size])
                del self._buffer[:self._message_size]
                self._bytes_consumed += self._message_size
                self._state = self.MESSAGE_READY
            else:
                self._state = self.NEED_MORE_DATA
        else:
            self._state = self.NEED_MORE_DATA

    @property
    def state(self) -> str:
        """Get the current parser state."""
        return self._state

    def has_message(self) -> bool:
        """Check if a complete message is ready to be taken."""
        return self._state == self.MESSAGE_READY

    def take_message(self, message_type: Type[MessageType]) -> Optional[MessageType]:
        """Take the parsed message.

        Args:
            message_type: The message class to parse into.

        Returns:
            The parsed message, or None if no message is ready.
        """
        if self._state != self.MESSAGE_READY or self._message_bytes is None:
            return None

        try:
            message = message_type()
            message.ParseFromString(self._message_bytes)

            # Reset for next message
            self._message_bytes = None
            self._message_size = None

            # Check if there's more data to parse
            if self._buffer:
                self._try_parse()
            else:
                self._state = self.NEED_MORE_DATA

            return message
        except Exception as e:
            self._state = self.ERROR
            self._error_message = f"Failed to parse message: {e}"
            return None

    @property
    def bytes_consumed(self) -> int:
        """Get the number of bytes consumed so far."""
        return self._bytes_consumed

    @property
    def error_message(self) -> Optional[str]:
        """Get the last error message."""
        return self._error_message

    def reset(self):
        """Reset the parser to its initial state."""
        self._buffer.clear()
        self._state = self.NEED_MORE_DATA
        self._message_size = None
        self._bytes_consumed = 0
        self._error_message = None
        self._message_bytes = None
        self._varint_value = 0
        self._varint_shift = 0

    def finish(self):
        """Mark parsing as done (no more messages expected)."""
        if self._state == self.NEED_MORE_DATA and not self._buffer:
            self._state = self.DONE


async def parse_async(data: bytes, message_type: Type[MessageType]) -> MessageType:
    """Parse a message asynchronously.

    This function runs the parsing in an executor to avoid blocking
    the event loop for large messages.

    Args:
        data: The serialized message bytes.
        message_type: The message class to parse into.

    Returns:
        The parsed message.

    Raises:
        google.protobuf.message.DecodeError: If parsing fails.
    """
    loop = asyncio.get_event_loop()

    def parse():
        message = message_type()
        message.ParseFromString(data)
        return message

    return await loop.run_in_executor(None, parse)


async def serialize_async(message: message_module.Message) -> bytes:
    """Serialize a message asynchronously.

    Args:
        message: The message to serialize.

    Returns:
        The serialized bytes.
    """
    loop = asyncio.get_event_loop()
    return await loop.run_in_executor(None, message.SerializeToString)


async def parse_stream(
    stream: AsyncIterator[bytes],
    message_type: Type[MessageType]
) -> AsyncIterator[MessageType]:
    """Yield messages from an async stream.

    This is an async generator that parses length-delimited messages
    from an async byte stream.

    Args:
        stream: An async iterator yielding bytes.
        message_type: The message class to parse into.

    Yields:
        Parsed messages.

    Example:
        async for message in parse_stream(network_stream, Person):
            await process_person(message)
    """
    parser = IncrementalParser()

    async for chunk in stream:
        parser.feed(chunk)
        while parser.has_message():
            message = parser.take_message(message_type)
            if message is not None:
                yield message


async def parse_delimited_stream(
    reader: asyncio.StreamReader,
    message_type: Type[MessageType]
) -> AsyncIterator[MessageType]:
    """Parse length-delimited messages from an asyncio StreamReader.

    Args:
        reader: An asyncio StreamReader.
        message_type: The message class to parse into.

    Yields:
        Parsed messages.
    """
    parser = IncrementalParser()

    while True:
        chunk = await reader.read(4096)
        if not chunk:
            break

        parser.feed(chunk)
        while parser.has_message():
            message = parser.take_message(message_type)
            if message is not None:
                yield message


class AsyncSerializer:
    """Async serialization utilities for protocol buffers."""

    @staticmethod
    async def serialize(message: message_module.Message) -> bytes:
        """Serialize a message asynchronously."""
        return await serialize_async(message)

    @staticmethod
    async def serialize_delimited(message: message_module.Message) -> bytes:
        """Serialize a message with a length prefix.

        Args:
            message: The message to serialize.

        Returns:
            The serialized bytes with a varint length prefix.
        """
        loop = asyncio.get_event_loop()

        def serialize():
            data = message.SerializeToString()
            size = len(data)

            # Encode size as varint
            varint_bytes = []
            while size >= 0x80:
                varint_bytes.append((size & 0x7F) | 0x80)
                size >>= 7
            varint_bytes.append(size)

            return bytes(varint_bytes) + data

        return await loop.run_in_executor(None, serialize)

    @staticmethod
    async def serialize_stream(
        messages: AsyncIterator[message_module.Message],
        writer: asyncio.StreamWriter
    ):
        """Serialize and write messages to a stream.

        Args:
            messages: An async iterator of messages.
            writer: An asyncio StreamWriter.
        """
        async for message in messages:
            data = await AsyncSerializer.serialize_delimited(message)
            writer.write(data)
            await writer.drain()


class MessageStreamReader:
    """Read messages from an async stream with flow control.

    This class provides a higher-level interface for reading messages
    from an async stream with support for backpressure and buffering.
    """

    def __init__(
        self,
        reader: asyncio.StreamReader,
        message_type: Type[MessageType],
        buffer_size: int = 10
    ):
        """Initialize the stream reader.

        Args:
            reader: An asyncio StreamReader.
            message_type: The message class to parse into.
            buffer_size: Maximum number of messages to buffer.
        """
        self._reader = reader
        self._message_type = message_type
        self._buffer_size = buffer_size
        self._queue: asyncio.Queue = asyncio.Queue(maxsize=buffer_size)
        self._parser = IncrementalParser()
        self._task: Optional[asyncio.Task] = None
        self._closed = False

    async def start(self):
        """Start reading messages in the background."""
        self._task = asyncio.create_task(self._read_loop())

    async def _read_loop(self):
        """Background task that reads and parses messages."""
        try:
            while not self._closed:
                chunk = await self._reader.read(4096)
                if not chunk:
                    break

                self._parser.feed(chunk)
                while self._parser.has_message():
                    message = self._parser.take_message(self._message_type)
                    if message is not None:
                        await self._queue.put(message)
        except asyncio.CancelledError:
            pass
        finally:
            await self._queue.put(None)  # Signal end of stream

    async def read(self) -> Optional[MessageType]:
        """Read the next message from the stream.

        Returns:
            The next message, or None if the stream is exhausted.
        """
        message = await self._queue.get()
        return message

    async def __aiter__(self):
        """Iterate over messages in the stream."""
        while True:
            message = await self.read()
            if message is None:
                break
            yield message

    async def close(self):
        """Close the stream reader."""
        self._closed = True
        if self._task:
            self._task.cancel()
            try:
                await self._task
            except asyncio.CancelledError:
                pass


# Convenience functions

async def read_delimited_message(
    reader: asyncio.StreamReader,
    message_type: Type[MessageType]
) -> Optional[MessageType]:
    """Read a single length-delimited message from a stream.

    Args:
        reader: An asyncio StreamReader.
        message_type: The message class to parse into.

    Returns:
        The parsed message, or None if the stream is exhausted.
    """
    # Read varint size
    size = 0
    shift = 0
    while True:
        byte_data = await reader.read(1)
        if not byte_data:
            return None

        byte = byte_data[0]
        size |= (byte & 0x7F) << shift
        if (byte & 0x80) == 0:
            break
        shift += 7
        if shift >= 35:
            raise ValueError("Varint too long")

    # Read message bytes
    data = await reader.readexactly(size)

    message = message_type()
    message.ParseFromString(data)
    return message


async def write_delimited_message(
    writer: asyncio.StreamWriter,
    message: message_module.Message
):
    """Write a length-delimited message to a stream.

    Args:
        writer: An asyncio StreamWriter.
        message: The message to write.
    """
    data = await AsyncSerializer.serialize_delimited(message)
    writer.write(data)
    await writer.drain()
