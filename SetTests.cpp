#include "SetTests.hpp"
#include "SequenceTests.hpp"

#include <cassert>

#include "TreeSet.hpp"
#include "Student.hpp"

static int Negate(const int& value) { return -value; }
static int Zero(const int&) { return 0; }
static bool Even(const int& value) { return value % 2 == 0; }
static int Sum(const int& value, const int& result) { return value + result; }
static int LabReduce(const int& value, const int& result) { return 2 * value + 3 * result; }
static bool Descending(const int& left, const int& right) { return left > right; }

void TestSetBasics()
{
    TreeSet<int> object;
    Set<int>& set = object;
    bool inserted = set.Insert(5);
    bool duplicate = set.Insert(5);
    assert(inserted && !duplicate && set.GetCount() == 1);
    CheckItems(*set.GetItems(0), {});
    CheckItems(*set.GetItems(1), {5});
    CheckSameItems(*set.GetItems(100), *set.GetItems());
    bool missing = set.Remove(999);
    assert(!missing && set.Contains(5));
    TreeSet<int> copy(object);
    copy.Insert(3);
    assert(object.GetCount() == 1 && copy.GetCount() == 2);
    TreeSet<int> assigned;
    assigned = copy;
    assigned.Remove(5);
    assert(copy.Contains(5));
    TreeSet<int>& same = assigned;
    assigned = same;
    assert(assigned.Contains(3));
    std::unique_ptr<Set<int>> clone = set.Clone();
    set.Clear();
    assert(set.GetCount() == 0 && clone->Contains(5));
}

void TestSetOperations()
{
    int left[] = {1, 2, 3, 3};
    int right[] = {3, 4};
    TreeSet<int> a(left, 4);
    TreeSet<int> b(right, 2);
    std::unique_ptr<Set<int>> united = a.Union(b);
    std::unique_ptr<Set<int>> common = a.Intersection(b);
    std::unique_ptr<Set<int>> difference = a.Difference(b);
    CheckItems(*united->GetItems(), {1, 2, 3, 4});
    CheckItems(*common->GetItems(), {3});
    CheckItems(*difference->GetItems(), {1, 2});
    assert(a.GetCount() == 3 && b.GetCount() == 2);
    assert(a.IsSubsetOf(*united));
    assert(!a.IsSubsetOf(b));
    assert(a.Union(a)->Equals(a));
    assert(a.Intersection(a)->Equals(a));
    assert(a.Difference(a)->GetCount() == 0);

    TreeSet<int> empty;
    assert(empty.IsSubsetOf(a));
    assert(!a.IsSubsetOf(empty));
    assert(empty.Equals(empty));
    assert(a.Union(empty)->Equals(a));
    assert(a.Intersection(empty)->GetCount() == 0);
    assert(a.Difference(empty)->Equals(a));
    assert(empty.Difference(a)->GetCount() == 0);

    int differentOrder[] = {3, 2, 1};
    TreeSet<int> same(differentOrder, 3);
    assert(a.Equals(same));
    TreeSet<int> disjoint;
    disjoint.Insert(100);
    assert(a.Intersection(disjoint)->GetCount() == 0);
}

void TestSetAlgorithmsAndTypes()
{
    int values[] = {1, 2, 3};
    TreeSet<int> object(values, 3);
    Set<int>& set = object;
    CheckItems(*set.Map(Negate)->GetItems(), {-3, -2, -1});
    assert(set.Map(Zero)->GetCount() == 1);
    CheckItems(*set.Where(Even)->GetItems(), {2});
    assert(set.Reduce(Sum, 0) == 6);
    assert(set.Reduce(LabReduce, 4) == 144);
    TreeSet<int> empty;
    assert(empty.Reduce(Sum, 17) == 17);

    TreeSet<std::string> words;
    words.Insert("hello");
    words.Insert("hello");
    words.Insert("world");
    assert(words.GetCount() == 2);
    TreeSet<double> numbers;
    numbers.Insert(1.25);
    numbers.Insert(-0.5);
    assert(numbers.Contains(1.25));

    TreeSet<Student> students(CompareStudents);
    students.Insert(Student(2, "Ivan", "", "Petrov", "2004-01-01"));
    students.Insert(Student(2, "Petr", "", "Ivanov", "2005-01-01"));
    assert(students.GetCount() == 1);
    assert(students.GetItems()->Get(0).GetFirstName() == "Ivan");
    std::unique_ptr<Set<Student>> clone = students.Clone();
    assert(clone->Equals(students));
}

void TestSetErrors()
{
    TreeSet<int> set;
    TreeSet<int> reversed(Descending);
    for (int mode = 0; mode < 8; ++mode)
    {
        bool thrown = false;
        try
        {
            if (mode == 0) { set.Map(nullptr); }
            if (mode == 1) { set.Where(nullptr); }
            if (mode == 2) { set.Reduce(nullptr, 0); }
            if (mode == 3) { set.Union(reversed); }
            if (mode == 4) { set.Intersection(reversed); }
            if (mode == 5) { set.Difference(reversed); }
            if (mode == 6) { set.IsSubsetOf(reversed); }
            if (mode == 7) { set.Equals(reversed); }
        }
        catch (const std::invalid_argument&) { thrown = true; }
        assert(thrown);
    }
}
