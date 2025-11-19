#!/usr/bin/env python3
"""Unit tests for deprecation analysis tools.

These tests verify the functionality of the deprecation reporter and
usage analyzer tools.
"""

import os
import tempfile
import unittest
from unittest.mock import patch

from google.protobuf import descriptor_pb2

from deprecation_reporter import (
    DeprecatedField,
    find_deprecated_fields,
    generate_report,
    extract_deprecation_info,
)
from usage_analyzer import (
    DeprecatedFieldUsage,
    DeprecationUsageAnalyzer,
    generate_usage_report,
)


class TestDeprecatedField(unittest.TestCase):
    """Tests for the DeprecatedField class."""

    def test_initialization(self):
        """Test DeprecatedField initialization."""
        field = DeprecatedField("TestMessage", "old_field", 1)
        self.assertEqual(field.message_name, "TestMessage")
        self.assertEqual(field.field_name, "old_field")
        self.assertEqual(field.field_number, 1)
        self.assertEqual(field.full_name, "TestMessage.old_field")
        self.assertIsNone(field.replacement)
        self.assertIsNone(field.removal_version)
        self.assertFalse(field.breaking)

    def test_enhanced_metadata(self):
        """Test DeprecatedField with enhanced metadata."""
        field = DeprecatedField("User", "legacy_id", 2)
        field.replacement = "uuid_id"
        field.removal_version = "v3.0"
        field.since_version = "v2.0"
        field.migration_guide = "Use uuid_id instead"
        field.breaking = True

        self.assertEqual(field.replacement, "uuid_id")
        self.assertEqual(field.removal_version, "v3.0")
        self.assertEqual(field.since_version, "v2.0")
        self.assertEqual(field.migration_guide, "Use uuid_id instead")
        self.assertTrue(field.breaking)


class TestFindDeprecatedFields(unittest.TestCase):
    """Tests for finding deprecated fields in descriptor sets."""

    def test_find_simple_deprecated_field(self):
        """Test finding a simple deprecated field."""
        # Create a test FileDescriptorSet
        fds = descriptor_pb2.FileDescriptorSet()
        file_proto = fds.file.add()
        file_proto.name = "test.proto"
        file_proto.package = "test"

        msg = file_proto.message_type.add()
        msg.name = "TestMessage"

        field = msg.field.add()
        field.name = "deprecated_field"
        field.number = 1
        field.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
        field.options.deprecated = True

        fields = find_deprecated_fields(fds)
        self.assertEqual(len(fields), 1)
        self.assertEqual(fields[0].field_name, "deprecated_field")
        self.assertEqual(fields[0].full_name, "test.TestMessage.deprecated_field")

    def test_find_multiple_deprecated_fields(self):
        """Test finding multiple deprecated fields."""
        fds = descriptor_pb2.FileDescriptorSet()
        file_proto = fds.file.add()
        file_proto.name = "test.proto"
        file_proto.package = "test"

        msg = file_proto.message_type.add()
        msg.name = "TestMessage"

        # Add deprecated field 1
        field1 = msg.field.add()
        field1.name = "old_field1"
        field1.number = 1
        field1.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
        field1.options.deprecated = True

        # Add normal field
        field2 = msg.field.add()
        field2.name = "normal_field"
        field2.number = 2
        field2.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

        # Add deprecated field 2
        field3 = msg.field.add()
        field3.name = "old_field2"
        field3.number = 3
        field3.type = descriptor_pb2.FieldDescriptorProto.TYPE_STRING
        field3.options.deprecated = True

        fields = find_deprecated_fields(fds)
        self.assertEqual(len(fields), 2)
        field_names = [f.field_name for f in fields]
        self.assertIn("old_field1", field_names)
        self.assertIn("old_field2", field_names)

    def test_no_deprecated_fields(self):
        """Test when there are no deprecated fields."""
        fds = descriptor_pb2.FileDescriptorSet()
        file_proto = fds.file.add()
        file_proto.name = "test.proto"

        msg = file_proto.message_type.add()
        msg.name = "TestMessage"

        field = msg.field.add()
        field.name = "normal_field"
        field.number = 1
        field.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32

        fields = find_deprecated_fields(fds)
        self.assertEqual(len(fields), 0)


class TestGenerateReport(unittest.TestCase):
    """Tests for report generation."""

    def test_empty_report(self):
        """Test report generation with no deprecated fields."""
        report = generate_report([])
        self.assertIn("No deprecated fields found", report)

    def test_simple_report(self):
        """Test report generation with deprecated fields."""
        field = DeprecatedField("User", "legacy_id", 1)
        field.replacement = "uuid_id"
        field.removal_version = "v3.0"
        field.breaking = True

        report = generate_report([field])
        self.assertIn("Deprecation Report", report)
        self.assertIn("User.legacy_id", report)
        self.assertIn("uuid_id", report)
        self.assertIn("v3.0", report)
        self.assertIn("Breaking Changes", report)

    def test_timeline_section(self):
        """Test that timeline section is generated."""
        field = DeprecatedField("Test", "old", 1)
        field.since_version = "v1.0"
        field.removal_version = "v2.0"

        report = generate_report([field])
        self.assertIn("Timeline", report)
        self.assertIn("v1.0", report)
        self.assertIn("v2.0", report)


class TestDeprecationUsageAnalyzer(unittest.TestCase):
    """Tests for the usage analyzer."""

    def setUp(self):
        """Set up test fixtures."""
        self.fds = descriptor_pb2.FileDescriptorSet()
        file_proto = self.fds.file.add()
        file_proto.name = "test.proto"
        file_proto.package = "test"

        msg = file_proto.message_type.add()
        msg.name = "TestMessage"

        field = msg.field.add()
        field.name = "deprecated_field"
        field.number = 1
        field.type = descriptor_pb2.FieldDescriptorProto.TYPE_INT32
        field.options.deprecated = True

    def test_analyzer_initialization(self):
        """Test analyzer initialization."""
        analyzer = DeprecationUsageAnalyzer(self.fds)
        self.assertEqual(len(analyzer.deprecated_fields), 1)
        self.assertIn("deprecated_field", analyzer.deprecated_fields)

    def test_analyze_cpp_file(self):
        """Test analyzing C++ file for deprecated field usage."""
        analyzer = DeprecationUsageAnalyzer(self.fds)

        # Create a temporary file with deprecated field usage
        with tempfile.NamedTemporaryFile(mode='w', suffix='.cc', delete=False) as f:
            f.write('int value = message.deprecated_field();\n')
            f.write('message.set_deprecated_field(42);\n')
            temp_file = f.name

        try:
            usages = analyzer.analyze_file(temp_file)
            self.assertGreater(len(usages), 0)
            self.assertEqual(usages[0].field_name, "deprecated_field")
        finally:
            os.unlink(temp_file)

    def test_analyze_java_file(self):
        """Test analyzing Java file for deprecated field usage."""
        analyzer = DeprecationUsageAnalyzer(self.fds)

        with tempfile.NamedTemporaryFile(mode='w', suffix='.java', delete=False) as f:
            f.write('int value = message.getDeprecatedField();\n')
            f.write('builder.setDeprecatedField(42);\n')
            temp_file = f.name

        try:
            usages = analyzer.analyze_file(temp_file)
            self.assertGreater(len(usages), 0)
        finally:
            os.unlink(temp_file)

    def test_analyze_python_file(self):
        """Test analyzing Python file for deprecated field usage."""
        analyzer = DeprecationUsageAnalyzer(self.fds)

        with tempfile.NamedTemporaryFile(mode='w', suffix='.py', delete=False) as f:
            f.write('value = message.deprecated_field\n')
            temp_file = f.name

        try:
            usages = analyzer.analyze_file(temp_file)
            self.assertGreater(len(usages), 0)
        finally:
            os.unlink(temp_file)

    def test_no_usage_in_clean_file(self):
        """Test that no usage is found in a file without deprecated fields."""
        analyzer = DeprecationUsageAnalyzer(self.fds)

        with tempfile.NamedTemporaryFile(mode='w', suffix='.cc', delete=False) as f:
            f.write('int value = message.normal_field();\n')
            temp_file = f.name

        try:
            usages = analyzer.analyze_file(temp_file)
            self.assertEqual(len(usages), 0)
        finally:
            os.unlink(temp_file)


class TestUsageReport(unittest.TestCase):
    """Tests for usage report generation."""

    def test_empty_usage_report(self):
        """Test report with no usages."""
        report = generate_usage_report([])
        self.assertIn("No deprecated field usage found", report)

    def test_usage_report_content(self):
        """Test usage report content."""
        usage = DeprecatedFieldUsage(
            "old_field",
            "Message.old_field",
            "/path/to/file.cc",
            42
        )
        usage.replacement = "new_field"

        report = generate_usage_report([usage])
        self.assertIn("Message.old_field", report)
        self.assertIn("/path/to/file.cc", report)
        self.assertIn("42", report)
        self.assertIn("new_field", report)


if __name__ == '__main__':
    unittest.main()
