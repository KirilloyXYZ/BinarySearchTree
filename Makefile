.PHONY: all run clean

all: lab1_app

lab1_app: main.o
	g++ main.o -o lab1_app

main.o: main.cpp Tree.hpp BinarySearchTree.hpp
	g++ -std=c++17 -Wall -Wextra -c main.cpp

run: lab1_app
	./lab1_app

clean:
	rm -f *.o lab1_app
