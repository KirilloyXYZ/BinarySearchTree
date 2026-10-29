#include <cstddef>
#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>

#include "BinarySearchTree.hpp"
#include "ArraySequence.hpp"
#include "Stack.hpp"

void Check(bool condition, const char* message)
{
    if (!condition) { throw std::runtime_error(message); }
}

template<typename Exception, typename Action>
void CheckThrows(Action action)
{
    try { action(); }
    catch (const Exception&) { return; }
    throw std::runtime_error("Expected exception was not thrown");
}

int Zero(const int&) { return 0; }
bool AtLeast20(const int& value) { return value >= 20; }
int Fold(const int& value, const int& result) { return 2 * value + 3 * result; }
bool Descending(const std::string& left, const std::string& right) { return left > right; }
bool NotStrict(const int& left, const int& right) { return left <= right; }

void TestStack()
{
    Stack<int> stack;
    const Stack<int>& readOnly = stack;
    Check(stack.IsEmpty(), "Empty stack");
    CheckThrows<std::out_of_range>([&]() { stack.Top(); });
    CheckThrows<std::out_of_range>([&]() { readOnly.Top(); });
    CheckThrows<std::out_of_range>([&]() { stack.Pop(); });

    stack.Push(10);
    stack.Push(20);
    Check(stack.Top() == 20 && readOnly.Top() == 20, "Stack top");
    stack.Top() = 25;
    Check(readOnly.Top() == 25, "Mutable stack top");
    stack.Pop();
    Check(stack.Top() == 10, "Stack LIFO order");
    stack.Pop();
    Check(stack.IsEmpty(), "Stack after Pop");
    stack.Push(30);
    Check(stack.Top() == 30, "Reuse empty stack");

    struct Frame
    {
        const int& value;
        std::size_t nextStep;

        explicit Frame(const int& value) : value(value), nextStep(0) { }
    };
    int value = 1;
    Stack<Frame> frames;
    frames.Push(Frame(value));
    value = 2;
    ++frames.Top().nextStep;
    Check(frames.Top().value == 2 && frames.Top().nextStep == 1, "Stack supports reference members");
}

void TestBasics()
{
    BinarySearchTree<std::string, int> tree;
    Tree<std::string, int>& base = tree;
    Check(base.GetCount() == 0 && base.GetHeight() == 0, "Empty tree");
    CheckThrows<std::out_of_range>([&]() { base.GetRoot(); });
    CheckThrows<std::out_of_range>([&]() { base.GetRootKey(); });
    CheckThrows<std::out_of_range>([&]() { base.Get(10); });

    base.Insert(20, "same");
    base.Insert(10, "same");
    base.Insert(30, "other");
    base.Insert(25, "child");
    Check(!base.Insert(20, "replacement"), "Duplicate key");
    Check(base.GetCount() == 4 && base.Get(20) == "same", "Key and value are independent");
    Check(base.Contains(25) && !base.Contains(100), "Search by key");
    Check(base.Remove(30) && base.Get(25) == "child", "Remove node with one child");
    Check(base.Remove(20) && base.GetCount() == 2, "Remove node with two children");
    Check(base.Get(10) == "same" && base.Get(25) == "child", "Values survive removal");
    Check(!base.Remove(100), "Missing key");
    base.Clear();
    Check(base.GetCount() == 0 && base.GetHeight() == 0, "Clear tree");
}

int visited[7];
std::size_t visitedCount = 0;

void Visit(const int& key, const int& value)
{
    Check(visitedCount < 7 && value == key * 10, "Traverse key and value");
    visited[visitedCount++] = key;
}

void TestTraversals()
{
    BinarySearchTree<int, int> tree;
    int keys[] = {4, 2, 6, 1, 3, 5, 7};
    for (int key : keys) { tree.Insert(key, key * 10); }
    TraversalOrder orders[] = {TraversalOrder::KLP, TraversalOrder::KPL, TraversalOrder::LKP,
                               TraversalOrder::LPK, TraversalOrder::PLK, TraversalOrder::PKL};
    int expected[6][7] = {
        {4, 2, 1, 3, 6, 5, 7}, {4, 6, 7, 5, 2, 3, 1},
        {1, 2, 3, 4, 5, 6, 7}, {1, 3, 2, 5, 7, 6, 4},
        {7, 5, 6, 3, 1, 2, 4}, {7, 6, 5, 4, 3, 2, 1}
    };
    const IEnumerable<int, int>& enumerable = tree;
    for (std::size_t i = 0; i < 6; ++i)
    {
        std::unique_ptr<IEnumerator<int, int>> enumerator = enumerable.GetEnumerator(orders[i]);
        CheckThrows<std::out_of_range>([&]() { enumerator->GetCurrent(); });
        CheckThrows<std::out_of_range>([&]() { enumerator->GetCurrentKey(); });
        for (std::size_t j = 0; j < 7; ++j)
        {
            Check(enumerator->MoveNext(), "Traversal length");
            Check(enumerator->GetCurrentKey() == expected[i][j], "Traversal order");
            Check(enumerator->GetCurrent() == expected[i][j] * 10, "Traversal value");
        }
        Check(!enumerator->MoveNext() && !enumerator->MoveNext(), "Traversal end");
        CheckThrows<std::out_of_range>([&]() { enumerator->GetCurrent(); });
        CheckThrows<std::out_of_range>([&]() { enumerator->GetCurrentKey(); });

        visitedCount = 0;
        enumerable.Traverse(orders[i], Visit);
        Check(visitedCount == 7, "Traverse length");
        for (std::size_t j = 0; j < 7; ++j)
        {
            Check(visited[j] == expected[i][j], "Traverse order");
        }
    }
    BinarySearchTree<int, int>::Enumerator first(tree);
    BinarySearchTree<int, int>::Enumerator second(tree);
    first.MoveNext();
    first.MoveNext();
    second.MoveNext();
    Check(first.GetCurrentKey() == 2 && second.GetCurrentKey() == 1, "Independent enumerators");
    CheckThrows<std::invalid_argument>([&]() { tree.GetEnumerator(static_cast<TraversalOrder>(-1)); });
}

void TestFunctions()
{
    BinarySearchTree<int, int> tree;
    tree.Insert(2, 10);
    tree.Insert(1, 20);
    tree.Insert(3, 20);
    std::unique_ptr<Tree<int, int>> mapped = tree.Map(Zero);
    Check(mapped->GetCount() == 3, "Map preserves distinct keys for equal values");
    Check(mapped->Get(1) == 0 && mapped->Get(2) == 0 && mapped->Get(3) == 0, "Map values");
    std::unique_ptr<Tree<int, int>> filtered = tree.Where(AtLeast20);
    Check(filtered->GetCount() == 2 && filtered->Get(1) == 20 && filtered->Get(3) == 20, "Where");
    Check(tree.Reduce(Fold, 4) == 568, "Reduce order and argument order");
    mapped->Remove(1);
    filtered->Remove(3);
    Check(tree.GetCount() == 3 && tree.Get(1) == 20 && tree.Get(3) == 20, "Independent results");

    ArraySequence<int> items;
    std::unique_ptr<IEnumerator<int, int>> enumerator = tree.GetEnumerator();
    while (items.GetLength() < 2 && enumerator->MoveNext())
    {
        items.Append(enumerator->GetCurrent());
    }
    Check(items.GetLength() == 2 && items.Get(0) == 20 && items.Get(1) == 10,
          "Collect values outside the tree");
    tree.Clear();
    Check(!tree.GetEnumerator()->MoveNext(), "Empty enumerator");
    Check(tree.Map(Zero)->GetCount() == 0 && tree.Where(AtLeast20)->GetCount() == 0, "Empty results");
    Check(tree.Reduce(Fold, 9) == 9, "Empty Reduce");
    CheckThrows<std::invalid_argument>([&]() { tree.Map(nullptr); });
    CheckThrows<std::invalid_argument>([&]() { tree.Where(nullptr); });
    CheckThrows<std::invalid_argument>([&]() { tree.Reduce(nullptr, 0); });
    CheckThrows<std::invalid_argument>([&]() { tree.Traverse(TraversalOrder::LKP, nullptr); });
}

struct Value
{
    int number;

    explicit Value(int number) : number(number) { }
};

Value Increase(const Value& value) { return Value(value.number + 1); }
bool AtLeastTwo(const Value& value) { return value.number >= 2; }
Value AddValues(const Value& value, const Value& result) { return Value(value.number + result.number); }

void TestTypes()
{
    BinarySearchTree<Value, std::string> records;
    records.Insert("second", Value{2});
    records.Insert("first", Value{1});
    Check(records.Get("first").number == 1, "Values need neither a default constructor nor comparison");
    std::unique_ptr<Tree<Value, std::string>> mapped = records.Map(Increase);
    Check(mapped->Get("first").number == 2 && mapped->Get("second").number == 3, "Map custom values");
    std::unique_ptr<Tree<Value, std::string>> filtered = records.Where(AtLeastTwo);
    Check(filtered->GetCount() == 1 && filtered->Get("second").number == 2, "Where custom values");
    Check(records.Reduce(AddValues, Value(10)).number == 13, "Reduce custom values");

    BinarySearchTree<double, std::string> tree(Descending);
    tree.Insert("a", 1.5);
    tree.Insert("b", 2.5);
    std::unique_ptr<IEnumerator<double, std::string>> enumerator = tree.GetEnumerator();
    Check(enumerator->MoveNext() && enumerator->GetCurrentKey() == "b" && enumerator->GetCurrent() == 2.5,
          "Custom comparator: first element");
    Check(enumerator->MoveNext() && enumerator->GetCurrentKey() == "a" && enumerator->GetCurrent() == 1.5,
          "Custom comparator: second element");
    Check(!enumerator->MoveNext(), "Custom comparator: end");
    CheckThrows<std::invalid_argument>([]() { BinarySearchTree<int, int> invalid(nullptr); });
    BinarySearchTree<int, int> invalid(NotStrict);
    CheckThrows<std::invalid_argument>([&]() { invalid.Insert(1, 10); });
}

void TestInvalidation()
{
    BinarySearchTree<int, int> tree;
    tree.Insert(1, 10);
    std::unique_ptr<IEnumerator<int, int>> enumerator = tree.GetEnumerator();
    enumerator->MoveNext();
    Check(!tree.Insert(1, 99) && !tree.Remove(100), "Failed changes");
    Check(enumerator->GetCurrent() == 10, "Failed changes keep enumerator valid");
    tree.Insert(2, 20);
    CheckThrows<std::logic_error>([&]() { enumerator->MoveNext(); });
    CheckThrows<std::logic_error>([&]() { enumerator->GetCurrentKey(); });
    enumerator = tree.GetEnumerator();
    enumerator->MoveNext();
    tree.Remove(1);
    CheckThrows<std::logic_error>([&]() { enumerator->GetCurrent(); });
    enumerator = tree.GetEnumerator();
    tree.Clear();
    CheckThrows<std::logic_error>([&]() { enumerator->MoveNext(); });
}

void TestAVL()
{
    int rotations[4][3] = {{30, 20, 10}, {10, 20, 30}, {30, 10, 20}, {10, 30, 20}};
    for (int (&keys)[3] : rotations)
    {
        BinarySearchTree<int, int> tree;
        for (int key : keys) { tree.Insert(key, -key); }
        Check(tree.GetRootKey() == 20 && tree.GetRoot() == -20 && tree.GetHeight() == 2, "AVL rotation");
    }

    BinarySearchTree<int, int> tree;
    bool present[128] = {};
    int expected[128] = {};
    std::size_t expectedCount = 0;
    std::mt19937 random(17);
    for (int step = 0; step < 1500; ++step)
    {
        int key = random() % 128;
        if (random() % 2 == 0)
        {
            Check(tree.Insert(key, step) == !present[key], "Random insert");
            if (!present[key])
            {
                present[key] = true;
                expected[key] = step;
                ++expectedCount;
            }
        }
        else
        {
            Check(tree.Remove(key) == present[key], "Random remove");
            if (present[key])
            {
                present[key] = false;
                --expectedCount;
            }
        }
        Check(tree.GetCount() == expectedCount, "Random count");
        std::unique_ptr<IEnumerator<int, int>> enumerator = tree.GetEnumerator();
        for (int expectedKey = 0; expectedKey < 128; ++expectedKey)
        {
            if (present[expectedKey])
            {
                Check(enumerator->MoveNext(), "Random traversal length");
                Check(enumerator->GetCurrentKey() == expectedKey && enumerator->GetCurrent() == expected[expectedKey],
                      "Random key and value");
            }
        }
        Check(!enumerator->MoveNext(), "Random traversal end");
        int heightLimit = 0;
        for (std::size_t count = expectedCount; count > 0; count /= 2) { heightLimit += 2; }
        Check(tree.GetHeight() <= heightLimit, "AVL height bound");
    }
}

int main()
{
    try
    {
        TestStack();
        TestBasics();
        TestTraversals();
        TestFunctions();
        TestTypes();
        TestInvalidation();
        TestAVL();
        std::cout << "All 7 test groups passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Test failed: " << error.what() << '\n';
        return 1;
    }
}
