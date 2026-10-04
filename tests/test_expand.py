from pathlib import Path
import os
import shlex
import subprocess
import sys
from tempfile import TemporaryDirectory
import unittest

from tools.expand import compact_cpp, expand

CXX = shlex.split(os.environ.get("CXX", "g++-14"))


class ExpandTest(unittest.TestCase):
    def test_compaction_matches_expected_file(self):
        directory = Path(__file__).resolve().parent
        source = (directory / "input.cpp").read_text(encoding="utf-8")
        expected = (directory / "expected.cpp").read_text(encoding="utf-8")
        self.maxDiff = None
        self.assertEqual(compact_cpp(source), expected.rstrip("\n"))

    def test_nested_and_duplicate_includes(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "lib").mkdir()
            (root / "lib/core.hpp").write_text(
                '#pragma once /* stripped on expansion */\n'
                '#include <vector>\ninline int value() {\n    return 7;\n}\n',
                encoding="utf-8",
            )
            (root / "lib/wrapper.hpp").write_text(
                '#ifndef TEST_WRAPPER_HPP\n#define TEST_WRAPPER_HPP\n'
                '# include "core.hpp" // relative include\n#endif\n', encoding="utf-8",
            )
            source = root / "main.cpp"
            source.write_text(
                '#include "lib/wrapper.hpp"\n#include "lib/core.hpp"\n'
                '#include "lib/wrapper.hpp"\n'
                'int main() {\n    return value() != 7;\n}\n', encoding="utf-8",
            )
            content = expand(source, root)
            self.assertIn("#include <vector>", content)
            self.assertNotIn('#include "', content)
            self.assertNotIn("#pragma once", content)
            self.assertNotIn("#ifndef", content)
            self.assertNotIn("#endif", content)
            self.assertEqual(content.count("inline int value()"), 1)
            self.assertIn('int main() {\n    return value() != 7;\n}', content)
            # Compile away from the headers, without an include search path.
            (root / "isolated").mkdir()
            combined = root / "isolated/submit.cpp"
            combined.write_text(content, encoding="utf-8")
            binary = root / "isolated/Submit"
            subprocess.run([*CXX, "-std=gnu++20", str(combined), "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True)

    def test_compaction_preserves_literals_comments_and_macros(self):
        code = r'''#include <cassert>
#include <string>
#define TWICE(x) \
    ((x) + \
     (x))
#define COMMENT_VALUE 1 /* multiline
comment */ + 2
// remove this comment
int main() {
    int/**/value = TWICE(2);
    const char* text = "http://example/a  b";
    const char* raw = R"tag(first
second // keep "quotes" \path)tag";
    const char quote = '\'';
    const char slash = '\\';
    long number = 1'000;
    assert(value == 4);
    assert(COMMENT_VALUE == 3);
    assert(std::string(text) == "http://example/a  b");
    assert(std::string(raw) == "first\nsecond // keep \"quotes\" \\path");
    assert(quote == '\'' && slash == '\\' && number == 1000);
}
'''
        content = compact_cpp(code)
        self.assertNotIn("remove this comment", content)
        self.assertIn('"http://example/a  b"', content)
        self.assertIn("#define TWICE(x) ((x) + (x))", content)
        code_lines = [line for line in content.splitlines() if line.strip() and not line.startswith("#")]
        self.assertEqual(len(code_lines), 1)
        with TemporaryDirectory() as directory:
            source = Path(directory) / "submit.cpp"
            source.write_text(content, encoding="utf-8")
            binary = Path(directory) / "Submit"
            subprocess.run([*CXX, "-std=gnu++20", str(source), "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True)

    def test_compaction_keeps_methods_on_separate_lines(self):
        code = '''namespace demo {
    struct Counter {
        int value;
        Counter(int initial)
            : value{initial} {
        }
        int get() const {
            return value;
        }
        void add(int amount) {
            auto change = [amount](int x) {
                return x + amount;
            };
            if (amount > 0) {
                value = change(value);
            }
        }
        template <class T>
            requires requires(T x) {
                x + 1;
            }
        int twice(T x) const {
            return x + x;
        }
    };
}
int main() {
    demo::Counter counter(3);
    counter.add(4);
    return counter.get() != 7 || counter.twice(5) != 10;
}
'''
        content = compact_cpp(code)
        self.assertIn("namespace demo {\n    struct Counter {\n", content)
        self.assertIn("        int value;\n", content)
        self.assertIn("        Counter(int initial) : value{initial} { }\n", content)
        self.assertIn("        int get() const { return value; }\n", content)
        self.assertIn(
            "        void add(int amount) { auto change = [amount](int x) { "
            "return x + amount; }; if (amount > 0) { value = change(value); } }\n", content,
        )
        self.assertIn("    };\n}\n", content)
        with TemporaryDirectory() as directory:
            source = Path(directory) / "submit.cpp"
            source.write_text(content, encoding="utf-8")
            binary = Path(directory) / "Submit"
            subprocess.run([*CXX, "-std=gnu++20", str(source), "-o", str(binary)], check=True)
            subprocess.run([str(binary)], check=True)

    def test_errors_leave_existing_output_intact(self):
        tool = Path(__file__).resolve().parents[1] / "tools/expand.py"
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "main.cpp"
            source.write_text('#include "missing.hpp"\n', encoding="utf-8")
            output = root / "submit.cpp"
            output.write_text("previous submission\n", encoding="utf-8")
            result = subprocess.run(
                [sys.executable, str(tool), str(source), "-o", str(output)],
                capture_output=True, text=True,
            )
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("cannot find", result.stderr)
            self.assertEqual(output.read_text(encoding="utf-8"), "previous submission\n")
            result = subprocess.run(
                [sys.executable, str(tool), str(source), "-o", str(source)],
                capture_output=True, text=True,
            )
            self.assertNotEqual(result.returncode, 0)
            self.assertEqual(source.read_text(encoding="utf-8"), '#include "missing.hpp"\n')

    def test_cycles_are_rejected(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "main.cpp"
            source.write_text('#include "main.cpp"\n', encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "cyclic include"):
                expand(source, root)

    def test_conditional_local_includes_are_rejected(self):
        with TemporaryDirectory() as directory:
            root = Path(directory)
            source = root / "main.cpp"
            source.write_text('#ifdef LOCAL\n#include "debug.hpp"\n#endif\n', encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "outside #if/#ifdef"):
                expand(source, root)
            source.write_text('#if 0\nint unused;\n#endif\nint main() {}\n', encoding="utf-8")
            self.assertIn("#if 0", expand(source, root))


if __name__ == "__main__":
    unittest.main()
