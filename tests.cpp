#include <iostream>

#include "TreeTests.hpp"
#include "SetTests.hpp"
#include "SequenceTests.hpp"
#include "EnumerationTests.hpp"

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
    TestTreeTypes();
    std::cout << "PASS: generic types and input validation\n";
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
    TestTraversalOrders();
    std::cout << "PASS: all six traversal orders and Traverse\n";
    TestEnumeratorStates();
    std::cout << "PASS: empty, singleton and independent enumerators\n";
    TestEnumeratorInvalidation();
    std::cout << "PASS: enumerator invalidation and owner lifetime\n";
    TestSetEnumeration();
    std::cout << "PASS: set enumeration and custom types\n";
    std::cout << "All 17 test groups passed!\n";
    return 0;
}
