// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

package com.google.protobuf;

/**
 * Represents a single validation error for a Protocol Buffer message field.
 *
 * <p>A ValidationError contains information about which field failed validation
 * and why. Multiple ValidationError instances can be aggregated into a
 * {@link ValidationResult}.
 *
 * <p>Example usage:
 * <pre>{@code
 * ValidationError error = ValidationError.newBuilder()
 *     .setField("user.email")
 *     .setMessage("must be a valid email address")
 *     .setRule("email")
 *     .build();
 * }</pre>
 */
public final class ValidationError {

  private final String field;
  private final String message;
  private final String rule;

  private ValidationError(Builder builder) {
    this.field = builder.field;
    this.message = builder.message;
    this.rule = builder.rule;
  }

  /**
   * Returns the field path that failed validation.
   *
   * <p>For nested messages, this will be a dot-separated path like "user.email".
   *
   * @return the field path
   */
  public String getField() {
    return field;
  }

  /**
   * Returns the human-readable error message.
   *
   * @return the error message
   */
  public String getMessage() {
    return message;
  }

  /**
   * Returns the validation rule that failed.
   *
   * <p>This can be used to programmatically identify the type of validation
   * that failed (e.g., "min_len", "max_len", "email", "pattern").
   *
   * @return the rule name
   */
  public String getRule() {
    return rule;
  }

  /**
   * Returns a new builder for this error.
   *
   * @return a new Builder instance
   */
  public Builder toBuilder() {
    return new Builder()
        .setField(field)
        .setMessage(message)
        .setRule(rule);
  }

  @Override
  public String toString() {
    if (field != null && !field.isEmpty()) {
      return field + ": " + message;
    }
    return message;
  }

  @Override
  public boolean equals(Object obj) {
    if (this == obj) return true;
    if (!(obj instanceof ValidationError)) return false;
    ValidationError other = (ValidationError) obj;
    return Objects.equals(field, other.field)
        && Objects.equals(message, other.message)
        && Objects.equals(rule, other.rule);
  }

  @Override
  public int hashCode() {
    return Objects.hash(field, message, rule);
  }

  /**
   * Creates a new builder for ValidationError.
   *
   * @return a new Builder instance
   */
  public static Builder newBuilder() {
    return new Builder();
  }

  /**
   * Builder for ValidationError instances.
   */
  public static final class Builder {
    private String field = "";
    private String message = "";
    private String rule = "";

    private Builder() {}

    /**
     * Sets the field path that failed validation.
     *
     * @param field the field path
     * @return this builder
     */
    public Builder setField(String field) {
      this.field = field != null ? field : "";
      return this;
    }

    /**
     * Sets the error message.
     *
     * @param message the error message
     * @return this builder
     */
    public Builder setMessage(String message) {
      this.message = message != null ? message : "";
      return this;
    }

    /**
     * Sets the validation rule that failed.
     *
     * @param rule the rule name
     * @return this builder
     */
    public Builder setRule(String rule) {
      this.rule = rule != null ? rule : "";
      return this;
    }

    /**
     * Builds a ValidationError instance.
     *
     * @return a new ValidationError
     */
    public ValidationError build() {
      return new ValidationError(this);
    }
  }

  // Import for Objects.equals and hash
  private static class Objects {
    static boolean equals(Object a, Object b) {
      return (a == b) || (a != null && a.equals(b));
    }

    static int hash(Object... values) {
      int result = 1;
      for (Object value : values) {
        result = 31 * result + (value == null ? 0 : value.hashCode());
      }
      return result;
    }
  }
}
