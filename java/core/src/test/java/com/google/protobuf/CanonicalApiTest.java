// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

package com.google.protobuf;

import static org.junit.Assert.assertArrayEquals;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;

import protobuf_unittest.UnittestProto.TestAllTypes;
import java.io.ByteArrayOutputStream;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

/**
 * Tests for the canonical API aliases in MessageLite.
 */
@RunWith(JUnit4.class)
public class CanonicalApiTest {

  /**
   * Test that serialize() produces the same output as toByteArray().
   */
  @Test
  public void testSerializeEquivalence() throws Exception {
    TestAllTypes message = TestAllTypes.newBuilder()
        .setOptionalInt32(123)
        .setOptionalString("test")
        .build();

    byte[] canonical = message.serialize();
    byte[] original = message.toByteArray();

    assertArrayEquals(original, canonical);
    assertTrue(canonical.length > 0);
  }

  /**
   * Test that serializeTo() produces the same output as writeTo().
   */
  @Test
  public void testSerializeToEquivalence() throws Exception {
    TestAllTypes message = TestAllTypes.newBuilder()
        .setOptionalInt32(456)
        .setOptionalString("hello")
        .build();

    ByteArrayOutputStream canonicalStream = new ByteArrayOutputStream();
    message.serializeTo(canonicalStream);
    byte[] canonical = canonicalStream.toByteArray();

    ByteArrayOutputStream originalStream = new ByteArrayOutputStream();
    message.writeTo(originalStream);
    byte[] original = originalStream.toByteArray();

    assertArrayEquals(original, canonical);
    assertTrue(canonical.length > 0);
  }

  /**
   * Test that clone() creates a proper copy.
   */
  @Test
  public void testCloneEquivalence() throws Exception {
    TestAllTypes message = TestAllTypes.newBuilder()
        .setOptionalInt32(789)
        .setOptionalString("world")
        .addRepeatedInt32(1)
        .addRepeatedInt32(2)
        .build();

    MessageLite cloned = message.clone();

    // Verify it's a MessageLite
    assertTrue(cloned instanceof TestAllTypes);

    // Cast and verify contents
    TestAllTypes clonedMessage = (TestAllTypes) cloned;
    assertEquals(message.getOptionalInt32(), clonedMessage.getOptionalInt32());
    assertEquals(message.getOptionalString(), clonedMessage.getOptionalString());
    assertEquals(message.getRepeatedInt32Count(), clonedMessage.getRepeatedInt32Count());
    assertEquals(message.getRepeatedInt32(0), clonedMessage.getRepeatedInt32(0));
    assertEquals(message.getRepeatedInt32(1), clonedMessage.getRepeatedInt32(1));
  }

  /**
   * Test round-trip: serialize -> parse.
   */
  @Test
  public void testRoundTrip() throws Exception {
    TestAllTypes original = TestAllTypes.newBuilder()
        .setOptionalInt32(12345)
        .setOptionalInt64(67890L)
        .setOptionalFloat(3.14f)
        .setOptionalString("round trip test")
        .addRepeatedInt32(1)
        .addRepeatedInt32(2)
        .addRepeatedInt32(3)
        .build();

    byte[] data = original.serialize();
    TestAllTypes restored = TestAllTypes.parseFrom(data);

    assertEquals(original.getOptionalInt32(), restored.getOptionalInt32());
    assertEquals(original.getOptionalInt64(), restored.getOptionalInt64());
    assertEquals(original.getOptionalFloat(), restored.getOptionalFloat(), 0.001f);
    assertEquals(original.getOptionalString(), restored.getOptionalString());
    assertEquals(original.getRepeatedInt32Count(), restored.getRepeatedInt32Count());
    assertEquals(original.getRepeatedInt32(0), restored.getRepeatedInt32(0));
    assertEquals(original.getRepeatedInt32(1), restored.getRepeatedInt32(1));
    assertEquals(original.getRepeatedInt32(2), restored.getRepeatedInt32(2));
  }

  /**
   * Test serialized size consistency.
   */
  @Test
  public void testSerializedSizeConsistency() throws Exception {
    TestAllTypes message = TestAllTypes.newBuilder()
        .setOptionalInt32(42)
        .setOptionalString("size test")
        .build();

    int size = message.getSerializedSize();
    byte[] serialized = message.serialize();

    assertEquals(size, serialized.length);
    assertTrue(size > 0);
  }

  /**
   * Test empty message.
   */
  @Test
  public void testEmptyMessage() throws Exception {
    TestAllTypes empty = TestAllTypes.getDefaultInstance();

    byte[] data = empty.serialize();
    assertEquals(0, data.length);

    TestAllTypes parsed = TestAllTypes.parseFrom(data);
    assertTrue(parsed.isInitialized());
    assertEquals(0, parsed.getSerializedSize());
  }

  /**
   * Test that canonical methods work through the MessageLite interface.
   */
  @Test
  public void testThroughMessageLiteInterface() throws Exception {
    MessageLite message = TestAllTypes.newBuilder()
        .setOptionalInt32(999)
        .build();

    byte[] data = message.serialize();
    assertTrue(data.length > 0);

    MessageLite cloned = message.clone();
    assertArrayEquals(message.serialize(), cloned.serialize());
  }
}
