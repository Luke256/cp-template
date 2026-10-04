#!/usr/bin/env python3
"""Inline each local header once, without submission include guards."""

import argparse
import re
import sys
from pathlib import Path
from tempfile import TemporaryDirectory

INCLUDE = re.compile(r'^\s*#\s*include\s*"([^"]+)"\s*(?://.*|/\*.*\*/\s*)?$')
PRAGMA_ONCE = re.compile(r'^\s*#\s*pragma\s+once\s*(?://.*|/\*.*\*/\s*)?$')
CONDITIONAL = re.compile(r'^\s*#\s*(ifdef|ifndef|if|endif)\b')
SCOPE_OPEN = re.compile(
    r'(?:^|>\s+)(namespace|struct|class|union|enum)\b[^{}();=]*\{$'
)
CPP_PART = re.compile(
    r'(?P<raw>(?P<prefix>u8|u|U|L)?R"(?P<delimiter>[^\s()\\]{0,16})'
    r'\((?P<body>.*?)\)(?P=delimiter)")'
    r'|(?:u8|u|U|L)?"(?:\\.|[^"\\])*"'
    r"|(?:u8|u|U|L)?'(?:\\.|[^'\\\n])*'"
    r"|\b\d[\w.']*"
    r'|(?P<comment>//(?:\\\r?\n|[^\n])*|/\*.*?\*/)',
    re.DOTALL,
)


def compact_cpp(code):
    """Compact formatted C++ declarations and methods, preserving type layout."""
    literals = []

    def protect(match):
        token = match.group(0)
        if match.group("comment") is not None:
            return " "
        if match.group("raw") is not None:
            escapes = str.maketrans({
                "\\": "\\\\", '"': '\\"', "\n": "\\n",
                "\r": "\\r", "\t": "\\t", "\0": "\\000",
            })
            token = (match.group("prefix") or "") + '"' + match.group("body").translate(escapes) + '"'
        else:
            token = token.replace("\\\r\n", "").replace("\\\n", "")
        literals.append(token)
        return f"\0{len(literals) - 1}\0"

    protected = CPP_PART.sub(protect, code).replace("\\\r\n", "").replace("\\\n", "")
    result = []
    pending = []
    scopes = []
    scope_starts = []
    previous_member = False
    indent = ""
    parentheses = brackets = braces = 0

    def flush():
        nonlocal previous_member
        if pending:
            text = " ".join(pending)
            member = bool(
                scopes and scopes[-1] and text.endswith(";")
                and not text.startswith(("using ", "typedef ", "friend ", "template "))
                and not any(char in text for char in "(){}")
            )
            if member and previous_member:
                result[-1] += " " + text
            else:
                result.append(indent + text)
            previous_member = member
            pending.clear()

    lines = [line.expandtabs(4) for line in protected.splitlines()]
    for index, original in enumerate(lines):
        line = " ".join(original.split())
        if not line:
            if not any(scopes) and parentheses == brackets == braces == 0 and not pending and result and result[-1]:
                    result.append("")
            continue
        if line.startswith("#"):
            flush()
            result.append(line)
            previous_member = False
            continue
        at_scope = parentheses == brackets == braces == 0
        if at_scope and (line.startswith("}") or line in ("public:", "protected:", "private:")):
            flush()
            previous_member = False
            if line.startswith("}") and scopes:
                type_scope = scopes.pop()
                start = scope_starts.pop()
                if type_scope and any(scopes) and all(
                    not part.lstrip().startswith("#")
                    and not any(char in part for char in "(){}")
                    for part in result[start + 1:]
                ):
                    result[start:] = [result[start] + " " + " ".join(
                        part.strip() for part in result[start + 1:] + [line]
                    )]
                    continue
            result.append(original[:len(original) - len(original.lstrip())] + line)
            continue
        if not pending:
            indent = original[:len(original) - len(original.lstrip())]
        pending.append(line)
        # Library headers follow .clang-format; only type/namespace blocks
        # retain their layout. Nested blocks inside a method stay on its line.
        scope = SCOPE_OPEN.search(" ".join(pending)) if at_scope else None
        if scope:
            flush()
            scopes.append(scope.group(1) != "namespace")
            scope_starts.append(len(result) - 1)
            continue
        parentheses += line.count("(") - line.count(")")
        brackets += line.count("[") - line.count("]")
        braces += line.count("{") - line.count("}")
        next_index = index + 1
        while next_index < len(lines) and not lines[next_index].strip():
            next_index += 1
        next_line = lines[next_index].lstrip() if next_index < len(lines) else ""
        if parentheses == brackets == braces == 0 and (
            line.endswith(";") or (line.endswith("}") and not next_line.startswith("{"))
        ):
            flush()
    flush()
    return re.sub(r"\0(\d+)\0", lambda match: literals[int(match.group(1))],
                  "\n".join(result).strip("\n"))


def expand(source, root, active=(), included=None):
    source = Path(source).resolve()
    root = Path(root).resolve()
    if source in active:
        raise ValueError(f"cyclic include: {source}")
    if included is None:
        included = set()
    if source in included:
        return ""
    included.add(source)
    lines = source.read_text(encoding="utf-8").splitlines()
    # Strip a conventional outer guard, keeping line numbers for diagnostics.
    guard = re.fullmatch(r'\s*#\s*ifndef\s+(\w+)\s*', lines[0]) if lines else None
    if (active and guard and len(lines) >= 3
            and lines[1].strip() == f"#define {guard.group(1)}"
            and re.fullmatch(r'\s*#\s*endif\s*(?://.*|/\*.*\*/\s*)?', lines[-1])):
        lines[0] = lines[1] = lines[-1] = ""
    result = []
    pending = []

    def flush():
        if pending:
            chunk = "\n".join(pending)
            result.append(compact_cpp(chunk) if active else chunk)
            pending.clear()

    depth = 0
    for number, line in enumerate(lines, 1):
        if PRAGMA_ONCE.fullmatch(line):
            continue
        directive = CONDITIONAL.match(line)
        if directive:
            depth += -1 if directive.group(1) == "endif" else 1
        match = INCLUDE.fullmatch(line)
        if not match:
            pending.append(line)
            continue
        # preprocessing if conditional header selection becomes necessary.
        if depth:
            raise ValueError(f"{source}:{number}: put local includes outside #if/#ifdef")
        name = match.group(1)
        candidates = [(source.parent / name).resolve(), (root / name).resolve()]
        header = next((p for p in candidates if p.is_file()), None)
        if header is None:
            raise FileNotFoundError(f'{source}:{number}: cannot find "{name}"')
        if not header.is_relative_to(root):
            raise ValueError(f"{source}:{number}: header is outside the project: {header}")
        if header in included and header not in active + (source,):
            continue
        flush()
        result.append(expand(header, root, active + (source,), included).rstrip("\n"))
    flush()
    return "\n".join(result) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("-o", "--output", type=Path, default=Path("build/submit.cpp"))
    args = parser.parse_args()
    try:
        if args.source.resolve() == args.output.resolve():
            raise ValueError("output must be different from the source")
        content = expand(args.source, Path(__file__).resolve().parent.parent)
        args.output.parent.mkdir(parents=True, exist_ok=True)
        # Replace the output only after expansion and writing both succeed.
        with TemporaryDirectory(dir=args.output.parent) as directory:
            staging = Path(directory) / "submit.cpp"
            staging.write_text(content, encoding="utf-8")
            staging.replace(args.output)
    except (OSError, ValueError) as error:
        print(f"expand: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
