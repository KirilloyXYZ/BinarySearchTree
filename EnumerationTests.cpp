#include "EnumerationTests.hpp"
#include "SequenceTests.hpp"

#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>

#include "BinarySearchTree.hpp"
#include "TreeSet.hpp"

static const TraversalOrder orders[] = {
    TraversalOrder::KLP, TraversalOrder::KPL, TraversalOrder::LKP,
    TraversalOrder::LPK, TraversalOrder::PLK, TraversalOrder::PKL
};

static void CheckNoCurrent(const IEnumerator<int>& enumerator)
{
    bool thrown = false;
    try { enumerator.GetCurrent(); }
    catch (const std::out_of_range&) { thrown = true; }
    assert(thrown);
}

static void CheckEnumeration(const IEnumerable<int>& container, TraversalOrder order,
                             std::initializer_list<int> expected)
{
    std::unique_ptr<IEnumerator<int>> enumerator = container.GetEnumerator(order);
    CheckNoCurrent(*enumerator);
    for (int value : expected)
    {
        assert(enumerator->MoveNext());
        assert(enumerator->GetCurrent() == value);
        assert(enumerator->GetCurrent() == value);
    }
    assert(!enumerator->MoveNext());
    CheckNoCurrent(*enumerator);
    assert(!enumerator->MoveNext());

    ArraySequence<int> visited;
    container.Traverse(order, [&visited](const int& value) { visited.Append(value); });
    CheckItems(visited, expected);
}

void TestTraversalOrders()
{
    int values[] = {4, 2, 6, 1, 3, 5, 7};
    BinarySearchTree<int> tree(values, 7);
    const Tree<int>& interface = tree;
    std::initializer_list<int> expected[] = {
        {4, 2, 1, 3, 6, 5, 7}, {4, 6, 7, 5, 2, 3, 1},
        {1, 2, 3, 4, 5, 6, 7}, {1, 3, 2, 5, 7, 6, 4},
        {7, 5, 6, 3, 1, 2, 4}, {7, 6, 5, 4, 3, 2, 1}
    };
    for (std::size_t i = 0; i < 6; ++i)
    {
        CheckEnumeration(interface, orders[i], expected[i]);
    }

    int unevenValues[] = {4, 2, 6, 3, 5};
    BinarySearchTree<int> uneven(unevenValues, 5);
    std::initializer_list<int> unevenExpected[] = {
        {4, 2, 3, 6, 5}, {4, 6, 5, 2, 3}, {2, 3, 4, 5, 6},
        {3, 2, 5, 6, 4}, {5, 6, 3, 2, 4}, {6, 5, 4, 3, 2}
    };
    for (std::size_t i = 0; i < 6; ++i)
    {
        CheckEnumeration(uneven, orders[i], unevenExpected[i]);
    }

    int sum = 0;
    interface.Traverse(TraversalOrder::LKP, [&sum](const int& value) { sum += value; });
    assert(sum == 28 && tree.GetCount() == 7 && tree.IsValid());
    bool thrown = false;
    try
    {
        interface.Traverse(TraversalOrder::KLP, [](const int&) { throw std::runtime_error("action failure"); });
    }
    catch (const std::runtime_error&) { thrown = true; }
    assert(thrown && tree.IsValid());
}

void TestEnumeratorStates()
{
    BinarySearchTree<int> empty;
    BinarySearchTree<int> single;
    single.Insert(42);
    for (TraversalOrder order : orders)
    {
        CheckEnumeration(empty, order, {});
        CheckEnumeration(single, order, {42});
    }

    int values[] = {2, 1, 3};
    BinarySearchTree<int> tree(values, 3);
    std::unique_ptr<IEnumerator<int>> first = tree.GetEnumerator();
    std::unique_ptr<IEnumerator<int>> second = tree.GetEnumerator(TraversalOrder::PKL);
    assert(first->MoveNext() && first->GetCurrent() == 1);
    assert(second->MoveNext() && second->GetCurrent() == 3);
    assert(first->MoveNext() && first->GetCurrent() == 2);
    assert(second->GetCurrent() == 3);
    std::unique_ptr<IEnumerator<int>> root = tree.GetEnumerator(TraversalOrder::KLP);
    assert(root->MoveNext() && &root->GetCurrent() == &tree.GetRoot());

    for (int invalid : {-1, 6})
    {
        bool thrown = false;
        try { empty.GetEnumerator(static_cast<TraversalOrder>(invalid)); }
        catch (const std::invalid_argument&) { thrown = true; }
        assert(thrown);
        thrown = false;
        bool called = false;
        try { tree.Traverse(static_cast<TraversalOrder>(invalid), [&called](const int&) { called = true; }); }
        catch (const std::invalid_argument&) { thrown = true; }
        assert(thrown && !called);
    }
}

static void CheckInvalidated(IEnumerator<int>& enumerator)
{
    bool thrown = false;
    try { enumerator.MoveNext(); }
    catch (const std::logic_error&) { thrown = true; }
    assert(thrown);
    thrown = false;
    try { enumerator.GetCurrent(); }
    catch (const std::out_of_range&) { assert(false); }
    catch (const std::logic_error&) { thrown = true; }
    assert(thrown);
}

void TestEnumeratorInvalidation()
{
    int values[] = {2, 1, 3};
    for (int started = 0; started < 2; ++started)
    {
        for (int mode = 0; mode < 4; ++mode)
        {
            BinarySearchTree<int> tree(values, 3);
            std::unique_ptr<IEnumerator<int>> enumerator = tree.GetEnumerator();
            if (started) { assert(enumerator->MoveNext()); }
            if (mode == 0) { tree.Insert(4); }
            if (mode == 1) { tree.Remove(2); }
            if (mode == 2) { tree.Clear(); }
            if (mode == 3) { BinarySearchTree<int> replacement; replacement.Insert(9); tree = replacement; }
            CheckInvalidated(*enumerator);
            std::size_t visited = 0;
            tree.Traverse(TraversalOrder::LKP, [&visited](const int&) { ++visited; });
            assert(visited == tree.GetCount() && tree.IsValid());
        }
        std::unique_ptr<IEnumerator<int>> expired;
        {
            BinarySearchTree<int> tree(values, 3);
            expired = tree.GetEnumerator();
            if (started) { assert(expired->MoveNext()); }
        }
        CheckInvalidated(*expired);
    }

    BinarySearchTree<int> tree(values, 3);
    std::unique_ptr<IEnumerator<int>> stable = tree.GetEnumerator();
    assert(stable->MoveNext());
    assert(!tree.Insert(2) && !tree.Remove(999));
    BinarySearchTree<int>& same = tree;
    tree = same;
    BinarySearchTree<int> copy(tree);
    copy.Clear();
    assert(stable->GetCurrent() == 1 && stable->MoveNext() && stable->GetCurrent() == 2);

    BinarySearchTree<int> empty;
    std::unique_ptr<IEnumerator<int>> cleared = empty.GetEnumerator();
    empty.Clear();
    CheckInvalidated(*cleared);

    bool thrown = false;
    try { tree.Traverse(TraversalOrder::LKP, [&tree](const int&) { tree.Clear(); }); }
    catch (const std::logic_error&) { thrown = true; }
    assert(thrown && tree.GetCount() == 0);
}

class EnumerationValue
{
public:
    static int copies;
    int key;
    EnumerationValue() : key(0) { }
    explicit EnumerationValue(int key) : key(key) { }
    EnumerationValue(const EnumerationValue& other) : key(other.key) { ++copies; }
    EnumerationValue& operator=(const EnumerationValue& other) { this->key = other.key; ++copies; return *this; }
};

int EnumerationValue::copies = 0;
static bool CompareEnumerationValues(const EnumerationValue& left, const EnumerationValue& right)
{
    return left.key < right.key;
}

void TestSetEnumeration()
{
    int values[] = {4, 2, 6, 1, 3, 5, 7};
    TreeSet<int> set(values, 7);
    const Set<int>& interface = set;
    std::initializer_list<int> expected[] = {
        {4, 2, 1, 3, 6, 5, 7}, {4, 6, 7, 5, 2, 3, 1},
        {1, 2, 3, 4, 5, 6, 7}, {1, 3, 2, 5, 7, 6, 4},
        {7, 5, 6, 3, 1, 2, 4}, {7, 6, 5, 4, 3, 2, 1}
    };
    for (std::size_t i = 0; i < 6; ++i) { CheckEnumeration(interface, orders[i], expected[i]); }
    TreeSet<int> empty;
    TreeSet<int> single;
    single.Insert(42);
    for (TraversalOrder order : orders)
    {
        CheckEnumeration(empty, order, {});
        CheckEnumeration(single, order, {42});
    }
    for (int mode = 0; mode < 4; ++mode)
    {
        TreeSet<int> changed(values, 7);
        std::unique_ptr<IEnumerator<int>> enumerator = changed.GetEnumerator();
        assert(enumerator->MoveNext());
        if (mode == 0) { changed.Insert(8); }
        if (mode == 1) { changed.Remove(4); }
        if (mode == 2) { changed.Clear(); }
        if (mode == 3) { changed = single; }
        CheckInvalidated(*enumerator);
    }

    TreeSet<EnumerationValue> custom(CompareEnumerationValues);
    custom.Insert(EnumerationValue(2));
    custom.Insert(EnumerationValue(1));
    TreeSet<EnumerationValue> copy(custom);
    EnumerationValue::copies = 0;
    int sum = 0;
    custom.Traverse(TraversalOrder::LKP, [&sum](const EnumerationValue& value) { sum += value.key; });
    assert(sum == 3 && custom.IsSubsetOf(copy) && EnumerationValue::copies == 0);

    TreeSet<std::string> words;
    words.Insert("b");
    words.Insert("a");
    std::string joined;
    words.Traverse(TraversalOrder::LKP, [&joined](const std::string& value) { joined += value; });
    assert(joined == "ab");
    BinarySearchTree<double> numbers;
    numbers.Insert(1.25);
    numbers.Insert(-0.5);
    double total = 0;
    numbers.Traverse(TraversalOrder::LKP, [&total](const double& value) { total += value; });
    assert(total == 0.75);
}
