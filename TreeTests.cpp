#include "TreeTests.hpp"

#include <cassert>
#include <cmath>
#include <limits>
#include <random>
#include <set>

#include "BinarySearchTree.hpp"
#include "Student.hpp"

static int Negate(const int& value) { return -value; }
static int Zero(const int&) { return 0; }
static bool Even(const int& value) { return value % 2 == 0; }
static bool Always(const int&) { return true; }
static bool Never(const int&) { return false; }
static int Sum(const int& value, const int& result) { return value + result; }
static int LabReduce(const int& value, const int& result) { return 2 * value + 3 * result; }
static bool Descending(const int& left, const int& right) { return left > right; }

class InspectedTree : public BinarySearchTree<int>
{
private:
    using Node = Tree<int>::Node;

    int CheckHeights(const Node* node) const
    {
        if (node == nullptr)
        {
            return 0;
        }

        int left = this->CheckHeights(node->left.get());
        int right = this->CheckHeights(node->right.get());
        int height = 1 + std::max(left, right);
        assert(std::abs(left - right) <= 1);
        assert(node->height == height);
        return height;
    }

public:
    void Check(const std::set<int>& expected) const
    {
        std::vector<int> values = this->GetItems();
        std::vector<int> correct(expected.begin(), expected.end());
        assert(values == correct);
        assert(this->GetCount() == static_cast<int>(expected.size()));
        int actualHeight = this->CheckHeights(this->root.get());
        assert(actualHeight == this->GetHeight());
        assert(this->IsValid());
    }
};

void TestTreeBasics()
{
    BinarySearchTree<int> object;
    Tree<int>& tree = object;
    assert(tree.GetCount() == 0);
    assert(tree.GetHeight() == 0);
    assert(tree.GetItems().empty());
    bool found = tree.Contains(7);
    bool removed = tree.Remove(7);
    assert(!found && !removed);

    int values[] = {5, 3, 7, 1, 4, 6, 9};
    for (int value : values)
    {
        bool inserted = tree.Insert(value);
        assert(inserted);
        assert(tree.Contains(value));
    }
    bool duplicate = tree.Insert(5);
    assert(!duplicate);
    assert(tree.GetCount() == 7);
    assert((tree.GetItems() == std::vector<int>{1, 3, 4, 5, 6, 7, 9}));
    assert((tree.GetItems(3) == std::vector<int>{1, 3, 4}));
    assert(tree.GetItems(0).empty());
    tree.Clear();
    tree.Clear();
    assert(tree.GetCount() == 0 && tree.IsValid());

    bool thrown = false;
    try { tree.GetRoot(); }
    catch (const std::out_of_range&) { thrown = true; }
    assert(thrown);
    thrown = false;
    try { tree.GetItems(-1); }
    catch (const std::invalid_argument&) { thrown = true; }
    assert(thrown);

    for (int mode = 0; mode < 3; ++mode)
    {
        thrown = false;
        try
        {
            if (mode == 0) { BinarySearchTree<int> bad(nullptr, 1); }
            if (mode == 1) { BinarySearchTree<int> bad(nullptr, -1); }
            if (mode == 2) { BinarySearchTree<int> bad(static_cast<Tree<int>::CompareFunction>(nullptr)); }
        }
        catch (const std::invalid_argument&) { thrown = true; }
        assert(thrown);
    }
}

void TestTreeBalanceAndRemoval()
{
    int rotations[][3] = {{30, 20, 10}, {10, 20, 30}, {30, 10, 20}, {10, 30, 20}};
    for (const int* values : rotations)
    {
        InspectedTree tree;
        std::set<int> expected;
        for (int i = 0; i < 3; ++i)
        {
            tree.Insert(values[i]);
            expected.insert(values[i]);
            tree.Check(expected);
        }
        assert(tree.GetRoot() == 20);
        assert(tree.GetHeight() == 2);
    }

    InspectedTree tree;
    std::set<int> expected;
    int items[] = {40, 20, 60, 10, 30, 50, 70, 25, 35, 65};
    for (int value : items)
    {
        tree.Insert(value);
        expected.insert(value);
    }
    int removals[] = {10, 70, 40, 20, 35, 65, 30, 25, 60, 50};
    for (int value : removals)
    {
        bool removed = tree.Remove(value);
        expected.erase(value);
        assert(removed);
        tree.Check(expected);
    }

    std::mt19937 random(2026);
    for (int i = 0; i < 3000; ++i)
    {
        int value = static_cast<int>(random() % 500);
        if (random() % 2 == 0)
        {
            bool actual = tree.Insert(value);
            bool correct = expected.insert(value).second;
            assert(actual == correct);
        }
        else
        {
            bool actual = tree.Remove(value);
            bool correct = expected.erase(value) != 0;
            assert(actual == correct);
        }
        tree.Check(expected);
    }

    tree.Clear();
    expected.clear();
    for (int value = 0; value < 1000; ++value)
    {
        tree.Insert(value);
        expected.insert(value);
    }
    tree.Check(expected);
    for (int value = 999; value >= 0; --value)
    {
        tree.Remove(value);
        expected.erase(value);
        tree.Check(expected);
    }
}

void TestTreeCopy()
{
    int values[] = {8, 4, 12, 2, 6, 10, 14};
    BinarySearchTree<int> tree(values, 7);
    std::string saved = tree.ToString(ValueToString<int>);
    BinarySearchTree<int> copy(tree);
    assert(copy.ToString(ValueToString<int>) == saved);
    copy.Remove(4);
    copy.Insert(100);
    assert(tree.Contains(4) && !tree.Contains(100));

    BinarySearchTree<int> assigned(Descending);
    assigned.Insert(999);
    assigned = tree;
    assert(assigned.GetCompare() == tree.GetCompare());
    assert(assigned.ToString(ValueToString<int>) == saved);
    BinarySearchTree<int>& same = assigned;
    assigned = same;
    assert(assigned.IsValid());

    std::unique_ptr<Tree<int>> clone = tree.Clone();
    clone->Clear();
    assert(tree.GetCount() == 7);
    BinarySearchTree<int> empty;
    assigned = empty;
    assert(assigned.GetCount() == 0 && assigned.IsValid());
}

void TestTreeAlgorithms()
{
    int values[] = {1, 2, 3};
    BinarySearchTree<int> tree(values, 3);
    Tree<int>& interface = tree;
    std::unique_ptr<Tree<int>> mapped = interface.Map(Negate);
    assert((mapped->GetItems() == std::vector<int>{-3, -2, -1}));
    assert(mapped->IsValid());
    std::unique_ptr<Tree<int>> collapsed = interface.Map(Zero);
    assert(collapsed->GetCount() == 1 && collapsed->Contains(0));
    std::unique_ptr<Tree<int>> even = interface.Where(Even);
    assert((even->GetItems() == std::vector<int>{2}));
    assert(interface.Where(Always)->GetCount() == 3);
    assert(interface.Where(Never)->GetCount() == 0);
    assert(interface.Reduce(Sum, 0) == 6);
    assert(interface.Reduce(LabReduce, 4) == 144);
    assert((tree.GetItems() == std::vector<int>{1, 2, 3}));

    BinarySearchTree<int> empty;
    assert(empty.Map(Negate)->GetCount() == 0);
    assert(empty.Where(Even)->GetCount() == 0);
    assert(empty.Reduce(Sum, 42) == 42);
    for (int mode = 0; mode < 3; ++mode)
    {
        bool thrown = false;
        try
        {
            if (mode == 0) { tree.Map(nullptr); }
            if (mode == 1) { tree.Where(nullptr); }
            if (mode == 2) { tree.Reduce(nullptr, 0); }
        }
        catch (const std::invalid_argument&) { thrown = true; }
        assert(thrown);
    }
}

void TestTreeSubtrees()
{
    int values[] = {8, 4, 12, 2, 6, 10, 14};
    BinarySearchTree<int> tree(values, 7);
    std::unique_ptr<Tree<int>> subtree = tree.GetSubtree(4);
    assert((subtree->GetItems() == std::vector<int>{2, 4, 6}));
    assert(tree.ContainsSubtree(*subtree));
    assert(tree.ContainsSubtree(*tree.GetSubtree(8)));
    subtree->Insert(5);
    assert(!tree.Contains(5));
    assert(!tree.ContainsSubtree(*subtree));

    BinarySearchTree<int> shape;
    shape.ReadFromString("(\"2\" # (\"4\" # #))", ValueFromString<int>);
    assert(!tree.ContainsSubtree(shape));
    BinarySearchTree<int> empty;
    assert(tree.ContainsSubtree(empty));
    assert(empty.ContainsSubtree(empty));
    assert(!empty.ContainsSubtree(tree));

    bool thrown = false;
    try { tree.GetSubtree(999); }
    catch (const std::out_of_range&) { thrown = true; }
    assert(thrown);
    BinarySearchTree<int> other(Descending);
    thrown = false;
    try { tree.ContainsSubtree(other); }
    catch (const std::invalid_argument&) { thrown = true; }
    assert(thrown);
}

static void CheckInvalidString(BinarySearchTree<int>& tree, const std::string& text)
{
    std::string before = tree.ToString(ValueToString<int>);
    bool thrown = false;
    try { tree.ReadFromString(text, ValueFromString<int>); }
    catch (const std::invalid_argument&) { thrown = true; }
    assert(thrown);
    assert(tree.ToString(ValueToString<int>) == before);
    assert(tree.IsValid());
}

void TestTreeSerialization()
{
    int values[] = {5, 3, 7};
    BinarySearchTree<int> tree(values, 3);
    std::string saved = tree.ToString(ValueToString<int>);
    assert(saved == "(\"5\" (\"3\" # #) (\"7\" # #))");
    BinarySearchTree<int> restored;
    restored.ReadFromString(saved, ValueFromString<int>);
    assert(restored.ToString(ValueToString<int>) == saved);
    assert(restored.GetCount() == 3 && restored.IsValid());
    restored.ReadFromString("  #  ", ValueFromString<int>);
    assert(restored.GetCount() == 0 && restored.ToString(ValueToString<int>) == "#");

    const char* invalid[] = {"", "# extra", "(5 # #)", "(\"x\" # #)",
        "(\"5\" #)", "(\"5\" # #", "(\"5\" # #) trailing",
        "(\"5\" (\"5\" # #) #)", "(\"5\" (\"9\" # #) #)",
        "(\"3\" (\"2\" (\"1\" # #) #) #)",
        "(\"5\" (\"3\" # (\"6\" # #)) (\"7\" # #))"};
    for (const char* text : invalid)
    {
        CheckInvalidString(tree, text);
    }
    std::string deep;
    for (int i = 0; i < 70; ++i) { deep += "(\"1\" "; }
    deep += '#';
    for (int i = 0; i < 70; ++i) { deep += " #)"; }
    CheckInvalidString(tree, deep);

    bool thrown = false;
    try { tree.ToString(nullptr); }
    catch (const std::invalid_argument&) { thrown = true; }
    assert(thrown);
    thrown = false;
    try { tree.ReadFromString(saved, nullptr); }
    catch (const std::invalid_argument&) { thrown = true; }
    assert(thrown);

    std::ostringstream printed;
    tree.Print(printed, ValueToString<int>, 2);
    assert(printed.str().find('5') != std::string::npos);
    thrown = false;
    try { tree.Print(printed, ValueToString<int>, 0); }
    catch (const std::invalid_argument&) { thrown = true; }
    assert(thrown);
}

class Record
{
public:
    int value;
    explicit Record(int value) : value(value) { }
};

static bool CompareRecords(const Record& left, const Record& right) { return left.value < right.value; }
static std::string WriteRecord(const Record& value) { return ValueToString(value.value); }
static Record ReadRecord(const std::string& text) { return Record(ValueFromString<int>(text)); }

void TestTreeTypes()
{
    BinarySearchTree<double> numbers;
    numbers.Insert(0.1);
    numbers.Insert(-1.2345678901234567);
    BinarySearchTree<double> restoredNumbers;
    restoredNumbers.ReadFromString(numbers.ToString(ValueToString<double>), ValueFromString<double>);
    assert(restoredNumbers.GetItems() == numbers.GetItems());
    bool thrown = false;
    try { numbers.Insert(std::numeric_limits<double>::quiet_NaN()); }
    catch (const std::invalid_argument&) { thrown = true; }
    assert(thrown && numbers.GetCount() == 2);

    BinarySearchTree<std::string> words;
    words.Insert(" a word ");
    words.Insert("# (quote \" and slash \\)\nnext line");
    words.Insert("");
    BinarySearchTree<std::string> restoredWords;
    restoredWords.ReadFromString(words.ToString(ValueToString<std::string>), ValueFromString<std::string>);
    assert(restoredWords.GetItems() == words.GetItems());

    BinarySearchTree<Record> records(CompareRecords);
    records.Insert(Record(2));
    records.Insert(Record(1));
    BinarySearchTree<Record> restoredRecords(CompareRecords);
    restoredRecords.ReadFromString(records.ToString(WriteRecord), ReadRecord);
    assert(restoredRecords.GetRoot().value == records.GetRoot().value);
    assert(restoredRecords.Contains(Record(1)));
    assert(restoredRecords.IsValid());

    Student student(10, "Ivan \"A\"", "Ivanovich", "Petrov", "2004-02-29");
    BinarySearchTree<Student> students(CompareStudents);
    students.Insert(student);
    bool duplicate = students.Insert(Student(10, "Another", "", "Name", "2001-01-01"));
    assert(!duplicate);
    BinarySearchTree<Student> restoredStudents(CompareStudents);
    restoredStudents.ReadFromString(students.ToString(StudentToString), StudentFromString);
    assert(StudentToString(restoredStudents.GetRoot()) == StudentToString(student));
    thrown = false;
    try { Student invalid(1, "Ivan", "", "Petrov", "2003-02-29"); }
    catch (const std::invalid_argument&) { thrown = true; }
    assert(thrown);
    thrown = false;
    try { StudentFromString("1 Ivan x Petrov 2005-01-01"); }
    catch (const std::invalid_argument&) { thrown = true; }
    assert(thrown);

    int values[] = {1, 3, 2};
    BinarySearchTree<int> reversed(values, 3, Descending);
    assert((reversed.GetItems() == std::vector<int>{3, 2, 1}));
    assert(reversed.IsValid());
    assert(reversed.Map(Negate)->GetCompare() == Descending);
}

class Tracked
{
public:
    static int alive;
    static int copiesLeft;
    int value;

    explicit Tracked(int value) : value(value) { ++alive; }
    Tracked(const Tracked& other) : value(other.value)
    {
        if (copiesLeft == 0) { throw std::runtime_error("test copy failure"); }
        if (copiesLeft > 0) { --copiesLeft; }
        ++alive;
    }
    Tracked& operator=(const Tracked& other) { this->value = other.value; return *this; }
    ~Tracked() { --alive; }
};

int Tracked::alive = 0;
int Tracked::copiesLeft = -1;
static bool CompareTracked(const Tracked& left, const Tracked& right) { return left.value < right.value; }
static Tracked ThrowingMap(const Tracked& value)
{
    if (value.value == 3) { throw std::runtime_error("test callback failure"); }
    return Tracked(value.value * 2);
}

void TestTreeLifetimeAndExceptions()
{
    assert(Tracked::alive == 0);
    {
        std::unique_ptr<Tree<Tracked>> tree = std::make_unique<BinarySearchTree<Tracked>>(CompareTracked);
        for (int i = 1; i <= 7; ++i) { tree->Insert(Tracked(i)); }
        assert(Tracked::alive == 7);
        bool thrown = false;
        try { tree->Map(ThrowingMap); }
        catch (const std::runtime_error&) { thrown = true; }
        assert(thrown && Tracked::alive == 7 && tree->GetCount() == 7);

        Tracked::copiesLeft = 2;
        thrown = false;
        try { tree->Clone(); }
        catch (const std::runtime_error&) { thrown = true; }
        Tracked::copiesLeft = -1;
        assert(thrown && Tracked::alive == 7 && tree->IsValid());

        Tracked::copiesLeft = 0;
        thrown = false;
        try { tree->Insert(Tracked(100)); }
        catch (const std::runtime_error&) { thrown = true; }
        Tracked::copiesLeft = -1;
        assert(thrown && Tracked::alive == 7 && !tree->Contains(Tracked(100)));

        bool removed = tree->Remove(tree->GetRoot());
        assert(removed && Tracked::alive == 6 && tree->IsValid());
    }
    assert(Tracked::alive == 0);
}
