"""Deprecation analysis tools for Protocol Buffers.

This package provides tools for analyzing and reporting on deprecated
protobuf fields with enhanced metadata support.

Tools:
- deprecation_reporter: Generate reports of all deprecated fields
- usage_analyzer: Find usage of deprecated fields in code
"""

from .deprecation_reporter import (
    DeprecatedField,
    find_deprecated_fields,
    generate_report,
)
from .usage_analyzer import (
    DeprecatedFieldUsage,
    DeprecationUsageAnalyzer,
    generate_usage_report,
)

__all__ = [
    'DeprecatedField',
    'find_deprecated_fields',
    'generate_report',
    'DeprecatedFieldUsage',
    'DeprecationUsageAnalyzer',
    'generate_usage_report',
]
