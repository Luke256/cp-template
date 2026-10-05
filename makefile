SHELL := /bin/bash
.PHONY: run template expand submit test clean

CXX = g++-14
CXXFLAGS = -std=c++20 -Wall -Wextra -O3 -fsanitize=undefined,address -g
SOURCE = main.cpp
TEMPLATE = tools/template
SUBMISSION = submit.cpp

INPUT = in.txt
ERRORLOG = error.log

PYTHON = python3
HEADERS = $(wildcard lib/*.hpp lib/*/*.hpp)

run: build/main
	@echo "Running..."
	touch $(INPUT)
	@./build/main < $(INPUT)

expand: $(TEMPLATE) $(HEADERS) tools/expand.py
	@$(PYTHON) tools/expand.py "$(TEMPLATE)" -o "$(SOURCE)"

template: $(TEMPLATE)
	@cp "$(TEMPLATE)" "$(SOURCE)"

submit: $(SUBMISSION)

$(SUBMISSION): $(SOURCE) $(HEADERS) tools/expand.py makefile
	@$(PYTHON) tools/expand.py "$(SOURCE)" -o "$@"

build/main: $(SOURCE) $(HEADERS) | build
	@echo "Compiling $(SOURCE)..."
	@if ! $(CXX) $(CXXFLAGS) -o build/main $(SOURCE) 2> $(ERRORLOG); then \
		echo "コンパイルエラーが発生しました"; \
		exit 1; \
	fi

	@if [ -s $(ERRORLOG) ]; then \
		echo "ビルド終了 (Warning)"; \
	else \
		echo "ビルド終了(正常)"; \
	fi

build:
	mkdir build

test: build/test-smoke build/test-expanded
	CXX="$(CXX)" $(PYTHON) -m unittest discover -s tests -p test_expand.py -v
	CXX="$(CXX)" $(PYTHON) -m unittest discover -s tests -p test_makefile.py -v
	./build/test-smoke
	./build/test-expanded

build/test-smoke: tests/smoke.cpp $(HEADERS) makefile | build
	$(CXX) $(CXXFLAGS) -UNDEBUG tests/smoke.cpp -o $@

build/test-expanded.cpp: tests/smoke.cpp $(HEADERS) tools/expand.py makefile | build
	$(PYTHON) tools/expand.py tests/smoke.cpp -o $@

build/test-expanded: build/test-expanded.cpp makefile | build
	$(CXX) $(CXXFLAGS) -UNDEBUG $< -o $@

clean:
	rm -rf build
	rm -f $(ERRORLOG)
	rm -f "$(SUBMISSION)"
	rm -rf tests/__pycache__
	rm -rf tools/__pycache__
