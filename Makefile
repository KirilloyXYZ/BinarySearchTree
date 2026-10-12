.PHONY: all run run_tests run_benchmark plots check clean

all: lab1_app

lab1_app: main.o Student.o
	g++ main.o Student.o -o lab1_app

main.o: main.cpp Tree.hpp BinarySearchTree.hpp Set.hpp TreeSet.hpp ValueFunctions.hpp Student.hpp
	g++ -std=c++17 -Wall -Wextra -c main.cpp

Student.o: Student.cpp Student.hpp
	g++ -std=c++17 -Wall -Wextra -c Student.cpp

tests: tests.o TreeTests.o SetTests.o Student.o
	g++ tests.o TreeTests.o SetTests.o Student.o -o tests

tests.o: tests.cpp TreeTests.hpp SetTests.hpp
	g++ -std=c++17 -Wall -Wextra -c tests.cpp

TreeTests.o: TreeTests.cpp TreeTests.hpp Tree.hpp BinarySearchTree.hpp ValueFunctions.hpp Student.hpp
	g++ -std=c++17 -Wall -Wextra -c TreeTests.cpp

SetTests.o: SetTests.cpp SetTests.hpp Set.hpp TreeSet.hpp Tree.hpp BinarySearchTree.hpp ValueFunctions.hpp Student.hpp
	g++ -std=c++17 -Wall -Wextra -c SetTests.cpp

benchmark: benchmark.cpp Tree.hpp BinarySearchTree.hpp Set.hpp TreeSet.hpp ValueFunctions.hpp
	g++ -std=c++17 -Wall -Wextra -O2 benchmark.cpp -o benchmark

run: lab1_app
	./lab1_app

run_tests: tests
	./tests

run_benchmark: benchmark
	./benchmark 1000000 results/timings.csv 3 both

plots:
	python3 plot_results.py

check: TreeTests.cpp SetTests.cpp tests.cpp Student.cpp Tree.hpp BinarySearchTree.hpp Set.hpp TreeSet.hpp ValueFunctions.hpp Student.hpp
	g++ -std=c++17 -Wall -Wextra -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer tests.cpp TreeTests.cpp SetTests.cpp Student.cpp -o tests_sanitized
	./tests_sanitized

clean:
	rm -f *.o lab1_app tests benchmark tests_sanitized
	rm -rf tests_sanitized.dSYM
