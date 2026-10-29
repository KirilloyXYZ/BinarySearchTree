.PHONY: all run test clean

all: lab1_app

lab1_app: main.o
	g++ main.o -o lab1_app

main.o: main.cpp Tree.hpp BinarySearchTree.hpp Stack.hpp IEnumerable.hpp IEnumerator.hpp TraversalOrder.hpp
	g++ -std=c++17 -Wall -Wextra -c main.cpp

run: lab1_app
	./lab1_app

lab1_tests: tests.cpp Tree.hpp BinarySearchTree.hpp Stack.hpp Sequence.hpp ArraySequence.hpp DynamicArray.hpp IEnumerable.hpp IEnumerator.hpp TraversalOrder.hpp
	g++ -std=c++17 -Wall -Wextra tests.cpp -o lab1_tests

test: lab1_tests
	./lab1_tests

clean:
	rm -f *.o lab1_app lab1_tests
