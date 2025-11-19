#!/usr/bin/env python3
"""Deprecation Reporter Tool for Protocol Buffers.

This tool generates a comprehensive report of all deprecated fields in a proto
schema, including enhanced deprecation metadata such as replacement fields,
removal versions, and migration guides.

Usage:
    protoc-deprecation-report --schema=<proto_file> [--current-version=<version>]

Example:
    protoc-deprecation-report --schema=user.proto --current-version=v2.5
"""

import argparse
import sys
from typing import List, Dict, Any, Optional

try:
    from google.protobuf import descriptor_pb2
    from google.protobuf import descriptor_pool
    from google.protobuf.compiler import plugin_pb2
except ImportError:
    print("Error: google.protobuf package not found.", file=sys.stderr)
    print("Install with: pip install protobuf", file=sys.stderr)
    sys.exit(1)


# Extension field number for deprecation metadata
DEPRECATION_EXTENSION_NUMBER = 1001


class DeprecatedField:
    """Represents a deprecated field with its metadata."""

    def __init__(self, message_name: str, field_name: str, field_number: int):
        self.message_name = message_name
        self.field_name = field_name
        self.field_number = field_number
        self.full_name = f"{message_name}.{field_name}"

        # Enhanced deprecation metadata
        self.replacement: Optional[str] = None
        self.removal_version: Optional[str] = None
        self.since_version: Optional[str] = None
        self.migration_guide: Optional[str] = None
        self.breaking: bool = False
        self.documentation_url: Optional[str] = None
        self.tags: Dict[str, str] = {}


def extract_deprecation_info(field_options) -> Dict[str, Any]:
    """Extract enhanced deprecation info from field options.

    Args:
        field_options: The FieldOptions containing potential deprecation extension.

    Returns:
        Dictionary with deprecation metadata fields.
    """
    info = {}

    # Try to find the deprecation extension
    for extension in field_options.ListFields():
        field_descriptor, value = extension
        if (field_descriptor.is_extension and
            field_descriptor.number == DEPRECATION_EXTENSION_NUMBER):
            # Extract fields from the DeprecationInfo message
            for sub_field in value.ListFields():
                sub_descriptor, sub_value = sub_field
                info[sub_descriptor.name] = sub_value

    return info


def find_deprecated_fields(descriptor_set: descriptor_pb2.FileDescriptorSet) -> List[DeprecatedField]:
    """Find all deprecated fields in the descriptor set.

    Args:
        descriptor_set: The FileDescriptorSet to analyze.

    Returns:
        List of DeprecatedField objects.
    """
    deprecated_fields = []

    for file_proto in descriptor_set.file:
        for message_type in file_proto.message_type:
            deprecated_fields.extend(
                _find_deprecated_in_message(message_type, file_proto.package)
            )

    return deprecated_fields


def _find_deprecated_in_message(message: descriptor_pb2.DescriptorProto,
                                 package: str) -> List[DeprecatedField]:
    """Recursively find deprecated fields in a message and its nested messages.

    Args:
        message: The DescriptorProto to analyze.
        package: The package name prefix.

    Returns:
        List of DeprecatedField objects.
    """
    deprecated_fields = []
    full_name = f"{package}.{message.name}" if package else message.name

    for field in message.field:
        if field.options.deprecated:
            dep_field = DeprecatedField(full_name, field.name, field.number)

            # Extract enhanced deprecation info
            info = extract_deprecation_info(field.options)
            dep_field.replacement = info.get('replacement')
            dep_field.removal_version = info.get('removal_version')
            dep_field.since_version = info.get('since_version')
            dep_field.migration_guide = info.get('migration_guide')
            dep_field.breaking = info.get('breaking', False)
            dep_field.documentation_url = info.get('documentation_url')
            dep_field.tags = dict(info.get('tags', {}))

            deprecated_fields.append(dep_field)

    # Check nested messages
    for nested in message.nested_type:
        deprecated_fields.extend(
            _find_deprecated_in_message(nested, full_name)
        )

    return deprecated_fields


def generate_report(deprecated_fields: List[DeprecatedField],
                   current_version: Optional[str] = None) -> str:
    """Generate a human-readable deprecation report.

    Args:
        deprecated_fields: List of deprecated fields to report on.
        current_version: Optional current version for timeline analysis.

    Returns:
        Formatted report string.
    """
    if not deprecated_fields:
        return "No deprecated fields found.\n"

    lines = []
    lines.append("=" * 60)
    lines.append("Deprecation Report")
    lines.append("=" * 60)
    lines.append("")

    if current_version:
        lines.append(f"Current Version: {current_version}")
        lines.append("")

    # Group by breaking vs non-breaking
    breaking = [f for f in deprecated_fields if f.breaking]
    non_breaking = [f for f in deprecated_fields if not f.breaking]

    if breaking:
        lines.append("Breaking Changes:")
        lines.append("-" * 40)
        for field in breaking:
            migration = f" -> {field.replacement}" if field.replacement else ""
            version = f" (removed in {field.removal_version})" if field.removal_version else ""
            lines.append(f"  - {field.full_name}{migration}{version}")
        lines.append("")

    if non_breaking:
        lines.append("Non-Breaking Deprecations:")
        lines.append("-" * 40)
        for field in non_breaking:
            migration = f" -> {field.replacement}" if field.replacement else ""
            version = f" (removed in {field.removal_version})" if field.removal_version else ""
            lines.append(f"  - {field.full_name}{migration}{version}")
        lines.append("")

    # Timeline section
    versioned_fields = [f for f in deprecated_fields if f.since_version or f.removal_version]
    if versioned_fields:
        lines.append("Timeline:")
        lines.append("-" * 40)
        events = []
        for field in versioned_fields:
            if field.since_version:
                events.append((field.since_version, "deprecated", field.full_name))
            if field.removal_version:
                suffix = " (BREAKING)" if field.breaking else ""
                events.append((field.removal_version, "removed", field.full_name + suffix))

        # Sort by version (simple string sort)
        events.sort(key=lambda x: x[0])
        for version, action, name in events:
            lines.append(f"  - {version}: {name} {action}")
        lines.append("")

    # Detailed field information
    lines.append("Detailed Information:")
    lines.append("-" * 40)
    for field in deprecated_fields:
        lines.append(f"\n{field.full_name}:")
        if field.since_version:
            lines.append(f"  Since: {field.since_version}")
        if field.removal_version:
            lines.append(f"  Removal: {field.removal_version}")
        if field.replacement:
            lines.append(f"  Replacement: {field.replacement}")
        if field.migration_guide:
            lines.append(f"  Migration: {field.migration_guide}")
        if field.documentation_url:
            lines.append(f"  Documentation: {field.documentation_url}")
        if field.breaking:
            lines.append("  Breaking: Yes")
        if field.tags:
            lines.append(f"  Tags: {field.tags}")

    lines.append("")
    lines.append("=" * 60)
    lines.append(f"Total deprecated fields: {len(deprecated_fields)}")
    lines.append(f"Breaking changes: {len(breaking)}")
    lines.append("=" * 60)

    return "\n".join(lines)


def main():
    """Main entry point for the deprecation reporter."""
    parser = argparse.ArgumentParser(
        description="Generate a report of deprecated fields in proto schemas."
    )
    parser.add_argument(
        "--schema",
        required=True,
        help="Path to the proto file or FileDescriptorSet binary"
    )
    parser.add_argument(
        "--current-version",
        help="Current version for timeline analysis (e.g., v2.5)"
    )
    parser.add_argument(
        "--format",
        choices=["text", "json"],
        default="text",
        help="Output format (default: text)"
    )

    args = parser.parse_args()

    # Read the descriptor set
    try:
        with open(args.schema, "rb") as f:
            descriptor_set = descriptor_pb2.FileDescriptorSet()
            descriptor_set.ParseFromString(f.read())
    except Exception as e:
        print(f"Error reading schema file: {e}", file=sys.stderr)
        print("\nNote: This tool expects a FileDescriptorSet binary.", file=sys.stderr)
        print("Generate one with: protoc --descriptor_set_out=schema.pb <proto_files>",
              file=sys.stderr)
        sys.exit(1)

    # Find deprecated fields
    deprecated_fields = find_deprecated_fields(descriptor_set)

    # Generate and print report
    if args.format == "text":
        report = generate_report(deprecated_fields, args.current_version)
        print(report)
    else:
        # JSON format
        import json
        data = {
            "current_version": args.current_version,
            "deprecated_fields": [
                {
                    "full_name": f.full_name,
                    "message": f.message_name,
                    "field": f.field_name,
                    "number": f.field_number,
                    "replacement": f.replacement,
                    "removal_version": f.removal_version,
                    "since_version": f.since_version,
                    "migration_guide": f.migration_guide,
                    "breaking": f.breaking,
                    "documentation_url": f.documentation_url,
                    "tags": f.tags,
                }
                for f in deprecated_fields
            ],
            "total": len(deprecated_fields),
            "breaking_count": sum(1 for f in deprecated_fields if f.breaking),
        }
        print(json.dumps(data, indent=2))


if __name__ == "__main__":
    main()
