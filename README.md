# 競技プログラミングテンプレート

AtCoderなどの競技プログラミングで使う、C++20の作業環境と自作ライブラリです。
`main.cpp`で解答を作成し、ローカルのヘッダーを1つのファイルに展開して提出できます。

## 必要な環境

- Bash
- GNU Make
- GCC / G++（既定のコマンドは`g++-14`）
- Python 3.9以上（提出用コードの生成とテストに使用）

## 使い方

1. リポジトリのルートで、解答を書く環境を用意する。

```bash
make template  # テンプレートをコピーし、ヘッダーを読み込む短い main.cpp を作成
```

最初からファイル単体で動作するコードを書く場合は、代わりに次を実行します。

```bash
make expand    # テンプレートのローカルヘッダーを展開した main.cpp を作成
```

**`make template`と`make expand`は、現在の`main.cpp`を上書きします。**
どちらも`tools/template.cpp`から新しく作るコマンドなので、解答を書き始める前に使います。

2. `main.cpp`に解答、`in.txt`に入力例を書く。
3. コンパイルして実行する。

```bash
make
```

`main.cpp`をコンパイルし、`in.txt`を標準入力として実行します。

4. 解答ができたら提出用ファイルを生成する。

```bash
make submit
```

AtCoderには生成された`expanded.cpp`の内容を提出します。
`make submit`は現在の`main.cpp`のローカルヘッダーを展開して別ファイルに保存し、
`main.cpp`は変更しません。すでに展開済みの`main.cpp`にも使えます。

### 個別に実行する

| コマンド | 内容 |
| --- | --- |
| `make template` | `tools/template.cpp`をコピーして`main.cpp`を初期化 |
| `make expand` | `tools/template.cpp`を展開して単体で動作する`main.cpp`を初期化 |
| `make build/main` | `main.cpp`をコンパイル |
| `make run` | コンパイルして`in.txt`で実行 |
| `make submit` / `make expanded.cpp` | 現在の`main.cpp`を展開して`expanded.cpp`に保存 |
| `make test` | 展開ツールとライブラリのテストを実行 |
| `make clean` | 実行ファイル・生成コード・ビルド出力・ログ・Pythonキャッシュを削除 |

`main.cpp`のコンパイル時の警告・エラーは`error.log`に保存されます。

コンパイラや入力ファイルは、コマンドラインで変更できます。

```bash
make run CXX=g++ INPUT=sample.txt
make test CXX=g++
make submit SUBMISSION=build/submit.cpp
```

### 展開ツールを直接使う

```bash
python3 tools/expand.py main.cpp -o expanded.cpp
```

`#include "..."`で指定したプロジェクト内のヘッダーを再帰的に展開し、
重複したヘッダーは1回だけ取り込みます。標準ライブラリの`#include <...>`は残ります。
ヘッダー部分のコメントや空白を整理し、`main.cpp`本体の書式は維持します。

ローカルの`#include`は`#if`・`#ifdef`などの条件分岐の外に記述してください。
循環参照・存在しないヘッダー・プロジェクト外のヘッダーはエラーになります。

## ライブラリ

まとめて使う場合は、次のヘッダーを読み込みます。

```cpp
#include "lib/all.hpp"
```

必要なヘッダーだけを個別に読み込むこともできます。

## 構成

```text
.
├── main.cpp          # C++の解答を作成するファイル
├── main.py           # Pythonの解答用ファイル（makeの対象外）
├── in.txt            # ローカル実行用の入力
├── makefile          # 初期化・ビルド・実行・提出用コードの生成
├── lib/              # 自作C++ライブラリ
├── tools/
│   ├── template.cpp  # 解答用テンプレート
│   └── expand.py     # ローカルヘッダーの展開ツール
├── tests/            # ライブラリと展開ツールのテスト
├── .vscode/          # VS Codeの設定
└── .zed/             # Zedの設定
```

生成される主なファイルは、実行ファイル`build/main`と提出用コード`expanded.cpp`です。

## テスト

```bash
make test
```

`tests/test_expand.py`でヘッダー展開とコード整形を確認し、
`tests/test_makefile.py`で初期化・提出用コードの生成を確認します。
`tests/smoke.cpp`を元の形と展開後の形の両方でコンパイル・実行します。
