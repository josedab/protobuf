// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

package com.google.protobuf;

import java.util.List;

/**
 * Exception thrown when Protocol Buffer message validation fails.
 *
 * <p>This exception is thrown by the {@code validateOrThrow()} method when
 * validation rules are not satisfied. The exception contains a
 * {@link ValidationResult} with detailed information about all validation
 * errors.
 *
 * <p>Example usage:
 * <pre>{@code
 * try {
 *     message.validateOrThrow();
 *     // Process valid message
 * } catch (ValidationException e) {
 *     System.err.println("Validation failed: " + e.getMessage());
 *     for (ValidationError error : e.getErrors()) {
 *         System.err.println("  " + error.getField() + ": " + error.getMessage());
 *     }
 * }
 * }</pre>
 */
public class ValidationException extends Exception {

  private static final long serialVersionUID = 1L;

  private final ValidationResult result;

  /**
   * Constructs a ValidationException with the given validation result.
   *
   * @param result the validation result containing errors
   */
  public ValidationException(ValidationResult result) {
    super(result.toString());
    this.result = result;
  }

  /**
   * Constructs a ValidationException with a message and validation result.
   *
   * @param message the error message
   * @param result the validation result containing errors
   */
  public ValidationException(String message, ValidationResult result) {
    super(message);
    this.result = result;
  }

  /**
   * Returns the validation result containing all errors.
   *
   * @return the ValidationResult
   */
  public ValidationResult getResult() {
    return result;
  }

  /**
   * Returns the list of validation errors.
   *
   * <p>This is a convenience method equivalent to
   * {@code getResult().getErrorsList()}.
   *
   * @return the list of ValidationError instances
   */
  public List<ValidationError> getErrors() {
    return result.getErrorsList();
  }

  /**
   * Returns the number of validation errors.
   *
   * @return the error count
   */
  public int getErrorCount() {
    return result.getErrorsCount();
  }
}
