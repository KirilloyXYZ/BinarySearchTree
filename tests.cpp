#include <iostream>

#include "TreeTests.hpp"
#include "SetTests.hpp"
#include "SequenceTests.hpp"

int main()
{
    TestSequenceBasics();
    std::cout << "PASS: sequences, copies and moves\n";
    TestSequenceLifetimeAndExceptions();
    std::cout << "PASS: sequence lifetime and exception cleanup\n";
    TestTreeBasics();
    std::cout << "PASS: tree basics and polymorphism\n";
    TestTreeBalanceAndRemoval();
    std::cout << "PASS: AVL rotations, removal and reference comparison\n";
    TestTreeCopy();
    std::cout << "PASS: independent copies\n";
    TestTreeAlgorithms();
    std::cout << "PASS: Map, Where, Reduce\n";
    TestTreeSubtrees();
    std::cout << "PASS: subtrees\n";
    TestTreeSerialization();
    std::cout << "PASS: serialization and invalid input\n";
    TestTreeTypes();
    std::cout << "PASS: generic types and students\n";
    TestTreeLifetimeAndExceptions();
    std::cout << "PASS: lifetime and exception cleanup\n";
    TestSetBasics();
    std::cout << "PASS: set basics and copies\n";
    TestSetOperations();
    std::cout << "PASS: set operations\n";
    TestSetAlgorithmsAndTypes();
    std::cout << "PASS: set algorithms and types\n";
    TestSetErrors();
    std::cout << "PASS: set error handling\n";
    std::cout << "All 14 test groups passed!\n";
    return 0;
}
