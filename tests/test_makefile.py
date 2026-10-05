from pathlib import Path
import os
import shlex
import shutil
import subprocess
import sys
from tempfile import TemporaryDirectory
import unittest


CXX = shlex.split(os.environ.get("CXX", "g++-14"))


class MakefileTest(unittest.TestCase):
    def setUp(self):
        self.directory = TemporaryDirectory()
        self.addCleanup(self.directory.cleanup)
        self.root = Path(self.directory.name)
        project = Path(__file__).resolve().parents[1]
        shutil.copyfile(project / "makefile", self.root / "makefile")
        shutil.copytree(project / "tools", self.root / "tools",
                        ignore=shutil.ignore_patterns("__pycache__"))
        shutil.copytree(project / "lib", self.root / "lib")

    def make(self, *arguments, succeeds=True):
        result = subprocess.run(
            ["make", "-s", *arguments, f"PYTHON={sys.executable}"],
            cwd=self.root, capture_output=True, text=True,
        )
        if succeeds:
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        else:
            self.assertNotEqual(result.returncode, 0)
        return result

    def test_template_resets_source_even_with_target_file(self):
        (self.root / "template").touch()
        self.make("template")
        source = self.root / "main.cpp"
        expected = (self.root / "tools/template.cpp").read_bytes()
        self.assertEqual(source.read_bytes(), expected)
        source.write_text("previous solution\n", encoding="utf-8")
        self.make("template")
        self.assertEqual(source.read_bytes(), expected)

    def test_expand_initializes_standalone_source(self):
        (self.root / "expand").touch()
        self.make("expand")
        source = self.root / "main.cpp"
        content = source.read_text(encoding="utf-8")
        self.assertNotIn('#include "', content)
        self.assertIn("int main ()", content)
        isolated = self.root / "isolated"
        isolated.mkdir()
        standalone = isolated / "main.cpp"
        standalone.write_text(content, encoding="utf-8")
        subprocess.run([*CXX, "-std=c++20", "-fsyntax-only", str(standalone)],
                       cwd=isolated, check=True)
        self.make("submit")
        self.assertEqual((self.root / "expanded.cpp").read_text(encoding="utf-8"), content)

    def test_submit_preserves_solution_and_supports_custom_output(self):
        source = self.root / "main.cpp"
        original = (b'#include "lib/all.hpp"\r\n'
                    b'int main() {\r\n    std::cout << 42 << "\\n";\r\n}\r\n')
        source.write_bytes(original)
        self.make("submit", "SUBMISSION=isolated/answer.cpp")
        output = self.root / "isolated/answer.cpp"
        self.assertEqual(source.read_bytes(), original)
        self.assertNotIn('#include "', output.read_text(encoding="utf-8"))
        binary = output.parent / "answer"
        subprocess.run([*CXX, "-std=c++20", str(output), "-o", str(binary)],
                       cwd=output.parent, check=True)
        result = subprocess.run([str(binary)], capture_output=True, text=True, check=True)
        self.assertEqual(result.stdout, "42\n")

    def test_failed_expansion_preserves_existing_files(self):
        source = self.root / "main.cpp"
        source.write_text("previous solution\n", encoding="utf-8")
        (self.root / "tools/template.cpp").write_text(
            '#include "missing.hpp"\n', encoding="utf-8",
        )
        self.make("expand", succeeds=False)
        self.assertEqual(source.read_text(encoding="utf-8"), "previous solution\n")
        source.write_text('#include "missing.hpp"\n', encoding="utf-8")
        output = self.root / "expanded.cpp"
        output.write_text("previous submission\n", encoding="utf-8")
        self.make("-B", "submit", succeeds=False)
        self.assertEqual(output.read_text(encoding="utf-8"), "previous submission\n")


if __name__ == "__main__":
    unittest.main()
