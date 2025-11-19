// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

package com.google.protobuf;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

/**
 * IncrementalParser provides a state machine for parsing protocol buffer messages
 * incrementally as bytes become available. This is useful for streaming scenarios
 * where data arrives in chunks.
 *
 * <p>Example usage:
 * <pre>{@code
 * IncrementalParser<Person> parser = new IncrementalParser<>(Person.parser());
 * while (true) {
 *     byte[] bytes = network.read();
 *     IncrementalParser.State state = parser.feed(bytes);
 *     if (state == IncrementalParser.State.MESSAGE_READY) {
 *         Person person = parser.takeMessage();
 *         processMessage(person);
 *     }
 * }
 * }</pre>
 *
 * @param <T> the message type being parsed
 */
public class IncrementalParser<T extends Message> {

  /** Parser states */
  public enum State {
    /** Parser needs more bytes to complete message */
    NEED_MORE_DATA,
    /** A complete message is ready to be taken */
    MESSAGE_READY,
    /** An error occurred during parsing */
    ERROR,
    /** Parsing is complete (no more messages expected) */
    DONE
  }

  private final Parser<T> messageParser;
  private final ByteArrayOutputStream buffer;
  private final long maxMessageSize;

  private State state;
  private int messageSize;
  private boolean haveMessageSize;
  private long bytesConsumed;
  private String errorMessage;
  private T parsedMessage;

  // Varint parsing state
  private int varintValue;
  private int varintShift;

  /** Default maximum message size (64MB) */
  public static final long DEFAULT_MAX_MESSAGE_SIZE = 64 * 1024 * 1024;

  /**
   * Creates an IncrementalParser with default settings.
   *
   * @param parser the parser for the message type
   */
  public IncrementalParser(Parser<T> parser) {
    this(parser, DEFAULT_MAX_MESSAGE_SIZE);
  }

  /**
   * Creates an IncrementalParser with a custom maximum message size.
   *
   * @param parser the parser for the message type
   * @param maxMessageSize maximum allowed message size in bytes
   */
  public IncrementalParser(Parser<T> parser, long maxMessageSize) {
    this.messageParser = parser;
    this.maxMessageSize = maxMessageSize;
    this.buffer = new ByteArrayOutputStream();
    this.state = State.NEED_MORE_DATA;
    this.messageSize = 0;
    this.haveMessageSize = false;
    this.bytesConsumed = 0;
    this.varintValue = 0;
    this.varintShift = 0;
  }

  /**
   * Feed bytes to the parser incrementally.
   *
   * @param bytes the bytes to feed
   * @return the current state after processing the bytes
   */
  public State feed(byte[] bytes) {
    return feed(bytes, 0, bytes.length);
  }

  /**
   * Feed bytes to the parser incrementally.
   *
   * @param bytes the byte array containing data
   * @param offset the offset in the array
   * @param length the number of bytes to process
   * @return the current state after processing the bytes
   */
  public State feed(byte[] bytes, int offset, int length) {
    if (state == State.ERROR || state == State.DONE) {
      return state;
    }

    int pos = offset;
    int end = offset + length;

    while (pos < end) {
      if (!haveMessageSize) {
        // Parse the varint size prefix
        while (pos < end) {
          byte b = bytes[pos++];
          varintValue |= (b & 0x7F) << varintShift;

          if ((b & 0x80) == 0) {
            // Varint complete
            messageSize = varintValue;
            varintValue = 0;
            varintShift = 0;

            if (messageSize > maxMessageSize) {
              state = State.ERROR;
              errorMessage = "Message size exceeds maximum: " + messageSize;
              return state;
            }

            haveMessageSize = true;
            break;
          }

          varintShift += 7;
          if (varintShift >= 35) {
            state = State.ERROR;
            errorMessage = "Varint too long";
            return state;
          }
        }
      }

      if (haveMessageSize) {
        // Read message bytes
        int remaining = messageSize - buffer.size();
        int available = end - pos;
        int toRead = Math.min(remaining, available);

        if (toRead > 0) {
          buffer.write(bytes, pos, toRead);
          pos += toRead;
          bytesConsumed += toRead;
        }

        if (buffer.size() >= messageSize) {
          // Message complete, parse it
          try {
            parsedMessage = messageParser.parseFrom(buffer.toByteArray());
            state = State.MESSAGE_READY;
            return state;
          } catch (InvalidProtocolBufferException e) {
            state = State.ERROR;
            errorMessage = "Failed to parse message: " + e.getMessage();
            return state;
          }
        }
      }
    }

    state = State.NEED_MORE_DATA;
    return state;
  }

  /**
   * Feed bytes from a ByteBuffer to the parser.
   *
   * @param buffer the ByteBuffer to read from
   * @return the current state after processing the bytes
   */
  public State feed(ByteBuffer buffer) {
    if (buffer.hasArray()) {
      return feed(buffer.array(), buffer.arrayOffset() + buffer.position(), buffer.remaining());
    } else {
      byte[] bytes = new byte[buffer.remaining()];
      buffer.get(bytes);
      return feed(bytes);
    }
  }

  /**
   * Get the current parser state.
   *
   * @return the current state
   */
  public State getState() {
    return state;
  }

  /**
   * Check if a complete message is ready to be taken.
   *
   * @return true if a message is ready
   */
  public boolean hasMessage() {
    return state == State.MESSAGE_READY;
  }

  /**
   * Take the parsed message. The parser will reset for the next message.
   *
   * @return the parsed message, or null if no message is ready
   */
  public T takeMessage() {
    if (state != State.MESSAGE_READY) {
      return null;
    }

    T message = parsedMessage;
    parsedMessage = null;
    buffer.reset();
    haveMessageSize = false;
    messageSize = 0;
    state = State.NEED_MORE_DATA;

    return message;
  }

  /**
   * Get the number of bytes consumed so far.
   *
   * @return bytes consumed
   */
  public long getBytesConsumed() {
    return bytesConsumed;
  }

  /**
   * Get the last error message.
   *
   * @return the error message, or null if no error
   */
  public String getErrorMessage() {
    return errorMessage;
  }

  /**
   * Reset the parser to its initial state.
   */
  public void reset() {
    state = State.NEED_MORE_DATA;
    buffer.reset();
    haveMessageSize = false;
    messageSize = 0;
    bytesConsumed = 0;
    varintValue = 0;
    varintShift = 0;
    errorMessage = null;
    parsedMessage = null;
  }

  /**
   * Mark parsing as done (no more messages expected).
   */
  public void finish() {
    if (state == State.NEED_MORE_DATA && buffer.size() == 0) {
      state = State.DONE;
    }
  }
}
