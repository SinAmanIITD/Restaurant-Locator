CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra -Iinclude

.PHONY: all run test clean

all: build/main

run: build/main
	./build/main

test: build/test_range_tree
	./build/test_range_tree

build/main: src/main.cpp include/*.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) src/main.cpp -o build/main

build/test_range_tree: tests/test_range_tree.cpp include/*.h
	@mkdir -p build
	$(CXX) $(CXXFLAGS) tests/test_range_tree.cpp -o build/test_range_tree

clean:
	rm -rf build
