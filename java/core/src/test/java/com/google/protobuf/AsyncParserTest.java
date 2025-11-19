// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

package com.google.protobuf;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicInteger;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import protobuf_unittest.UnittestProto.TestAllTypes;

/** Tests for {@link AsyncParser}. */
@RunWith(JUnit4.class)
public class AsyncParserTest {

  @Test
  public void testParseAsyncFromBytes() throws Exception {
    TestAllTypes message = TestAllTypes.newBuilder()
        .setOptionalInt32(42)
        .setOptionalString("async")
        .build();

    byte[] data = message.toByteArray();

    AsyncParser asyncParser = new AsyncParser();
    CompletableFuture<TestAllTypes> future =
        asyncParser.parseAsync(data, TestAllTypes.parser());

    TestAllTypes parsed = future.get(5, TimeUnit.SECONDS);
    assertNotNull(parsed);
    assertEquals(42, parsed.getOptionalInt32());
    assertEquals("async", parsed.getOptionalString());
  }

  @Test
  public void testParseAsyncFromInputStream() throws Exception {
    TestAllTypes message = TestAllTypes.newBuilder()
        .setOptionalInt32(123)
        .build();

    ByteArrayInputStream input = new ByteArrayInputStream(message.toByteArray());

    AsyncParser asyncParser = new AsyncParser();
    CompletableFuture<TestAllTypes> future =
        asyncParser.parseAsync(input, TestAllTypes.parser());

    TestAllTypes parsed = future.get(5, TimeUnit.SECONDS);
    assertNotNull(parsed);
    assertEquals(123, parsed.getOptionalInt32());
  }

  @Test
  public void testParseAsyncFromByteString() throws Exception {
    TestAllTypes message = TestAllTypes.newBuilder()
        .setOptionalInt32(456)
        .build();

    ByteString data = message.toByteString();

    AsyncParser asyncParser = new AsyncParser();
    CompletableFuture<TestAllTypes> future =
        asyncParser.parseAsync(data, TestAllTypes.parser());

    TestAllTypes parsed = future.get(5, TimeUnit.SECONDS);
    assertNotNull(parsed);
    assertEquals(456, parsed.getOptionalInt32());
  }

  @Test
  public void testSerializeAsync() throws Exception {
    TestAllTypes message = TestAllTypes.newBuilder()
        .setOptionalInt32(789)
        .build();

    AsyncParser asyncParser = new AsyncParser();
    CompletableFuture<byte[]> future = asyncParser.serializeAsync(message);

    byte[] data = future.get(5, TimeUnit.SECONDS);
    assertNotNull(data);

    TestAllTypes parsed = TestAllTypes.parseFrom(data);
    assertEquals(789, parsed.getOptionalInt32());
  }

  @Test
  public void testParseStreamAsync() throws Exception {
    // Create multiple delimited messages
    ByteArrayOutputStream output = new ByteArrayOutputStream();
    for (int i = 0; i < 5; i++) {
      TestAllTypes message = TestAllTypes.newBuilder()
          .setOptionalInt32(i * 10)
          .build();
      message.writeDelimitedTo(output);
    }

    ByteArrayInputStream input = new ByteArrayInputStream(output.toByteArray());

    List<TestAllTypes> messages = new ArrayList<>();
    AsyncParser asyncParser = new AsyncParser();
    CompletableFuture<Integer> future = asyncParser.parseStreamAsync(
        input, TestAllTypes.parser(), messages::add);

    int count = future.get(5, TimeUnit.SECONDS);
    assertEquals(5, count);
    assertEquals(5, messages.size());

    for (int i = 0; i < 5; i++) {
      assertEquals(i * 10, messages.get(i).getOptionalInt32());
    }
  }

  @Test
  public void testParseDelimitedAsync() throws Exception {
    TestAllTypes message = TestAllTypes.newBuilder()
        .setOptionalInt32(100)
        .build();

    ByteArrayOutputStream output = new ByteArrayOutputStream();
    message.writeDelimitedTo(output);

    ByteArrayInputStream input = new ByteArrayInputStream(output.toByteArray());

    AsyncParser asyncParser = new AsyncParser();
    CompletableFuture<TestAllTypes> future =
        asyncParser.parseDelimitedAsync(input, TestAllTypes.parser());

    TestAllTypes parsed = future.get(5, TimeUnit.SECONDS);
    assertNotNull(parsed);
    assertEquals(100, parsed.getOptionalInt32());
  }
}

/** Tests for {@link IncrementalParser}. */
@RunWith(JUnit4.class)
class IncrementalParserTest {

  @Test
  public void testParseSingleMessage() throws Exception {
    TestAllTypes message = TestAllTypes.newBuilder()
        .setOptionalInt32(42)
        .setOptionalString("hello")
        .build();

    byte[] data = createDelimitedMessage(message);

    IncrementalParser<TestAllTypes> parser =
        new IncrementalParser<>(TestAllTypes.parser());
    IncrementalParser.State state = parser.feed(data);

    assertEquals(IncrementalParser.State.MESSAGE_READY, state);
    assertTrue(parser.hasMessage());

    TestAllTypes parsed = parser.takeMessage();
    assertNotNull(parsed);
    assertEquals(42, parsed.getOptionalInt32());
    assertEquals("hello", parsed.getOptionalString());
  }

  @Test
  public void testParseInChunks() throws Exception {
    TestAllTypes message = TestAllTypes.newBuilder()
        .setOptionalInt32(123)
        .setOptionalString("chunked")
        .build();

    byte[] data = createDelimitedMessage(message);

    IncrementalParser<TestAllTypes> parser =
        new IncrementalParser<>(TestAllTypes.parser());

    // Feed one byte at a time
    for (int i = 0; i < data.length - 1; i++) {
      IncrementalParser.State state = parser.feed(data, i, 1);
      assertEquals(IncrementalParser.State.NEED_MORE_DATA, state);
      assertFalse(parser.hasMessage());
    }

    // Feed the last byte
    IncrementalParser.State state = parser.feed(data, data.length - 1, 1);
    assertEquals(IncrementalParser.State.MESSAGE_READY, state);

    TestAllTypes parsed = parser.takeMessage();
    assertNotNull(parsed);
    assertEquals(123, parsed.getOptionalInt32());
    assertEquals("chunked", parsed.getOptionalString());
  }

  @Test
  public void testParseMultipleMessages() throws Exception {
    ByteArrayOutputStream output = new ByteArrayOutputStream();

    for (int i = 0; i < 5; i++) {
      TestAllTypes message = TestAllTypes.newBuilder()
          .setOptionalInt32(i * 10)
          .build();
      output.write(createDelimitedMessage(message));
    }

    byte[] data = output.toByteArray();

    IncrementalParser<TestAllTypes> parser =
        new IncrementalParser<>(TestAllTypes.parser());

    // Feed all data at once
    parser.feed(data);

    // Parse all messages
    for (int i = 0; i < 5; i++) {
      assertTrue(parser.hasMessage());
      TestAllTypes parsed = parser.takeMessage();
      assertNotNull(parsed);
      assertEquals(i * 10, parsed.getOptionalInt32());
    }

    assertFalse(parser.hasMessage());
  }

  @Test
  public void testMaxMessageSizeEnforced() throws Exception {
    IncrementalParser<TestAllTypes> parser =
        new IncrementalParser<>(TestAllTypes.parser(), 100);

    // Create a message larger than the limit
    StringBuilder largeString = new StringBuilder();
    for (int i = 0; i < 200; i++) {
      largeString.append('x');
    }

    TestAllTypes message = TestAllTypes.newBuilder()
        .setOptionalString(largeString.toString())
        .build();

    byte[] data = createDelimitedMessage(message);

    IncrementalParser.State state = parser.feed(data);
    assertEquals(IncrementalParser.State.ERROR, state);
    assertNotNull(parser.getErrorMessage());
  }

  @Test
  public void testReset() throws Exception {
    TestAllTypes message = TestAllTypes.newBuilder()
        .setOptionalInt32(42)
        .build();

    byte[] data = createDelimitedMessage(message);

    IncrementalParser<TestAllTypes> parser =
        new IncrementalParser<>(TestAllTypes.parser());

    // Parse partial data
    parser.feed(data, 0, data.length / 2);

    // Reset
    parser.reset();
    assertEquals(IncrementalParser.State.NEED_MORE_DATA, parser.getState());
    assertEquals(0, parser.getBytesConsumed());

    // Parse complete message
    parser.feed(data);
    assertTrue(parser.hasMessage());
  }

  @Test
  public void testTakeMessageWhenNotReady() throws Exception {
    IncrementalParser<TestAllTypes> parser =
        new IncrementalParser<>(TestAllTypes.parser());

    assertNull(parser.takeMessage());
  }

  private byte[] createDelimitedMessage(TestAllTypes message) throws IOException {
    ByteArrayOutputStream output = new ByteArrayOutputStream();
    message.writeDelimitedTo(output);
    return output.toByteArray();
  }
}
