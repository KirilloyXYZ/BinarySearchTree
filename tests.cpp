#include <cstddef>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

#include "BinarySearchTree.hpp"
#include "ArraySequence.hpp"
#include "Stack.hpp"
#include "DataGeneration.hpp"
#include "Set.hpp"

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

void WriteItem(std::ostream& stream, const int& key, const int& value)
{
    stream << key << " : " << value;
}

void TestPrint()
{
    BinarySearchTree<int, int> tree;
    std::ostringstream output;
    tree.Print(output, WriteItem);
    Check(output.str() == "Дерево пусто.\n", "Print empty tree");
    tree.Insert(2, 20);
    output.str("");
    tree.Print(output, WriteItem);
    Check(output.str() == "К: 2 : 20\n", "Print single node");
    tree.Insert(1, 10);
    tree.Insert(3, 30);
    output.str("");
    const Tree<int, int>& base = tree;
    base.Print(output, WriteItem);
    Check(output.str() == "К: 2 : 20\n├── Л: 1 : 10\n└── П: 3 : 30\n", "Print branches through interface");
    output.str("");
    tree.Print(output, WriteItem, 1);
    Check(output.str().find("Показано 1 из 3") != std::string::npos &&
          output.str().find("Л:") == std::string::npos, "Print depth limit");
    output.str("");
    tree.Print(output, WriteItem, 8, 2);
    Check(output.str().find("Показано 2 из 3") != std::string::npos &&
          output.str().find("П:") == std::string::npos, "Print node limit");
    Check(tree.IsValid() && tree.GetCount() == 3, "Print does not change tree");
    CheckThrows<std::invalid_argument>([&]() { tree.Print(output, nullptr); });
    CheckThrows<std::invalid_argument>([&]() { tree.Print(output, WriteItem, 0); });
    CheckThrows<std::invalid_argument>([&]() { tree.Print(output, WriteItem, 8, 0); });
    tree.Insert(0, 0);
    output.str("");
    tree.Print(output, WriteItem);
    Check(output.str().find("│   └── Л: 0 : 0") != std::string::npos, "Print nested branch prefix");
}

void TestGeneration()
{
    DynamicArray<int> ascending = GenerateKeys(9, -4, 4, DataOrder::Ascending, 17);
    DynamicArray<int> descending = GenerateKeys(9, -4, 4, DataOrder::Descending, 17);
    DynamicArray<int> random = GenerateKeys(9, -4, 4, DataOrder::Random, 17);
    DynamicArray<int> repeated = GenerateKeys(9, -4, 4, DataOrder::Random, 17);
    bool present[9] = {};
    for (std::size_t i = 0; i < 9; ++i)
    {
        Check(ascending.Get(i) == static_cast<int>(i) - 4, "Ascending generation");
        Check(descending.Get(i) == 4 - static_cast<int>(i), "Descending generation");
        Check(random.Get(i) >= -4 && random.Get(i) <= 4, "Generated key range");
        Check(!present[random.Get(i) + 4], "Distinct generated keys");
        present[random.Get(i) + 4] = true;
        Check(random.Get(i) == repeated.Get(i), "Reproducible generation");
    }
    Check(GenerateKeys(0, 1, 1, DataOrder::Random, 0).GetSize() == 0, "Empty generation");
    Check(GenerateKeys(1, -3, -3, DataOrder::Ascending, 0).Get(0) == -3, "Single generated key");
    DynamicArray<int> limits = GenerateKeys(3, std::numeric_limits<int>::min(),
                                          std::numeric_limits<int>::max(), DataOrder::Ascending, 0);
    Check(limits.Get(0) == std::numeric_limits<int>::min() && limits.Get(1) == -1 &&
          limits.Get(2) == std::numeric_limits<int>::max(), "Full integer range");
    CheckThrows<std::invalid_argument>([]() { GenerateKeys(1, 2, 1, DataOrder::Random, 0); });
    CheckThrows<std::invalid_argument>([]() { GenerateKeys(3, 1, 2, DataOrder::Random, 0); });
    CheckThrows<std::invalid_argument>([]() { GenerateKeys(0, 1, 2, static_cast<DataOrder>(-1), 0); });
}

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
    std::unique_ptr<IEnumerator<int, int>> first = enumerable.GetEnumerator();
    std::unique_ptr<IEnumerator<int, int>> second = enumerable.GetEnumerator();
    first->MoveNext();
    first->MoveNext();
    second->MoveNext();
    Check(first->GetCurrentKey() == 2 && second->GetCurrentKey() == 1, "Independent enumerators");
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
bool CompareKeys(const Value& left, const Value& right) { return left.number < right.number; }
bool SameValue(const Value& left, const Value& right) { return left.number == right.number; }
bool ByLength(const std::string& left, const std::string& right) { return left.size() < right.size(); }

void TestSetBasics()
{
    Set<int> set;
    const Set<int>& readOnly = set;
    Check(readOnly.IsEmpty() && readOnly.GetCount() == 0 && !readOnly.Contains(1), "Empty set");
    Check(!set.Remove(1), "Remove from empty set");
    std::ostringstream output;
    set.Print(output);
    Check(output.str() == "{}", "Print empty set");
    Check(set.Insert(3) && set.Insert(1) && set.Insert(2), "Set insertion");
    Check(!set.Insert(2) && set.GetCount() == 3 && readOnly.Contains(2), "Set ignores duplicates");
    output.str("");
    readOnly.Print(output);
    Check(output.str() == "{1, 2, 3}", "Set print order");
    output.str("");
    set.Print(output, 2);
    Check(output.str() == "{1, 2, ...}", "Set print limit");
    CheckThrows<std::invalid_argument>([&]() { set.Print(output, 0); });
    Check(set.Remove(2) && !set.Remove(2) && !set.Contains(2) && set.GetCount() == 2, "Set removal");
    set.Clear();
    set.Clear();
    Check(set.IsEmpty() && set.Insert(4) && set.Contains(4), "Reuse cleared set");
    CheckThrows<std::invalid_argument>([]() { Set<int> invalid(nullptr); });
}

void TestSetOperations()
{
    Set<int> first;
    Set<int> second;
    Set<int> empty;
    for (int value : {1, 2, 3}) { first.Insert(value); }
    for (int value : {3, 4}) { second.Insert(value); }
    std::unique_ptr<Set<int>> united = first.Union(second);
    std::unique_ptr<Set<int>> common = first.Intersection(second);
    std::unique_ptr<Set<int>> difference = first.Difference(second);
    Check(united->GetCount() == 4 && first.IsSubsetOf(*united) && second.IsSubsetOf(*united), "Set union");
    Check(common->GetCount() == 1 && common->Contains(3), "Set intersection");
    Check(difference->GetCount() == 2 && difference->Contains(1) && difference->Contains(2), "Set difference");
    Check(second.Difference(first)->GetCount() == 1 && second.Difference(first)->Contains(4), "Reverse difference");
    Check(!united->IsSubsetOf(first) && !first.IsSubsetOf(second), "Subset rejection");
    Set<int> reordered;
    for (int value : {4, 3, 2, 1}) { reordered.Insert(value); }
    Check(united->Equals(reordered) && reordered.Equals(*united), "Set equality ignores tree shape");
    Set<int> different;
    for (int value : {2, 3, 5}) { different.Insert(value); }
    Check(!first.Equals(different) && !first.IsSubsetOf(different), "Different sets of equal size");
    Check(first.Equals(first) && first.Union(first)->Equals(first) &&
          first.Intersection(first)->Equals(first) && first.Difference(first)->IsEmpty(), "Self operations");
    Check(empty.Equals(empty) && empty.IsSubsetOf(first) && !first.IsSubsetOf(empty) &&
          !empty.Equals(first), "Empty set relations");
    Check(first.Union(empty)->Equals(first) && empty.Union(first)->Equals(first) &&
          first.Intersection(empty)->IsEmpty() && empty.Difference(first)->IsEmpty() &&
          first.Difference(empty)->Equals(first), "Operations with empty set");
    Set<int> disjoint;
    disjoint.Insert(9);
    Check(first.Intersection(disjoint)->IsEmpty() && first.Difference(disjoint)->Equals(first), "Disjoint sets");
    united->Remove(3);
    common->Clear();
    Check(first.GetCount() == 3 && second.GetCount() == 2 && first.Contains(3) && second.Contains(3),
          "Set results are independent");
    first.Clear();
    second.Clear();
    Check(difference->GetCount() == 2 && difference->Contains(1) && united->Contains(4), "Results survive source clearing");
}

void TestSetTypes()
{
    Set<Value> first(CompareKeys);
    Set<Value> second(CompareKeys);
    first.Insert(Value(1));
    first.Insert(Value(2));
    second.Insert(Value(2));
    Check(!first.Insert(Value(2)) && first.Contains(Value(1)), "Set values need no default constructor or operators");
    Check(first.Union(second)->Equals(first) && first.Intersection(second)->Equals(second) &&
          first.Difference(second)->Contains(Value(1)), "Set operations with custom values");

    Set<std::string> descending(Descending);
    Set<std::string> other(Descending);
    descending.Insert("pear");
    descending.Insert("apple");
    other.Insert("banana");
    std::ostringstream output;
    std::unique_ptr<Set<std::string>> result = descending.Union(other);
    result->Print(output);
    Check(output.str() == "{pear, banana, apple}" && descending.IsSubsetOf(*result), "Set result preserves comparator");
    Set<std::string> ascending;
    CheckThrows<std::invalid_argument>([&]() { descending.Union(ascending); });
    CheckThrows<std::invalid_argument>([&]() { descending.Intersection(ascending); });
    CheckThrows<std::invalid_argument>([&]() { descending.Difference(ascending); });
    CheckThrows<std::invalid_argument>([&]() { descending.IsSubsetOf(ascending); });
    CheckThrows<std::invalid_argument>([&]() { descending.Equals(ascending); });

    Set<std::string> byLength(ByLength);
    Set<std::string> sameLength(ByLength);
    byLength.Insert("one");
    sameLength.Insert("cat");
    Check(!byLength.Insert("two") && byLength.Contains("dog") && byLength.Equals(sameLength), "Comparator defines set equivalence");
    Check(byLength.Union(sameLength)->GetCount() == 1 && byLength.Intersection(sameLength)->Equals(byLength) &&
          byLength.Difference(sameLength)->IsEmpty(), "Algebra uses comparator equivalence");
}

void TestRandomSets()
{
    Set<int> first;
    Set<int> second;
    bool present[2][64] = {};
    std::mt19937 random(29);
    for (int step = 0; step < 500; ++step)
    {
        int side = random() % 2;
        int key = random() % 64;
        Set<int>& selected = side == 0 ? first : second;
        bool insert = random() % 2 == 0;
        bool changed = insert ? selected.Insert(key) : selected.Remove(key);
        Check(changed == (insert ? !present[side][key] : present[side][key]), "Random set change");
        present[side][key] = insert;
    }
    std::unique_ptr<Set<int>> united = first.Union(second);
    std::unique_ptr<Set<int>> common = first.Intersection(second);
    std::unique_ptr<Set<int>> difference = first.Difference(second);
    std::size_t unionCount = 0, commonCount = 0, differenceCount = 0;
    bool subset = true, equal = true;
    for (int key = 0; key < 64; ++key)
    {
        bool inUnion = present[0][key] || present[1][key];
        bool inCommon = present[0][key] && present[1][key];
        bool inDifference = present[0][key] && !present[1][key];
        Check(united->Contains(key) == inUnion && common->Contains(key) == inCommon &&
              difference->Contains(key) == inDifference, "Random set algebra");
        unionCount += inUnion;
        commonCount += inCommon;
        differenceCount += inDifference;
        if (inDifference) { subset = false; }
        if (present[0][key] != present[1][key]) { equal = false; }
    }
    Check(united->GetCount() == unionCount && common->GetCount() == commonCount &&
          difference->GetCount() == differenceCount, "Random set result counts");
    Check(first.IsSubsetOf(second) == subset && first.Equals(second) == equal, "Random set relations");
}

void TestOperators()
{
    BinarySearchTree<std::string, int> tree;
    const BinarySearchTree<std::string, int>& readOnly = tree;
    CheckThrows<std::out_of_range>([&]() { readOnly[1]; });
    Check(tree.GetCount() == 0, "Missing subscript does not insert");
    (tree += {2, "two"}) += {1, "one"};
    Check(&(tree += {3, "three"}) == &tree, "Insertion operator returns this tree");
    Check(readOnly[1] == "one" && readOnly[3] == "three", "Subscript reads values");
    std::unique_ptr<IEnumerator<std::string, int>> enumerator = tree.GetEnumerator();
    enumerator->MoveNext();
    tree += {2, "replacement"};
    Check(tree.GetCount() == 3 && tree[2] == "two", "Insertion operator keeps duplicate value");
    Check(enumerator->GetCurrent() == "one", "Duplicate insertion keeps enumerator valid");
    tree += {4, "four"};
    CheckThrows<std::logic_error>([&]() { enumerator->MoveNext(); });
    Check(tree.IsValid(), "Operator insertion preserves AVL invariants");
}

void TestSubtrees()
{
    BinarySearchTree<int, int> tree;
    BinarySearchTree<int, int> empty;
    Check(tree.ContainsSubtree(empty), "Empty tree contains empty subtree");
    CheckThrows<std::out_of_range>([&]() { tree.GetSubtree(1); });
    int keys[] = {4, 2, 7, 1, 3, 6, 8, 5};
    for (int key : keys) { tree.Insert(key, key * 10); }
    std::unique_ptr<IEnumerator<int, int>> enumerator = tree.GetEnumerator();
    enumerator->MoveNext();
    std::unique_ptr<BinarySearchTree<int, int>> subtree = tree.GetSubtree(7);
    Check(subtree->GetRootKey() == 7 && subtree->GetCount() == 4 && subtree->GetHeight() == 3 &&
          subtree->IsValid(), "Extract preserves subtree root, count and heights");
    int expected[] = {7, 6, 5, 8};
    std::unique_ptr<IEnumerator<int, int>> copied = subtree->GetEnumerator(TraversalOrder::KLP);
    for (int key : expected)
    {
        Check(copied->MoveNext() && copied->GetCurrentKey() == key && copied->GetCurrent() == key * 10,
              "Extract preserves structure and values");
    }
    Check(!copied->MoveNext(), "Extract traversal length");
    Check(tree.ContainsSubtree(*subtree) && tree.ContainsSubtree(tree), "Matching and whole subtrees");
    Check(tree.ContainsSubtree(empty) && !empty.ContainsSubtree(tree), "Empty subtree semantics");
    CheckThrows<std::out_of_range>([&]() { tree.GetSubtree(99); });
    CheckThrows<std::invalid_argument>([&]() { tree.ContainsSubtree(empty, nullptr); });
    Check(enumerator->GetCurrentKey() == 1, "Subtree operations keep source enumerator valid");

    BinarySearchTree<int, int> wrong;
    wrong.Insert(5, 999);
    Check(!tree.ContainsSubtree(wrong), "Subtree values must match");
    wrong.Clear();
    wrong.Insert(99, 50);
    Check(!tree.ContainsSubtree(wrong), "Missing subtree root");
    wrong.Clear();
    for (int key : {6, 5, 7, 8}) { wrong.Insert(key, key * 10); }
    Check(!tree.ContainsSubtree(wrong), "Same records with different structure do not match");
    wrong.Clear();
    wrong.Insert(7, 70);
    wrong.Insert(4, 60);
    wrong.Insert(8, 80);
    wrong.Insert(3, 50);
    Check(!tree.ContainsSubtree(wrong), "Subtree keys must match");

    subtree->Remove(5);
    Check(tree.Contains(5) && !tree.ContainsSubtree(*subtree), "Copy is independent and missing branches do not match");
    std::unique_ptr<BinarySearchTree<int, int>> leaf = tree.GetSubtree(5);
    Check(leaf->GetCount() == 1 && leaf->GetHeight() == 1 && tree.ContainsSubtree(*leaf), "Leaf extraction");
    std::unique_ptr<BinarySearchTree<int, int>> whole = tree.GetSubtree(tree.GetRootKey());
    Check(whole->GetCount() == tree.GetCount() && tree.ContainsSubtree(*whole), "Whole tree extraction");
    tree.Clear();
    Check(whole->GetCount() == 8 && whole->IsValid() && leaf->Get(5) == 50, "Copies survive source clearing");
}

void TestTypes()
{
    static_assert(IsLessComparable<int>::value, "Integer keys support comparison");
    static_assert(IsLessComparable<std::string>::value, "String keys support comparison");
    static_assert(!IsLessComparable<Value>::value, "Value has no operator<");
    BinarySearchTree<int, Value> keyed(CompareKeys);
    keyed.Insert(Value(2), 20);
    keyed.Insert(Value(1), 10);
    Check(keyed.Get(Value(1)) == 10 && keyed.IsValid(), "Keys without operator< use a custom comparator");
    Check(keyed.Map(Zero)->Get(Value(2)) == 0, "Map preserves custom comparison for keys without operator<");
    Check(keyed.ContainsSubtree(*keyed.GetSubtree(Value(2))), "Subtree keys need no comparison operators");
    Check(keyed.Remove(Value(2)) && keyed.IsValid(), "Remove key without operator<");

    BinarySearchTree<Value, std::string> records;
    records.Insert("second", Value{2});
    records.Insert("first", Value{1});
    Check(records.Get("first").number == 1, "Values need neither a default constructor nor comparison");
    std::unique_ptr<Tree<Value, std::string>> mapped = records.Map(Increase);
    Check(mapped->Get("first").number == 2 && mapped->Get("second").number == 3, "Map custom values");
    std::unique_ptr<Tree<Value, std::string>> filtered = records.Where(AtLeastTwo);
    Check(filtered->GetCount() == 1 && filtered->Get("second").number == 2, "Where custom values");
    Check(records.Reduce(AddValues, Value(10)).number == 13, "Reduce custom values");
    std::unique_ptr<BinarySearchTree<Value, std::string>> record = records.GetSubtree("first");
    Check(records.ContainsSubtree(*record, SameValue), "Custom subtree value equality without operator==");
    record->Remove("first");
    record->Insert("first", Value(99));
    Check(!records.ContainsSubtree(*record, SameValue), "Custom equality detects different values");

    BinarySearchTree<double, std::string> tree(Descending);
    tree.Insert("a", 1.5);
    tree.Insert("b", 2.5);
    std::unique_ptr<IEnumerator<double, std::string>> enumerator = tree.GetEnumerator();
    Check(enumerator->MoveNext() && enumerator->GetCurrentKey() == "b" && enumerator->GetCurrent() == 2.5,
          "Custom comparator: first element");
    Check(enumerator->MoveNext() && enumerator->GetCurrentKey() == "a" && enumerator->GetCurrent() == 1.5,
          "Custom comparator: second element");
    Check(!enumerator->MoveNext(), "Custom comparator: end");
    Check(tree.IsValid(), "Validation with custom comparator");
    std::unique_ptr<BinarySearchTree<double, std::string>> reversed = tree.GetSubtree("a");
    Check(tree.ContainsSubtree(*reversed), "Subtree with descending comparator");
    reversed->Insert("c", 3.5);
    std::unique_ptr<IEnumerator<double, std::string>> first = reversed->GetEnumerator();
    Check(first->MoveNext() && first->GetCurrentKey() == "c" && reversed->IsValid() && !tree.Contains("c"),
          "Extract preserves comparator and independent ownership");
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
        Check(tree.IsValid(), "AVL rotation invariants");
    }

    int removals[4][6] = {{3, 2, 4, 1, 4, 2}, {2, 1, 3, 4, 1, 3},
                          {5, 2, 6, 4, 6, 4}, {2, 1, 5, 3, 1, 3}};
    for (int (&items)[6] : removals)
    {
        BinarySearchTree<int, int> tree;
        for (int i = 0; i < 4; ++i) { tree.Insert(items[i], -items[i]); }
        Check(tree.Remove(items[4]), "Remove before AVL rotation");
        Check(tree.GetRootKey() == items[5] && tree.GetCount() == 3 && tree.IsValid(), "Deletion AVL rotation");
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
        Check(tree.IsValid(), "Random AVL invariants");
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

void TestOrderedAVL()
{
    for (DataOrder order : {DataOrder::Ascending, DataOrder::Descending})
    {
        BinarySearchTree<int, int> tree;
        Check(tree.IsValid(), "Empty AVL invariants");
        DynamicArray<int> keys = GenerateKeys(10000, 0, 9999, order, 0);
        for (std::size_t i = 0; i < keys.GetSize(); ++i) { tree.Insert(keys.Get(i), keys.Get(i)); }
        Check(tree.IsValid(), "Ordered insertion invariants");
        int rootKey = tree.GetRootKey();
        // Имитируем повреждение порядка и восстанавливаем ключ до следующих операций.
        const_cast<int&>(tree.GetRootKey()) = 10000;
        Check(!tree.IsValid(), "Validation detects invalid key order");
        const_cast<int&>(tree.GetRootKey()) = rootKey;
        for (std::size_t i = 0; i < keys.GetSize(); ++i)
        {
            Check(tree.Remove(keys.Get(i)), "Ordered removal");
            if (i % 257 == 0) { Check(tree.IsValid(), "Deletion rebalancing invariants"); }
        }
        Check(tree.IsValid() && tree.GetCount() == 0, "Remove all nodes");
    }
}

void TestAsymmetricTraversals()
{
    BinarySearchTree<int, int> tree;
    int keys[] = {4, 2, 7, 1, 3, 6, 8, 5};
    for (int key : keys) { tree.Insert(key, key); }
    int expected[6][8] = {
        {4, 2, 1, 3, 7, 6, 5, 8}, {4, 7, 8, 6, 5, 2, 3, 1},
        {1, 2, 3, 4, 5, 6, 7, 8}, {1, 3, 2, 5, 6, 8, 7, 4},
        {8, 5, 6, 7, 3, 1, 2, 4}, {8, 7, 6, 5, 4, 3, 2, 1}
    };
    for (int order = 0; order < 6; ++order)
    {
        std::unique_ptr<IEnumerator<int, int>> enumerator = tree.GetEnumerator(static_cast<TraversalOrder>(order));
        for (int key : expected[order])
        {
            Check(enumerator->MoveNext() && enumerator->GetCurrentKey() == key, "Asymmetric traversal");
        }
        Check(!enumerator->MoveNext(), "Asymmetric traversal length");
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
        TestPrint();
        TestGeneration();
        TestOrderedAVL();
        TestAsymmetricTraversals();
        TestOperators();
        TestSubtrees();
        TestSetBasics();
        TestSetOperations();
        TestSetTypes();
        TestRandomSets();
        std::cout << "All 17 test groups passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Test failed: " << error.what() << '\n';
        return 1;
    }
}
