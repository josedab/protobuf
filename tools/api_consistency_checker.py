#!/usr/bin/env python3
"""API Consistency Checker for Protocol Buffers.

This tool analyzes the Protocol Buffers codebase to measure and report
on API consistency across different language implementations.

Usage:
    python tools/api_consistency_checker.py [--verbose] [--json]
"""

import argparse
import json
import os
import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional, Set, Tuple

# Canonical API mapping: canonical_name -> {language: [actual_names]}
CANONICAL_API = {
    # Serialization
    'serialize': {
        'cpp': ['SerializeAsString', 'SerializeToString'],
        'java': ['toByteArray'],
        'python': ['SerializeToString'],
    },
    'serializeTo': {
        'cpp': ['SerializeToOstream', 'SerializeToCodedStream'],
        'java': ['writeTo'],
        'python': [],
    },
    'parse': {
        'cpp': ['ParseFromString', 'ParseFromArray'],
        'java': ['parseFrom'],
        'python': ['ParseFromString'],
    },
    'clear': {
        'cpp': ['Clear'],
        'java': ['clear'],  # Builder only
        'python': ['Clear'],
    },
    'isInitialized': {
        'cpp': ['IsInitialized'],
        'java': ['isInitialized'],
        'python': ['IsInitialized'],
    },
    'getSerializedSize': {
        'cpp': ['ByteSizeLong', 'ByteSize'],
        'java': ['getSerializedSize'],
        'python': ['ByteSize'],
    },

    # Field access (patterns)
    'getField': {
        'cpp': ['<field>()'],
        'java': ['get<Field>()'],
        'python': ['<field>'],
    },
    'setField': {
        'cpp': ['set_<field>()'],
        'java': ['set<Field>()'],  # Builder
        'python': ['<field> ='],
    },
    'hasField': {
        'cpp': ['has_<field>()'],
        'java': ['has<Field>()'],
        'python': ['HasField("<field>")'],
    },
    'clearField': {
        'cpp': ['clear_<field>()'],
        'java': ['clear<Field>()'],  # Builder
        'python': ['ClearField("<field>")'],
    },

    # Reflection
    'getDescriptor': {
        'cpp': ['GetDescriptor', 'descriptor'],
        'java': ['getDescriptor', 'getDescriptorForType'],
        'python': ['DESCRIPTOR'],
    },
    'getReflection': {
        'cpp': ['GetReflection'],
        'java': [],  # Built into Message interface
        'python': [],  # Different approach
    },

    # Builder operations
    'newBuilder': {
        'cpp': [],  # No builder pattern
        'java': ['newBuilder'],
        'python': [],  # No builder pattern
    },
    'toBuilder': {
        'cpp': [],  # No builder pattern
        'java': ['toBuilder'],
        'python': [],  # No builder pattern
    },
    'build': {
        'cpp': [],  # No builder pattern
        'java': ['build'],
        'python': [],  # No builder pattern
    },
    'mergeFrom': {
        'cpp': ['MergeFrom', 'MergeFromString'],
        'java': ['mergeFrom'],
        'python': ['MergeFrom', 'MergeFromString'],
    },
}

# Files to analyze for each language
LANGUAGE_FILES = {
    'cpp': [
        'src/google/protobuf/message_lite.h',
        'src/google/protobuf/message.h',
    ],
    'java': [
        'java/core/src/main/java/com/google/protobuf/MessageLite.java',
        'java/core/src/main/java/com/google/protobuf/Message.java',
        'java/core/src/main/java/com/google/protobuf/AbstractMessage.java',
    ],
    'python': [
        'python/google/protobuf/message.py',
    ],
}


@dataclass
class MethodInfo:
    """Information about a method found in the codebase."""
    name: str
    file: str
    line: int
    signature: str


@dataclass
class ConsistencyReport:
    """Report on API consistency for a language."""
    language: str
    total_canonical_methods: int
    implemented_methods: int
    has_canonical_alias: int
    missing_methods: List[str]
    methods_needing_alias: List[str]
    consistency_score: float
    canonical_score: float


def find_methods_in_file(filepath: str) -> List[MethodInfo]:
    """Find all method declarations in a file."""
    methods = []

    if not os.path.exists(filepath):
        return methods

    with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
        lines = f.readlines()

    # Patterns for different languages
    cpp_method_pattern = re.compile(
        r'(?:virtual\s+)?(?:static\s+)?(?:const\s+)?'
        r'(?:[\w:]+\s+)?(\w+)\s*\([^)]*\)\s*(?:const)?(?:\s*override)?(?:\s*=\s*0)?;'
    )

    java_method_pattern = re.compile(
        r'(?:public|protected|private|default)?\s*'
        r'(?:static\s+)?(?:final\s+)?(?:abstract\s+)?'
        r'(?:<[^>]+>\s+)?'
        r'(?:[\w<>\[\],\s]+)\s+(\w+)\s*\([^)]*\)'
    )

    python_method_pattern = re.compile(
        r'def\s+(\w+)\s*\([^)]*\):'
    )

    # Determine file type
    if filepath.endswith('.h') or filepath.endswith('.cc'):
        pattern = cpp_method_pattern
    elif filepath.endswith('.java'):
        pattern = java_method_pattern
    elif filepath.endswith('.py'):
        pattern = python_method_pattern
    else:
        return methods

    for i, line in enumerate(lines, 1):
        matches = pattern.findall(line)
        for match in matches:
            methods.append(MethodInfo(
                name=match,
                file=filepath,
                line=i,
                signature=line.strip()
            ))

    return methods


def analyze_language(language: str, base_path: str) -> Tuple[Set[str], Set[str]]:
    """Analyze a language implementation for canonical methods.

    Returns:
        Tuple of (found_methods, canonical_aliases)
    """
    found_methods: Set[str] = set()
    canonical_aliases: Set[str] = set()

    files = LANGUAGE_FILES.get(language, [])

    for filepath in files:
        full_path = os.path.join(base_path, filepath)
        methods = find_methods_in_file(full_path)

        for method in methods:
            found_methods.add(method.name)

            # Check if this is a canonical alias
            for canonical, mappings in CANONICAL_API.items():
                if method.name == canonical:
                    canonical_aliases.add(canonical)
                    break

    return found_methods, canonical_aliases


def calculate_consistency(language: str, found_methods: Set[str],
                         canonical_aliases: Set[str]) -> ConsistencyReport:
    """Calculate consistency score for a language."""
    total = 0
    implemented = 0
    has_alias = 0
    missing = []
    needs_alias = []

    for canonical, mappings in CANONICAL_API.items():
        lang_methods = mappings.get(language, [])

        if not lang_methods:
            # This canonical method doesn't apply to this language
            continue

        total += 1

        # Check if any of the current implementations exist
        method_exists = any(
            m.replace('<field>', '').replace('<Field>', '') in str(found_methods)
            for m in lang_methods
        )

        if method_exists:
            implemented += 1

            # Check if canonical alias exists
            if canonical in canonical_aliases:
                has_alias += 1
            else:
                needs_alias.append(canonical)
        else:
            missing.append(canonical)

    consistency_score = (implemented / total * 100) if total > 0 else 0
    canonical_score = (has_alias / implemented * 100) if implemented > 0 else 0

    return ConsistencyReport(
        language=language,
        total_canonical_methods=total,
        implemented_methods=implemented,
        has_canonical_alias=has_alias,
        missing_methods=missing,
        methods_needing_alias=needs_alias,
        consistency_score=consistency_score,
        canonical_score=canonical_score
    )


def print_report(report: ConsistencyReport, verbose: bool = False):
    """Print a consistency report."""
    print(f"\n{'='*50}")
    print(f"  {report.language.upper()} API Consistency Report")
    print(f"{'='*50}")
    print(f"  Implementation Score: {report.consistency_score:.1f}%")
    print(f"  Canonical Alias Score: {report.canonical_score:.1f}%")
    print(f"  Methods Implemented: {report.implemented_methods}/{report.total_canonical_methods}")
    print(f"  Canonical Aliases: {report.has_canonical_alias}/{report.implemented_methods}")

    if verbose:
        if report.missing_methods:
            print(f"\n  Missing Methods:")
            for method in report.missing_methods:
                print(f"    - {method}")

        if report.methods_needing_alias:
            print(f"\n  Methods Needing Canonical Alias:")
            for method in report.methods_needing_alias:
                print(f"    - {method}")

    print()


def generate_json_report(reports: List[ConsistencyReport]) -> dict:
    """Generate a JSON report from consistency reports."""
    return {
        'timestamp': '2025-11-19',
        'version': '1.0.0',
        'languages': {
            report.language: {
                'implementation_score': round(report.consistency_score, 2),
                'canonical_score': round(report.canonical_score, 2),
                'methods_implemented': report.implemented_methods,
                'total_methods': report.total_canonical_methods,
                'canonical_aliases': report.has_canonical_alias,
                'missing_methods': report.missing_methods,
                'needs_alias': report.methods_needing_alias,
            }
            for report in reports
        },
        'summary': {
            'average_implementation': round(
                sum(r.consistency_score for r in reports) / len(reports), 2
            ),
            'average_canonical': round(
                sum(r.canonical_score for r in reports) / len(reports), 2
            ),
        }
    }


def main():
    parser = argparse.ArgumentParser(
        description='Check API consistency across Protocol Buffers language implementations'
    )
    parser.add_argument(
        '--verbose', '-v',
        action='store_true',
        help='Show detailed information about missing methods'
    )
    parser.add_argument(
        '--json',
        action='store_true',
        help='Output results as JSON'
    )
    parser.add_argument(
        '--base-path',
        default='.',
        help='Base path to the protobuf repository'
    )

    args = parser.parse_args()

    # Analyze each language
    reports = []
    languages = ['cpp', 'java', 'python']

    for language in languages:
        found_methods, canonical_aliases = analyze_language(language, args.base_path)
        report = calculate_consistency(language, found_methods, canonical_aliases)
        reports.append(report)

    # Output results
    if args.json:
        json_report = generate_json_report(reports)
        print(json.dumps(json_report, indent=2))
    else:
        print("\n" + "="*50)
        print("  Protocol Buffers API Consistency Check")
        print("="*50)

        for report in reports:
            print_report(report, args.verbose)

        # Summary
        avg_impl = sum(r.consistency_score for r in reports) / len(reports)
        avg_canonical = sum(r.canonical_score for r in reports) / len(reports)

        print("="*50)
        print("  Summary")
        print("="*50)
        print(f"  Average Implementation Score: {avg_impl:.1f}%")
        print(f"  Average Canonical Score: {avg_canonical:.1f}%")
        print()

        if avg_canonical < 50:
            print("  Recommendation: Add canonical method aliases")
            print("  to improve cross-language consistency.")
        elif avg_canonical < 90:
            print("  Recommendation: Continue adding canonical aliases")
            print("  to reach 90% consistency target.")
        else:
            print("  Excellent! API consistency target met.")
        print()


if __name__ == '__main__':
    main()
