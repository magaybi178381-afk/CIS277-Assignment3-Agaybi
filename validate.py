#!/usr/bin/env python3
"""Public preflight for CIS-277 Assignment 3. This does not assign a grade."""

import argparse
import os
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile


ROOT = Path(__file__).resolve().parent
REQUIRED = [
    "MessageInbox.h", "MessageInbox.cpp", "main.cpp", "README.md",
    "tests/public_tests.cpp",
]
HEADINGS = [
    "# CIS-277 Assignment 3: Text Messages and Dynamic Memory",
    "## Student", "## Design", "## How to Compile", "## How to Run",
    "## Tests and Results", "## Analysis Questions", "## AI Use",
]


def run_command(command, timeout):
    try:
        result = subprocess.run(
            command, cwd=ROOT, capture_output=True, text=True, timeout=timeout
        )
    except FileNotFoundError:
        print(f"INCOMPLETE: command not found: {command[0]}")
        return None
    except subprocess.TimeoutExpired:
        print(f"FAIL: timed out after {timeout} seconds: {' '.join(command)}")
        return False
    if result.returncode != 0:
        print(f"FAIL: command exited {result.returncode}: {' '.join(command)}")
        if result.stdout.strip():
            print(result.stdout[-6000:])
        if result.stderr.strip():
            print(result.stderr[-6000:])
        return False
    if result.stdout.strip():
        print(result.stdout.strip()[-3000:])
    return True


def readme_headings(text):
    """Accept heading lines, not headings shown inside a fenced example."""
    headings = set()
    fence = None
    for raw_line in text.splitlines():
        line = raw_line.strip()
        if fence is not None:
            if line.startswith(fence):
                fence = None
            continue
        if line.startswith("```") or line.startswith("~~~"):
            fence = line[:3]
        elif line.startswith("#"):
            headings.add(line)
    return headings


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--compiler", default=os.environ.get("CXX", "g++"),
        help="C++23 compiler command (default: CXX or g++; current cl is also supported)",
    )
    parser.add_argument(
        "--sanitize", action="store_true",
        help="enable AddressSanitizer and UndefinedBehaviorSanitizer with GCC/clang",
    )
    args = parser.parse_args()

    missing = [name for name in REQUIRED if not (ROOT / name).is_file()]
    if missing:
        print("FAIL: required files missing from the repository root:")
        for name in missing:
            print("  " + name)
        return 1
    print("PASS: required files found")

    readme = (ROOT / "README.md").read_text(encoding="utf-8-sig", errors="replace")
    headings = readme_headings(readme)
    missing_headings = [heading for heading in HEADINGS if heading not in headings]
    if missing_headings:
        print("FAIL: README headings missing (headings in fenced examples do not count):")
        for heading in missing_headings:
            print("  " + heading)
        return 1
    print("PASS: README headings found (content still needs instructor review)")

    try:
        compiler = shlex.split(args.compiler)
    except ValueError as error:
        print(f"INCOMPLETE: invalid compiler command: {error}")
        return 2
    if not compiler:
        print("INCOMPLETE: no compiler command was provided")
        return 2
    msvc = Path(compiler[0]).name.lower() in ("cl", "cl.exe")
    if msvc and args.sanitize:
        print("INCOMPLETE: --sanitize requires GCC or clang in this preflight")
        return 2

    def compile_command(sources, target, header_only=False):
        if msvc:
            options = [
                "/nologo", "/std:c++latest", "/Zc:__cplusplus", "/EHsc", "/W4",
                "/I", str(ROOT),
            ]
            if header_only:
                options.append("/DA3_HEADER_ONLY")
            return compiler + options + [str(ROOT / name) for name in sources] + [
                "/Fo" + str(Path(target).parent) + os.sep, "/Fe" + target,
            ]
        options = ["-std=c++23", "-Wall", "-Wextra", "-pedantic", "-I", str(ROOT)]
        if header_only:
            options.append("-DA3_HEADER_ONLY")
        if args.sanitize:
            options += [
                "-fsanitize=address,undefined", "-fno-omit-frame-pointer", "-g",
            ]
        return compiler + options + [str(ROOT / name) for name in sources] + ["-o", target]

    with tempfile.TemporaryDirectory(prefix="cis277-a3-") as directory:
        suffix = ".exe" if os.name == "nt" else ""
        stages = [
            (
                "independent header check",
                ["tests/public_tests.cpp"], "header_check" + suffix, True, False,
            ),
            (
                "full public tests",
                ["tests/public_tests.cpp", "MessageInbox.cpp"],
                "public_tests" + suffix, False, True,
            ),
            (
                "student demonstration",
                ["main.cpp", "MessageInbox.cpp"],
                "message_inbox" + suffix, False, False,
            ),
        ]
        for label, sources, filename, header_only, execute in stages:
            target = str(Path(directory) / filename)
            print(f"Compiling {label}...")
            compiled = run_command(compile_command(sources, target, header_only), 60)
            if compiled is None:
                return 2
            if not compiled:
                return 1
            if execute:
                print(f"Running {label}...")
                passed = run_command([target], 15)
                if passed is None:
                    return 2
                if not passed:
                    return 1

    print("PASS: public preflight checks completed. Review the full assignment before submitting.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
