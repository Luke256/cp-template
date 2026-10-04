from pathlib import Path
import os
import shutil
import subprocess
import sys
from tempfile import TemporaryDirectory
import unittest


class PlayTest(unittest.TestCase):
    def test_matching_and_mismatching_outputs(self):
        project = Path(__file__).resolve().parents[1]
        with TemporaryDirectory() as directory:
            root = Path(directory)
            (root / "tools").mkdir()
            (root / "lib").mkdir()
            for name in ("makefile", "play.sh", "tools/expand.py"):
                shutil.copyfile(project / name, root / name)
            (root / "main.cpp").write_text(
                '#include <iostream>\n#include "lib/value.hpp"\n'
                'int main() { int n; std::cin >> n; std::cout << value() + n << "\\n"; }\n',
                encoding="utf-8",
            )
            (root / "in.txt").write_text("0\n", encoding="utf-8")
            clipboard = root / "clip.exe"
            clipboard.write_text(
                '#!/usr/bin/env python3\nfrom pathlib import Path\nimport sys\n'
                'Path("clipboard.bin").write_bytes(sys.stdin.buffer.read())\n',
                encoding="utf-8",
            )
            clipboard.chmod(0o755)
            environment = dict(os.environ, PATH=f"{root}:{os.environ['PATH']}",
                               CXXFLAGS="-O0 -std=gnu++20", PYTHON=sys.executable)
            header = root / "lib/value.hpp"
            header.write_text('inline int value() {\n    return 7;\n}\n', encoding="utf-8")
            result = subprocess.run(["sh", "play.sh", "-j8"], cwd=root, env=environment,
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("\n7\n", result.stdout)
            self.assertIn("出力一致\n", result.stdout)
            self.assertEqual(result.stderr, "")
            stages = [
                " main.cpp -o build/Original", "./build/Original < in.txt",
                "tools/expand.py main.cpp", " build/submit.cpp -o Main",
                "./Main < in.txt", "diff -u build/main.out build/submit.out",
                'subprocess.run(["clip.exe"]',
            ]
            positions = [result.stdout.index(stage) for stage in stages]
            self.assertEqual(positions, sorted(positions))
            self.assertEqual((root / "build/main.out").read_text(), "7\n")
            self.assertEqual((root / "build/submit.out").read_text(), "7\n")
            copied = (root / "clipboard.bin").read_bytes()
            self.assertEqual(copied.decode("utf-16"),
                             (root / "build/submit.cpp").read_text(encoding="utf-8"))

            outputs = [root / name for name in ("build/Original", "Main", "build/submit.cpp")]
            timestamps = [path.stat().st_mtime_ns for path in outputs]
            result = subprocess.run(["sh", "play.sh"], cwd=root, env=environment,
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("\n7\n", result.stdout)
            self.assertNotIn("g++-14", result.stdout)
            self.assertEqual([path.stat().st_mtime_ns for path in outputs], timestamps)

            (root / "in.txt").write_text("1\n", encoding="utf-8")
            result = subprocess.run(["sh", "play.sh"], cwd=root, env=environment,
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("\n8\n", result.stdout)
            self.assertEqual([path.stat().st_mtime_ns for path in outputs], timestamps)

            header.write_text('inline int value() {\n    return 7;\n}\n// comment\n', encoding="utf-8")
            result = subprocess.run(["sh", "play.sh"], cwd=root, env=environment,
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertIn("\n8\n", result.stdout)
            self.assertNotEqual(outputs[0].stat().st_mtime_ns, timestamps[0])
            self.assertEqual([path.stat().st_mtime_ns for path in outputs[1:]], timestamps[1:])

            result = subprocess.run(["make", "-s"], cwd=root, env=environment,
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            self.assertEqual(result.stdout, "8\nGenerated build/submit.cpp\n8\n出力一致\n")
            self.assertEqual(outputs[1].stat().st_mtime_ns, timestamps[1])
            self.assertEqual((root / "clipboard.bin").read_bytes(), copied)

            (root / "in.txt").write_text("0\n", encoding="utf-8")
            header.write_text('inline int value() {\n    return __LINE__;\n}\n', encoding="utf-8")
            result = subprocess.run(["sh", "play.sh"], cwd=root, env=environment,
                                    capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("--- build/main.out", result.stdout)
            self.assertIn("+++ build/submit.out", result.stdout)
            self.assertEqual((root / "build/main.out").read_text(), "2\n")
            self.assertEqual((root / "build/submit.out").read_text(), "3\n")
            self.assertNotEqual(outputs[1].stat().st_mtime_ns, timestamps[1])
            self.assertEqual((root / "clipboard.bin").read_bytes(), copied)

            timestamp = outputs[1].stat().st_mtime_ns
            header.unlink()
            result = subprocess.run(["make", "-s"], cwd=root, env=environment,
                                    capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("lib/value.hpp", result.stderr)
            self.assertEqual(outputs[1].stat().st_mtime_ns, timestamp)
            self.assertEqual((root / "clipboard.bin").read_bytes(), copied)


if __name__ == "__main__":
    unittest.main()
