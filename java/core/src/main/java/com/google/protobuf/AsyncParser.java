// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

package com.google.protobuf;

import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionException;
import java.util.concurrent.Executor;
import java.util.concurrent.ForkJoinPool;
import java.util.function.Consumer;

/**
 * AsyncParser provides asynchronous parsing capabilities for protocol buffer messages
 * using CompletableFuture. This enables non-blocking message parsing that integrates
 * well with modern async/reactive Java applications.
 *
 * <p>Example usage:
 * <pre>{@code
 * AsyncParser asyncParser = new AsyncParser();
 * CompletableFuture<Person> future = asyncParser.parseAsync(inputStream, Person.parser());
 * future.thenAccept(person -> {
 *     System.out.println("Parsed: " + person.getName());
 * });
 * }</pre>
 */
public class AsyncParser {

  private final Executor executor;

  /** Creates an AsyncParser using the common ForkJoinPool. */
  public AsyncParser() {
    this(ForkJoinPool.commonPool());
  }

  /**
   * Creates an AsyncParser with a custom executor.
   *
   * @param executor the executor to use for async operations
   */
  public AsyncParser(Executor executor) {
    this.executor = executor;
  }

  /**
   * Parses a message asynchronously from an InputStream.
   *
   * @param <T> the message type
   * @param input the input stream to parse from
   * @param parser the parser for the message type
   * @return a CompletableFuture that will contain the parsed message
   */
  public <T extends Message> CompletableFuture<T> parseAsync(
      InputStream input, Parser<T> parser) {
    return CompletableFuture.supplyAsync(() -> {
      try {
        return parser.parseFrom(input);
      } catch (IOException e) {
        throw new CompletionException(e);
      }
    }, executor);
  }

  /**
   * Parses a message asynchronously from a byte array.
   *
   * @param <T> the message type
   * @param data the byte array to parse
   * @param parser the parser for the message type
   * @return a CompletableFuture that will contain the parsed message
   */
  public <T extends Message> CompletableFuture<T> parseAsync(
      byte[] data, Parser<T> parser) {
    return CompletableFuture.supplyAsync(() -> {
      try {
        return parser.parseFrom(data);
      } catch (InvalidProtocolBufferException e) {
        throw new CompletionException(e);
      }
    }, executor);
  }

  /**
   * Parses a message asynchronously from a ByteBuffer.
   *
   * @param <T> the message type
   * @param buffer the ByteBuffer to parse
   * @param parser the parser for the message type
   * @return a CompletableFuture that will contain the parsed message
   */
  public <T extends Message> CompletableFuture<T> parseAsync(
      ByteBuffer buffer, Parser<T> parser) {
    return CompletableFuture.supplyAsync(() -> {
      try {
        return parser.parseFrom(CodedInputStream.newInstance(buffer));
      } catch (IOException e) {
        throw new CompletionException(e);
      }
    }, executor);
  }

  /**
   * Parses a message asynchronously from a ByteString.
   *
   * @param <T> the message type
   * @param data the ByteString to parse
   * @param parser the parser for the message type
   * @return a CompletableFuture that will contain the parsed message
   */
  public <T extends Message> CompletableFuture<T> parseAsync(
      ByteString data, Parser<T> parser) {
    return CompletableFuture.supplyAsync(() -> {
      try {
        return parser.parseFrom(data);
      } catch (InvalidProtocolBufferException e) {
        throw new CompletionException(e);
      }
    }, executor);
  }

  /**
   * Parses a delimited message asynchronously from an InputStream.
   * The message is expected to be preceded by its size as a varint.
   *
   * @param <T> the message type
   * @param input the input stream to parse from
   * @param parser the parser for the message type
   * @return a CompletableFuture that will contain the parsed message
   */
  public <T extends Message> CompletableFuture<T> parseDelimitedAsync(
      InputStream input, Parser<T> parser) {
    return CompletableFuture.supplyAsync(() -> {
      try {
        return parser.parseDelimitedFrom(input);
      } catch (IOException e) {
        throw new CompletionException(e);
      }
    }, executor);
  }

  /**
   * Serializes a message asynchronously to a byte array.
   *
   * @param message the message to serialize
   * @return a CompletableFuture that will contain the serialized bytes
   */
  public CompletableFuture<byte[]> serializeAsync(Message message) {
    return CompletableFuture.supplyAsync(message::toByteArray, executor);
  }

  /**
   * Serializes a message asynchronously to a ByteString.
   *
   * @param message the message to serialize
   * @return a CompletableFuture that will contain the serialized ByteString
   */
  public CompletableFuture<ByteString> serializeToByteStringAsync(Message message) {
    return CompletableFuture.supplyAsync(message::toByteString, executor);
  }

  /**
   * Parses multiple delimited messages from a stream asynchronously,
   * invoking a callback for each message.
   *
   * @param <T> the message type
   * @param input the input stream to parse from
   * @param parser the parser for the message type
   * @param messageHandler callback invoked for each parsed message
   * @return a CompletableFuture that completes when all messages are parsed
   */
  public <T extends Message> CompletableFuture<Integer> parseStreamAsync(
      InputStream input, Parser<T> parser, Consumer<T> messageHandler) {
    return CompletableFuture.supplyAsync(() -> {
      int count = 0;
      try {
        T message;
        while ((message = parser.parseDelimitedFrom(input)) != null) {
          messageHandler.accept(message);
          count++;
        }
      } catch (IOException e) {
        throw new CompletionException(e);
      }
      return count;
    }, executor);
  }

  /**
   * Gets the executor used by this parser.
   *
   * @return the executor
   */
  public Executor getExecutor() {
    return executor;
  }
}
