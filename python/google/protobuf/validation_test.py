# Protocol Buffers - Google's data interchange format
# Copyright 2008 Google Inc.  All rights reserved.
#
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file or at
# https://developers.google.com/open-source/licenses/bsd

"""Tests for validation module."""

import unittest
from google.protobuf import validation


class ValidationErrorTest(unittest.TestCase):
    """Tests for ValidationError class."""

    def test_create_error(self):
        error = validation.ValidationError(
            field="email",
            message="must be valid email",
            rule="email"
        )
        self.assertEqual(error.field, "email")
        self.assertEqual(error.message, "must be valid email")
        self.assertEqual(error.rule, "email")

    def test_error_str(self):
        error = validation.ValidationError(
            field="username",
            message="too short"
        )
        self.assertEqual(str(error), "username: too short")

    def test_error_str_no_field(self):
        error = validation.ValidationError(message="general error")
        self.assertEqual(str(error), "general error")


class ValidationResultTest(unittest.TestCase):
    """Tests for ValidationResult class."""

    def test_valid_result(self):
        result = validation.ValidationResult()
        self.assertTrue(result.is_valid)
        self.assertEqual(len(result.errors), 0)
        self.assertTrue(bool(result))

    def test_invalid_result(self):
        errors = [
            validation.ValidationError(field="email", message="invalid"),
            validation.ValidationError(field="age", message="too low")
        ]
        result = validation.ValidationResult(errors=errors)
        self.assertFalse(result.is_valid)
        self.assertEqual(len(result.errors), 2)
        self.assertFalse(bool(result))

    def test_result_str(self):
        result = validation.ValidationResult()
        self.assertEqual(str(result), "Validation passed")

        errors = [validation.ValidationError(field="email", message="invalid")]
        result = validation.ValidationResult(errors=errors)
        self.assertIn("Validation failed", str(result))


class ValidationExceptionTest(unittest.TestCase):
    """Tests for ValidationException class."""

    def test_exception(self):
        errors = [validation.ValidationError(field="test", message="error")]
        result = validation.ValidationResult(errors=errors)
        exc = validation.ValidationException(result)

        self.assertEqual(exc.result, result)
        self.assertEqual(exc.errors, errors)

    def test_raise_exception(self):
        errors = [validation.ValidationError(field="test", message="error")]
        result = validation.ValidationResult(errors=errors)

        with self.assertRaises(validation.ValidationException):
            raise validation.ValidationException(result)


class EmailValidationTest(unittest.TestCase):
    """Tests for email validation."""

    def test_valid_emails(self):
        valid_emails = [
            "test@example.com",
            "user.name@domain.co.uk",
            "user+tag@example.org",
            "user123@test.io",
        ]
        for email in valid_emails:
            self.assertTrue(
                validation.is_valid_email(email),
                f"Expected {email} to be valid"
            )

    def test_invalid_emails(self):
        invalid_emails = [
            "",
            "invalid",
            "@example.com",
            "user@",
            "user@domain",
            "user@@example.com",
        ]
        for email in invalid_emails:
            self.assertFalse(
                validation.is_valid_email(email),
                f"Expected {email} to be invalid"
            )


class URIValidationTest(unittest.TestCase):
    """Tests for URI validation."""

    def test_valid_uris(self):
        valid_uris = [
            "http://example.com",
            "https://example.com/path",
            "ftp://files.example.com",
            "custom://host",
        ]
        for uri in valid_uris:
            self.assertTrue(
                validation.is_valid_uri(uri),
                f"Expected {uri} to be valid"
            )

    def test_invalid_uris(self):
        invalid_uris = [
            "",
            "example.com",
            "://example.com",
            "1http://example.com",
        ]
        for uri in invalid_uris:
            self.assertFalse(
                validation.is_valid_uri(uri),
                f"Expected {uri} to be invalid"
            )


class UUIDValidationTest(unittest.TestCase):
    """Tests for UUID validation."""

    def test_valid_uuids(self):
        valid_uuids = [
            "550e8400-e29b-41d4-a716-446655440000",
            "00000000-0000-0000-0000-000000000000",
            "ffffffff-ffff-ffff-ffff-ffffffffffff",
        ]
        for uuid in valid_uuids:
            self.assertTrue(
                validation.is_valid_uuid(uuid),
                f"Expected {uuid} to be valid"
            )

    def test_invalid_uuids(self):
        invalid_uuids = [
            "",
            "550e8400-e29b-41d4-a716-44665544000",  # Too short
            "550e8400-e29b-41d4-a716-4466554400000",  # Too long
            "550e8400e29b41d4a716446655440000",  # No dashes
            "550e8400-e29b-41d4-a716-44665544000g",  # Invalid char
        ]
        for uuid in invalid_uuids:
            self.assertFalse(
                validation.is_valid_uuid(uuid),
                f"Expected {uuid} to be invalid"
            )


class HostnameValidationTest(unittest.TestCase):
    """Tests for hostname validation."""

    def test_valid_hostnames(self):
        valid_hostnames = [
            "example.com",
            "sub.domain.example.com",
            "localhost",
            "host123",
        ]
        for hostname in valid_hostnames:
            self.assertTrue(
                validation.is_valid_hostname(hostname),
                f"Expected {hostname} to be valid"
            )

    def test_invalid_hostnames(self):
        invalid_hostnames = [
            "",
            "-invalid.com",
            "invalid-.com",
            ".invalid.com",
        ]
        for hostname in invalid_hostnames:
            self.assertFalse(
                validation.is_valid_hostname(hostname),
                f"Expected {hostname} to be invalid"
            )


class IPValidationTest(unittest.TestCase):
    """Tests for IP address validation."""

    def test_valid_ipv4(self):
        valid_ips = [
            "192.168.1.1",
            "0.0.0.0",
            "255.255.255.255",
            "10.0.0.1",
        ]
        for ip in valid_ips:
            self.assertTrue(
                validation.is_valid_ipv4(ip),
                f"Expected {ip} to be valid IPv4"
            )

    def test_invalid_ipv4(self):
        invalid_ips = [
            "",
            "192.168.1",
            "192.168.1.1.1",
            "256.1.1.1",
            "192.168.1.a",
        ]
        for ip in invalid_ips:
            self.assertFalse(
                validation.is_valid_ipv4(ip),
                f"Expected {ip} to be invalid IPv4"
            )

    def test_valid_ip(self):
        self.assertTrue(validation.is_valid_ip("192.168.1.1"))
        self.assertTrue(validation.is_valid_ip("::1"))

    def test_invalid_ip(self):
        self.assertFalse(validation.is_valid_ip(""))
        self.assertFalse(validation.is_valid_ip("invalid"))


class PatternValidationTest(unittest.TestCase):
    """Tests for pattern matching."""

    def test_matches_pattern(self):
        self.assertTrue(validation.matches_pattern("abc123", "^[a-z0-9]+$"))
        self.assertTrue(validation.matches_pattern("test", "^test$"))
        self.assertFalse(validation.matches_pattern("ABC", "^[a-z]+$"))

    def test_invalid_pattern(self):
        self.assertFalse(validation.matches_pattern("test", "[invalid"))


class StringLengthValidationTest(unittest.TestCase):
    """Tests for string length validation."""

    def test_valid_length(self):
        self.assertIsNone(validation.validate_string_length("test", 1, 10))
        self.assertIsNone(validation.validate_string_length("", 0, None))
        self.assertIsNone(validation.validate_string_length("exactly10!", 10, 10))

    def test_invalid_length(self):
        self.assertIsNotNone(validation.validate_string_length("", 1, None))
        self.assertIsNotNone(validation.validate_string_length("toolong", None, 5))


class NumericRangeValidationTest(unittest.TestCase):
    """Tests for numeric range validation."""

    def test_valid_range(self):
        self.assertIsNone(validation.validate_numeric_range(5, gte=0, lte=10))
        self.assertIsNone(validation.validate_numeric_range(0, gte=0))
        self.assertIsNone(validation.validate_numeric_range(10, lte=10))

    def test_invalid_range(self):
        self.assertIsNotNone(validation.validate_numeric_range(-1, gte=0))
        self.assertIsNotNone(validation.validate_numeric_range(11, lte=10))
        self.assertIsNotNone(validation.validate_numeric_range(5, gt=5))
        self.assertIsNotNone(validation.validate_numeric_range(5, lt=5))


class InListValidationTest(unittest.TestCase):
    """Tests for in/not_in list validation."""

    def test_validate_in_list(self):
        self.assertIsNone(validation.validate_in_list(1, [1, 2, 3]))
        self.assertIsNotNone(validation.validate_in_list(4, [1, 2, 3]))

    def test_validate_not_in_list(self):
        self.assertIsNone(validation.validate_not_in_list(4, [1, 2, 3]))
        self.assertIsNotNone(validation.validate_not_in_list(1, [1, 2, 3]))


class RepeatedItemsValidationTest(unittest.TestCase):
    """Tests for repeated items validation."""

    def test_valid_count(self):
        self.assertIsNone(validation.validate_repeated_items([1, 2, 3], 1, 5))
        self.assertIsNone(validation.validate_repeated_items([1, 2, 3], 3, 3))

    def test_invalid_count(self):
        self.assertIsNotNone(validation.validate_repeated_items([1, 2, 3], 5))
        self.assertIsNotNone(validation.validate_repeated_items([1, 2, 3], max_items=2))

    def test_unique(self):
        self.assertIsNone(validation.validate_repeated_items([1, 2, 3], unique=True))
        self.assertIsNotNone(validation.validate_repeated_items([1, 2, 2], unique=True))


if __name__ == '__main__':
    unittest.main()
