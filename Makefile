.PHONY: all run run_tests run_benchmark check clean

HEADERS = Tree.hpp BinarySearchTree.hpp Set.hpp TreeSet.hpp Sequence.hpp ArraySequence.hpp DynamicArray.hpp CompareValues.hpp IEnumerable.hpp IEnumerator.hpp TreeEnumerator.hpp TraversalOrder.hpp

all: lab1_app

lab1_app: main.o
	g++ main.o -o lab1_app

main.o: main.cpp $(HEADERS) ValueFunctions.hpp
	g++ -std=c++17 -Wall -Wextra -c main.cpp

tests: tests.o TreeTests.o SetTests.o SequenceTests.o EnumerationTests.o
	g++ tests.o TreeTests.o SetTests.o SequenceTests.o EnumerationTests.o -o tests

tests.o: tests.cpp TreeTests.hpp SetTests.hpp SequenceTests.hpp EnumerationTests.hpp
	g++ -std=c++17 -Wall -Wextra -c tests.cpp

TreeTests.o: TreeTests.cpp TreeTests.hpp SequenceTests.hpp $(HEADERS) ValueFunctions.hpp
	g++ -std=c++17 -Wall -Wextra -c TreeTests.cpp

SetTests.o: SetTests.cpp SetTests.hpp SequenceTests.hpp $(HEADERS)
	g++ -std=c++17 -Wall -Wextra -c SetTests.cpp

SequenceTests.o: SequenceTests.cpp SequenceTests.hpp Sequence.hpp ArraySequence.hpp DynamicArray.hpp
	g++ -std=c++17 -Wall -Wextra -c SequenceTests.cpp

EnumerationTests.o: EnumerationTests.cpp EnumerationTests.hpp SequenceTests.hpp $(HEADERS)
	g++ -std=c++17 -Wall -Wextra -c EnumerationTests.cpp

benchmark: benchmark.cpp $(HEADERS) ValueFunctions.hpp
	g++ -std=c++17 -Wall -Wextra -O2 benchmark.cpp -o benchmark

run: lab1_app
	./lab1_app

run_tests: tests
	./tests

run_benchmark: benchmark
	./benchmark 1000000 results/timings.csv 3 both

check: TreeTests.cpp SetTests.cpp SequenceTests.cpp EnumerationTests.cpp tests.cpp TreeTests.hpp SetTests.hpp SequenceTests.hpp EnumerationTests.hpp $(HEADERS) ValueFunctions.hpp
	g++ -std=c++17 -Wall -Wextra -g -O1 -fsanitize=address,undefined -fno-omit-frame-pointer tests.cpp TreeTests.cpp SetTests.cpp SequenceTests.cpp EnumerationTests.cpp -o tests_sanitized
	./tests_sanitized

clean:
	rm -f *.o lab1_app tests benchmark tests_sanitized
	rm -rf tests_sanitized.dSYM
