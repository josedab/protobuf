#!/usr/bin/env python3
"""Usage Analyzer Tool for Protocol Buffers Deprecation.

This tool scans a codebase for usage of deprecated protobuf fields and
generates a report showing where deprecated fields are being used.

Usage:
    protoc-usage-analyzer --schema=<descriptor_set> --codebase=<path> [options]

Example:
    protoc-usage-analyzer --schema=schema.pb --codebase=./src --fail-on-deprecated
"""

import argparse
import os
import re
import sys
from typing import List, Dict, Set, Tuple, Optional
from collections import defaultdict

try:
    from google.protobuf import descriptor_pb2
except ImportError:
    print("Error: google.protobuf package not found.", file=sys.stderr)
    print("Install with: pip install protobuf", file=sys.stderr)
    sys.exit(1)


# Extension field number for deprecation metadata
DEPRECATION_EXTENSION_NUMBER = 1001


class DeprecatedFieldUsage:
    """Represents usage of a deprecated field in code."""

    def __init__(self, field_name: str, full_name: str, file_path: str, line_number: int):
        self.field_name = field_name
        self.full_name = full_name
        self.file_path = file_path
        self.line_number = line_number

        # Enhanced deprecation metadata
        self.replacement: Optional[str] = None
        self.since_version: Optional[str] = None
        self.removal_version: Optional[str] = None


class DeprecationUsageAnalyzer:
    """Analyzes codebase for deprecated protobuf field usage."""

    def __init__(self, descriptor_set: descriptor_pb2.FileDescriptorSet):
        self.descriptor_set = descriptor_set
        self.deprecated_fields: Dict[str, Dict] = {}
        self._extract_deprecated_fields()

    def _extract_deprecated_fields(self):
        """Extract all deprecated fields from the descriptor set."""
        for file_proto in self.descriptor_set.file:
            for message_type in file_proto.message_type:
                self._extract_from_message(message_type, file_proto.package)

    def _extract_from_message(self, message: descriptor_pb2.DescriptorProto, package: str):
        """Extract deprecated fields from a message."""
        full_name = f"{package}.{message.name}" if package else message.name

        for field in message.field:
            if field.options.deprecated:
                field_info = {
                    "message": full_name,
                    "field_name": field.name,
                    "full_name": f"{full_name}.{field.name}",
                    "replacement": None,
                    "since_version": None,
                    "removal_version": None,
                }

                # Extract enhanced deprecation info
                for extension in field.options.ListFields():
                    field_descriptor, value = extension
                    if (field_descriptor.is_extension and
                        field_descriptor.number == DEPRECATION_EXTENSION_NUMBER):
                        for sub_field in value.ListFields():
                            sub_descriptor, sub_value = sub_field
                            field_info[sub_descriptor.name] = sub_value

                self.deprecated_fields[field.name] = field_info

        # Check nested messages
        for nested in message.nested_type:
            self._extract_from_message(nested, full_name)

    def analyze_file(self, file_path: str) -> List[DeprecatedFieldUsage]:
        """Analyze a single file for deprecated field usage.

        Args:
            file_path: Path to the source file to analyze.

        Returns:
            List of DeprecatedFieldUsage objects found in the file.
        """
        usages = []

        try:
            with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
                lines = content.split('\n')
        except Exception as e:
            return usages

        # Build patterns for each deprecated field
        for field_name, field_info in self.deprecated_fields.items():
            # Common patterns for field access in various languages
            patterns = [
                # C++: message.field_name() or message.set_field_name()
                rf'\b{field_name}\s*\(',
                rf'\.{field_name}\s*\(',
                rf'set_{field_name}\s*\(',
                rf'has_{field_name}\s*\(',
                rf'clear_{field_name}\s*\(',
                rf'mutable_{field_name}\s*\(',

                # Java: getMessage().getFieldName() or setFieldName()
                rf'get{self._to_camel_case(field_name)}\s*\(',
                rf'set{self._to_camel_case(field_name)}\s*\(',
                rf'has{self._to_camel_case(field_name)}\s*\(',
                rf'clear{self._to_camel_case(field_name)}\s*\(',

                # Python: message.field_name
                rf'\.{field_name}\b(?!\s*\()',

                # Proto text format or JSON
                rf'"{field_name}"\s*:',
                rf'{field_name}\s*:',
            ]

            for i, line in enumerate(lines, start=1):
                for pattern in patterns:
                    if re.search(pattern, line):
                        usage = DeprecatedFieldUsage(
                            field_name,
                            field_info["full_name"],
                            file_path,
                            i
                        )
                        usage.replacement = field_info.get("replacement")
                        usage.since_version = field_info.get("since_version")
                        usage.removal_version = field_info.get("removal_version")
                        usages.append(usage)
                        break  # Only count once per line per field

        return usages

    def _to_camel_case(self, snake_str: str) -> str:
        """Convert snake_case to CamelCase."""
        components = snake_str.split('_')
        return ''.join(x.title() for x in components)

    def analyze_directory(self, directory: str, extensions: List[str] = None) -> List[DeprecatedFieldUsage]:
        """Analyze all files in a directory for deprecated field usage.

        Args:
            directory: Root directory to scan.
            extensions: List of file extensions to scan (e.g., ['.cc', '.java', '.py']).

        Returns:
            List of all DeprecatedFieldUsage objects found.
        """
        if extensions is None:
            extensions = ['.cc', '.cpp', '.h', '.hpp', '.java', '.py', '.go',
                         '.cs', '.rb', '.php', '.js', '.ts', '.proto']

        all_usages = []

        for root, dirs, files in os.walk(directory):
            # Skip common non-source directories
            dirs[:] = [d for d in dirs if d not in [
                '.git', 'node_modules', '__pycache__', 'build', 'dist',
                'target', '.idea', '.vscode'
            ]]

            for file in files:
                if any(file.endswith(ext) for ext in extensions):
                    file_path = os.path.join(root, file)
                    usages = self.analyze_file(file_path)
                    all_usages.extend(usages)

        return all_usages


def generate_usage_report(usages: List[DeprecatedFieldUsage]) -> str:
    """Generate a human-readable usage report.

    Args:
        usages: List of deprecated field usages to report.

    Returns:
        Formatted report string.
    """
    if not usages:
        return "No deprecated field usage found.\n"

    lines = []
    lines.append("=" * 60)
    lines.append("Deprecated Field Usage Report")
    lines.append("=" * 60)
    lines.append("")

    # Group by field
    by_field: Dict[str, List[DeprecatedFieldUsage]] = defaultdict(list)
    for usage in usages:
        by_field[usage.full_name].append(usage)

    for full_name, field_usages in sorted(by_field.items()):
        first = field_usages[0]
        version_info = ""
        if first.since_version:
            version_info = f" (deprecated {first.since_version})"

        lines.append(f"{full_name}{version_info}:")

        # Group by file
        by_file: Dict[str, List[int]] = defaultdict(list)
        for usage in field_usages:
            by_file[usage.file_path].append(usage.line_number)

        for file_path, line_numbers in sorted(by_file.items()):
            line_str = ", ".join(str(ln) for ln in sorted(set(line_numbers)))
            lines.append(f"  - {file_path}:{line_str}")

        if first.replacement:
            lines.append(f"  Suggested replacement: {first.replacement}")
        lines.append("")

    lines.append("=" * 60)
    lines.append(f"Total usages: {len(usages)}")
    lines.append(f"Unique deprecated fields: {len(by_field)}")
    lines.append("=" * 60)

    return "\n".join(lines)


def main():
    """Main entry point for the usage analyzer."""
    parser = argparse.ArgumentParser(
        description="Analyze codebase for deprecated protobuf field usage."
    )
    parser.add_argument(
        "--schema",
        required=True,
        help="Path to FileDescriptorSet binary"
    )
    parser.add_argument(
        "--codebase",
        required=True,
        help="Root directory of codebase to scan"
    )
    parser.add_argument(
        "--extensions",
        help="Comma-separated list of file extensions to scan (e.g., .cc,.java,.py)"
    )
    parser.add_argument(
        "--fail-on-deprecated",
        action="store_true",
        help="Exit with non-zero status if deprecated usage found"
    )
    parser.add_argument(
        "--format",
        choices=["text", "json"],
        default="text",
        help="Output format (default: text)"
    )

    args = parser.parse_args()

    # Validate codebase directory
    if not os.path.isdir(args.codebase):
        print(f"Error: Codebase directory not found: {args.codebase}", file=sys.stderr)
        sys.exit(1)

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

    # Create analyzer and scan codebase
    analyzer = DeprecationUsageAnalyzer(descriptor_set)

    extensions = None
    if args.extensions:
        extensions = [ext.strip() for ext in args.extensions.split(',')]

    usages = analyzer.analyze_directory(args.codebase, extensions)

    # Generate and print report
    if args.format == "text":
        report = generate_usage_report(usages)
        print(report)
    else:
        # JSON format
        import json
        data = {
            "usages": [
                {
                    "field_name": u.field_name,
                    "full_name": u.full_name,
                    "file": u.file_path,
                    "line": u.line_number,
                    "replacement": u.replacement,
                    "since_version": u.since_version,
                    "removal_version": u.removal_version,
                }
                for u in usages
            ],
            "total": len(usages),
            "unique_fields": len(set(u.full_name for u in usages)),
        }
        print(json.dumps(data, indent=2))

    # Exit with error if requested and deprecated usage found
    if args.fail_on_deprecated and usages:
        sys.exit(1)


if __name__ == "__main__":
    main()
