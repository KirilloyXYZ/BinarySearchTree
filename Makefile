CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic
PYTHON = .venv/bin/python
HEADERS = $(wildcard *.hpp)

.PHONY: all run test sanitize benchmark setup graphs clean

all: lab1_app

lab1_app: main.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) main.cpp -o $@

run: lab1_app
	./lab1_app

lab1_tests: tests.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) tests.cpp -o $@

test: lab1_tests
	./lab1_tests

sanitize:
	$(CXX) $(CXXFLAGS) -g -fsanitize=address,undefined -fno-omit-frame-pointer tests.cpp -o lab1_tests_sanitized
	./lab1_tests_sanitized

lab1_bench: benchmark.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -O2 benchmark.cpp -o $@

benchmark: lab1_bench
	./lab1_bench

setup:
	python3 -m venv .venv
	$(PYTHON) -m pip install -r requirements.txt

graphs:
	$(PYTHON) plot_benchmarks.py results/benchmark.csv

clean:
	rm -f *.o lab1_app lab1_tests lab1_tests_sanitized lab1_bench
