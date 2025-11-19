// Protocol Buffers - Google's data interchange format
// Copyright 2008 Google Inc.  All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file or at
// https://developers.google.com/open-source/licenses/bsd

package com.google.protobuf;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Contains the results of validating a Protocol Buffer message.
 *
 * <p>A ValidationResult indicates whether validation passed and contains
 * a list of all validation errors found.
 *
 * <p>Example usage:
 * <pre>{@code
 * ValidationResult result = message.validate();
 * if (!result.getIsValid()) {
 *     for (ValidationError error : result.getErrorsList()) {
 *         System.out.println(error.getField() + ": " + error.getMessage());
 *     }
 * }
 * }</pre>
 */
public final class ValidationResult {

  private boolean isValid;
  private final List<ValidationError> errors;

  private ValidationResult(Builder builder) {
    this.isValid = builder.isValid;
    this.errors = Collections.unmodifiableList(new ArrayList<>(builder.errors));
  }

  /**
   * Returns whether validation passed.
   *
   * @return true if no validation errors were found
   */
  public boolean getIsValid() {
    return isValid;
  }

  /**
   * Returns the list of validation errors.
   *
   * @return an unmodifiable list of ValidationError instances
   */
  public List<ValidationError> getErrorsList() {
    return errors;
  }

  /**
   * Returns the number of validation errors.
   *
   * @return the error count
   */
  public int getErrorsCount() {
    return errors.size();
  }

  /**
   * Returns the error at the specified index.
   *
   * @param index the index of the error
   * @return the ValidationError at the specified index
   * @throws IndexOutOfBoundsException if index is out of range
   */
  public ValidationError getErrors(int index) {
    return errors.get(index);
  }

  @Override
  public String toString() {
    if (isValid) {
      return "Validation passed";
    }
    StringBuilder sb = new StringBuilder("Validation failed: ");
    for (int i = 0; i < errors.size(); i++) {
      if (i > 0) {
        sb.append("; ");
      }
      sb.append(errors.get(i).toString());
    }
    return sb.toString();
  }

  /**
   * Creates a new builder for ValidationResult.
   *
   * @return a new Builder instance
   */
  public static Builder newBuilder() {
    return new Builder();
  }

  /**
   * Builder for ValidationResult instances.
   */
  public static final class Builder {
    private boolean isValid = true;
    private final List<ValidationError> errors = new ArrayList<>();

    private Builder() {}

    /**
     * Sets whether validation passed.
     *
     * @param isValid true if validation passed
     * @return this builder
     */
    public Builder setIsValid(boolean isValid) {
      this.isValid = isValid;
      return this;
    }

    /**
     * Adds a validation error.
     *
     * <p>Note: Adding an error does not automatically set isValid to false.
     * You must call setIsValid(false) explicitly.
     *
     * @param error the validation error to add
     * @return this builder
     */
    public Builder addErrors(ValidationError error) {
      if (error != null) {
        this.errors.add(error);
        this.isValid = false;
      }
      return this;
    }

    /**
     * Adds all validation errors from another result.
     *
     * @param result the result to merge errors from
     * @return this builder
     */
    public Builder mergeFrom(ValidationResult result) {
      if (result != null) {
        for (ValidationError error : result.getErrorsList()) {
          addErrors(error);
        }
      }
      return this;
    }

    /**
     * Clears all validation errors.
     *
     * @return this builder
     */
    public Builder clearErrors() {
      this.errors.clear();
      this.isValid = true;
      return this;
    }

    /**
     * Builds a ValidationResult instance.
     *
     * @return a new ValidationResult
     */
    public ValidationResult build() {
      return new ValidationResult(this);
    }
  }
}
