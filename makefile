SHELL := /bin/bash
.PHONY: copy test

CXX = g++-14
CXXFLAGS = -std=c++20 -Wall -Wextra -O3 -fsanitize=undefined,address -g
SOURCE = main.cpp

INPUT = in.txt
ERRORLOG = error.log

PYTHON = python3
HEADERS = $(wildcard lib/*.hpp lib/*/*.hpp)

all: run .WAIT verify .WAIT copy

verify: build/main build/expanded
	@echo -n "Verifying... "
	@if diff -q <(./build/main < $(INPUT)) <(./build/expanded < $(INPUT)); then \
		echo "OK　(｀･ω･´)"; \
	else \
		echo "Failed (´・ω・｀)"; \
		exit 1; \
	fi

run: build/main
	@echo "Running..."
	@./build/main < $(INPUT)

build/main: $(SOURCE) $(HEADERS) build
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

build/expanded.cpp: $(SOURCE) $(HEADERS) tools/expand.py
	@$(PYTHON) tools/expand.py $(SOURCE) -o build/expanded.cpp

build:
	mkdir build
	
build/expanded: build/expanded.cpp | build
	@echo "Compiling expanded.cpp..."
	@$(CXX) $(CXXFLAGS) -o build/expanded build/expanded.cpp 2> $(ERRORLOG)

copy: build/expanded.cpp
	@xclip -selection clipboard < build/expanded.cpp
	@echo "Copied to clipboard"

test: build/test-smoke build/test-expanded
	CXX="$(CXX)" $(PYTHON) -m unittest discover -s tests -p test_expand.py -v
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
	rm -rf tests/__pycache__
	rm -rf tools/__pycache__
