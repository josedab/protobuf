# Protocol Buffers - Google's data interchange format
# Copyright 2008 Google Inc.  All rights reserved.
#
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file or at
# https://developers.google.com/open-source/licenses/bsd

"""Validation support for Protocol Buffer messages.

This module provides classes and utilities for validating Protocol Buffer
messages according to validation rules defined in .proto files.

Example usage:
    from google.protobuf import validation

    # Create a message
    user = User()
    user.email = "invalid-email"
    user.age = -5

    # Validate the message
    result = user.validate()
    if not result.is_valid:
        for error in result.errors:
            print(f"{error.field}: {error.message}")

    # Or use validate_or_raise
    try:
        user.validate_or_raise()
    except validation.ValidationException as e:
        print(f"Validation failed: {e}")
"""

import re
from typing import List, Optional


class ValidationError:
    """Represents a single validation error.

    Attributes:
        field: The field path that failed validation (e.g., "user.email").
        message: Human-readable error message.
        rule: The validation rule that failed (e.g., "min_len").
    """

    def __init__(self, field: str = "", message: str = "", rule: str = ""):
        """Initialize a ValidationError.

        Args:
            field: The field path that failed validation.
            message: Human-readable error message.
            rule: The validation rule that failed.
        """
        self.field = field
        self.message = message
        self.rule = rule

    def __str__(self) -> str:
        """Return string representation of the error."""
        if self.field:
            return f"{self.field}: {self.message}"
        return self.message

    def __repr__(self) -> str:
        """Return detailed representation of the error."""
        return f"ValidationError(field='{self.field}', message='{self.message}', rule='{self.rule}')"


class ValidationResult:
    """Contains the results of validating a message.

    Attributes:
        is_valid: Whether validation passed (True if no errors).
        errors: List of ValidationError objects.
    """

    def __init__(self, errors: Optional[List[ValidationError]] = None):
        """Initialize a ValidationResult.

        Args:
            errors: List of validation errors. If None or empty, is_valid is True.
        """
        self.errors = errors if errors else []
        self.is_valid = len(self.errors) == 0

    def __bool__(self) -> bool:
        """Return True if validation passed."""
        return self.is_valid

    def __str__(self) -> str:
        """Return string representation of the result."""
        if self.is_valid:
            return "Validation passed"
        error_msgs = [str(e) for e in self.errors]
        return f"Validation failed: {'; '.join(error_msgs)}"

    def __repr__(self) -> str:
        """Return detailed representation of the result."""
        return f"ValidationResult(is_valid={self.is_valid}, errors={self.errors})"


class ValidationException(Exception):
    """Exception raised when validation fails.

    Attributes:
        result: The ValidationResult containing the errors.
    """

    def __init__(self, result: ValidationResult):
        """Initialize a ValidationException.

        Args:
            result: The ValidationResult containing the errors.
        """
        self.result = result
        super().__init__(str(result))

    @property
    def errors(self) -> List[ValidationError]:
        """Return the list of validation errors."""
        return self.result.errors


# Validation helper functions

def is_valid_email(value: str) -> bool:
    """Check if a string is a valid email address.

    Uses a simplified email regex pattern that covers most common cases.

    Args:
        value: The string to validate.

    Returns:
        True if the string appears to be a valid email address.
    """
    if not value:
        return False
    # Simple but reasonable email pattern
    pattern = r'^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$'
    return bool(re.match(pattern, value))


def is_valid_uri(value: str) -> bool:
    """Check if a string is a valid URI.

    Args:
        value: The string to validate.

    Returns:
        True if the string appears to be a valid URI.
    """
    if not value:
        return False
    # Basic URI pattern
    pattern = r'^[a-zA-Z][a-zA-Z0-9+.-]*://[^\s]+$'
    return bool(re.match(pattern, value))


def is_valid_uuid(value: str) -> bool:
    """Check if a string is a valid UUID.

    Args:
        value: The string to validate.

    Returns:
        True if the string is a valid UUID.
    """
    if not value:
        return False
    pattern = r'^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$'
    return bool(re.match(pattern, value))


def is_valid_hostname(value: str) -> bool:
    """Check if a string is a valid hostname (RFC 1123).

    Args:
        value: The string to validate.

    Returns:
        True if the string is a valid hostname.
    """
    if not value or len(value) > 253:
        return False
    # Hostname pattern per RFC 1123
    pattern = r'^[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(\.[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$'
    return bool(re.match(pattern, value))


def is_valid_ip(value: str) -> bool:
    """Check if a string is a valid IP address (v4 or v6).

    Args:
        value: The string to validate.

    Returns:
        True if the string is a valid IP address.
    """
    return is_valid_ipv4(value) or is_valid_ipv6(value)


def is_valid_ipv4(value: str) -> bool:
    """Check if a string is a valid IPv4 address.

    Args:
        value: The string to validate.

    Returns:
        True if the string is a valid IPv4 address.
    """
    if not value:
        return False
    parts = value.split('.')
    if len(parts) != 4:
        return False
    try:
        return all(0 <= int(part) <= 255 for part in parts)
    except ValueError:
        return False


def is_valid_ipv6(value: str) -> bool:
    """Check if a string is a valid IPv6 address.

    Args:
        value: The string to validate.

    Returns:
        True if the string is a valid IPv6 address.
    """
    if not value:
        return False
    # Basic IPv6 pattern (simplified)
    pattern = r'^([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}$|^::$|^([0-9a-fA-F]{1,4}:)*:([0-9a-fA-F]{1,4}:)*[0-9a-fA-F]{1,4}$'
    return bool(re.match(pattern, value))


def matches_pattern(value: str, pattern: str) -> bool:
    """Check if a string matches a regex pattern.

    Args:
        value: The string to validate.
        pattern: The regex pattern to match against.

    Returns:
        True if the string matches the pattern.
    """
    try:
        return bool(re.match(pattern, value))
    except re.error:
        return False


def validate_string_length(value: str, min_len: Optional[int] = None,
                          max_len: Optional[int] = None) -> Optional[str]:
    """Validate string length constraints.

    Args:
        value: The string to validate.
        min_len: Minimum length (optional).
        max_len: Maximum length (optional).

    Returns:
        Error message if validation fails, None otherwise.
    """
    length = len(value)
    if min_len is not None and length < min_len:
        return f"length must be >= {min_len}, got {length}"
    if max_len is not None and length > max_len:
        return f"length must be <= {max_len}, got {length}"
    return None


def validate_numeric_range(value, gte=None, lte=None, gt=None, lt=None) -> Optional[str]:
    """Validate numeric range constraints.

    Args:
        value: The numeric value to validate.
        gte: Greater than or equal to (optional).
        lte: Less than or equal to (optional).
        gt: Greater than (optional).
        lt: Less than (optional).

    Returns:
        Error message if validation fails, None otherwise.
    """
    if gte is not None and value < gte:
        return f"must be >= {gte}, got {value}"
    if lte is not None and value > lte:
        return f"must be <= {lte}, got {value}"
    if gt is not None and value <= gt:
        return f"must be > {gt}, got {value}"
    if lt is not None and value >= lt:
        return f"must be < {lt}, got {value}"
    return None


def validate_in_list(value, allowed_values: list) -> Optional[str]:
    """Validate that a value is in a list of allowed values.

    Args:
        value: The value to validate.
        allowed_values: List of allowed values.

    Returns:
        Error message if validation fails, None otherwise.
    """
    if value not in allowed_values:
        return f"must be one of {allowed_values}, got {value}"
    return None


def validate_not_in_list(value, disallowed_values: list) -> Optional[str]:
    """Validate that a value is not in a list of disallowed values.

    Args:
        value: The value to validate.
        disallowed_values: List of disallowed values.

    Returns:
        Error message if validation fails, None otherwise.
    """
    if value in disallowed_values:
        return f"must not be one of {disallowed_values}, got {value}"
    return None


def validate_repeated_items(items: list, min_items: Optional[int] = None,
                           max_items: Optional[int] = None,
                           unique: bool = False) -> Optional[str]:
    """Validate repeated field constraints.

    Args:
        items: The list of items to validate.
        min_items: Minimum number of items (optional).
        max_items: Maximum number of items (optional).
        unique: Whether all items must be unique.

    Returns:
        Error message if validation fails, None otherwise.
    """
    count = len(items)
    if min_items is not None and count < min_items:
        return f"must have at least {min_items} items, got {count}"
    if max_items is not None and count > max_items:
        return f"must have at most {max_items} items, got {count}"
    if unique:
        # Check for duplicates (works for hashable items)
        try:
            if len(set(items)) != len(items):
                return "all items must be unique"
        except TypeError:
            # Items are not hashable, check manually
            seen = []
            for item in items:
                if item in seen:
                    return "all items must be unique"
                seen.append(item)
    return None
