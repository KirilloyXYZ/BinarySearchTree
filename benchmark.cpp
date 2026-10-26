#include <algorithm>
#include <chrono>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>

#include "BinarySearchTree.hpp"
#include "TreeSet.hpp"
#include "DynamicArray.hpp"
#include "ValueFunctions.hpp"

using Clock = std::chrono::steady_clock;

static int Negate(const int& value) { return -value; }
static bool Even(const int& value) { return value % 2 == 0; }
static int Xor(const int& value, const int& result) { return value ^ result; }

static double Elapsed(Clock::time_point start)
{
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}

static void Record(std::ostream& output, std::size_t count, const std::string& order,
                   const std::string& operation, std::size_t calls, std::size_t repeat, double milliseconds, long long checksum)
{
    output << count << ',' << order << ',' << operation << ',' << calls << ',' << repeat << ','
           << std::setprecision(12) << milliseconds << ',' << checksum << '\n';
    output.flush();
    std::cout << std::setw(20) << operation << ": "
              << std::fixed << std::setprecision(6) << milliseconds << " ms"
              << " (" << calls << " calls, " << milliseconds * 1000 / calls << " us/call)\n";
}

static void MeasureDetailed(std::ostream& output, BinarySearchTree<int>& tree,
                            std::size_t count, const std::string& order, std::size_t repeat)
{
    Clock::time_point start = Clock::now();
    int checksum = tree.Reduce(Xor, 0);
    double time = Elapsed(start);
    Record(output, count, order, "reduce", 1, repeat, time, checksum);

    start = Clock::now();
    std::unique_ptr<Tree<int>> result = tree.Clone();
    time = Elapsed(start);
    Record(output, count, order, "clone", 1, repeat, time, result->GetCount());
    result.reset();

    start = Clock::now();
    result = tree.Map(Negate);
    time = Elapsed(start);
    Record(output, count, order, "map", 1, repeat, time, result->GetCount());
    result.reset();

    start = Clock::now();
    result = tree.Where(Even);
    time = Elapsed(start);
    Record(output, count, order, "where", 1, repeat, time, result->GetCount());
    result.reset();

    int rootValue = tree.GetRoot();
    start = Clock::now();
    result = tree.GetSubtree(rootValue);
    time = Elapsed(start);
    Record(output, count, order, "get_subtree", 1, repeat, time, result->GetCount());
    start = Clock::now();
    bool contained = tree.ContainsSubtree(*result);
    time = Elapsed(start);
    Record(output, count, order, "contains_subtree", 1, repeat, time, contained);
    result.reset();

    if (!contained || !tree.IsValid())
    {
        throw std::runtime_error("benchmark: invalid detailed result");
    }

    TraversalOrder orders[] = {TraversalOrder::KLP, TraversalOrder::KPL, TraversalOrder::LKP,
        TraversalOrder::LPK, TraversalOrder::PLK, TraversalOrder::PKL};
    const char* names[] = {"enumerate_KLP", "enumerate_KPL", "enumerate_LKP",
        "enumerate_LPK", "enumerate_PLK", "enumerate_PKL"};
    for (std::size_t i = 0; i < 6; ++i)
    {
        start = Clock::now();
        std::unique_ptr<IEnumerator<int>> enumerator = tree.GetEnumerator(orders[i]);
        std::size_t visited = 0;
        int enumeratedChecksum = 0;
        while (enumerator->MoveNext())
        {
            enumeratedChecksum ^= enumerator->GetCurrent();
            ++visited;
        }
        time = Elapsed(start);
        Record(output, count, order, names[i], count, repeat, time, enumeratedChecksum);
        if (visited != count || enumeratedChecksum != checksum)
        {
            throw std::runtime_error("benchmark: invalid enumeration result");
        }
    }

    start = Clock::now();
    std::size_t visited = 0;
    int traversedChecksum = 0;
    tree.Traverse(TraversalOrder::LKP, [&visited, &traversedChecksum](const int& value)
    {
        ++visited;
        traversedChecksum ^= value;
    });
    time = Elapsed(start);
    Record(output, count, order, "traverse", count, repeat, time, traversedChecksum);
    if (visited != count || traversedChecksum != checksum)
    {
        throw std::runtime_error("benchmark: invalid Traverse result");
    }
}

static void MeasureSets(std::ostream& output, std::size_t count, const std::string& order, std::size_t repeat)
{
    TreeSet<int> a, b;
    for (std::size_t i = 0; i < count; ++i)
    {
        a.Insert(static_cast<int>(i));
        b.Insert(static_cast<int>(i + count / 2));
    }

    Clock::time_point start = Clock::now();
    std::unique_ptr<Set<int>> result = a.Union(b);
    double time = Elapsed(start);
    std::size_t unionCount = result->GetCount();
    Record(output, count, order, "set_union", 1, repeat, time, unionCount);
    start = Clock::now();
    bool included = a.IsSubsetOf(*result);
    time = Elapsed(start);
    Record(output, count, order, "set_subset", 1, repeat, time, included);
    result.reset();

    start = Clock::now();
    result = a.Intersection(b);
    time = Elapsed(start);
    std::size_t commonCount = result->GetCount();
    Record(output, count, order, "set_intersection", 1, repeat, time, commonCount);
    result.reset();

    start = Clock::now();
    result = a.Difference(b);
    time = Elapsed(start);
    Record(output, count, order, "set_difference", 1, repeat, time, result->GetCount());
    result.reset();

    start = Clock::now();
    result = a.Map(Negate);
    time = Elapsed(start);
    Record(output, count, order, "set_map", 1, repeat, time, result->GetCount());
    result.reset();
    start = Clock::now();
    result = a.Where(Even);
    time = Elapsed(start);
    Record(output, count, order, "set_where", 1, repeat, time, result->GetCount());
    result.reset();
    start = Clock::now();
    int checksum = a.Reduce(Xor, 0);
    time = Elapsed(start);
    Record(output, count, order, "set_reduce", 1, repeat, time, checksum);

    result = a.Clone();
    start = Clock::now();
    bool equal = a.Equals(*result);
    time = Elapsed(start);
    Record(output, count, order, "set_equals", 1, repeat, time, equal);
    if (!included || !equal || unionCount != count + count / 2 || commonCount != count - count / 2)
    {
        throw std::runtime_error("benchmark: invalid set result");
    }
}

static void MeasureTree(std::ostream& output, std::size_t count, const std::string& order,
                        std::size_t repeat, std::size_t detailedLimit, unsigned int seed)
{
    DynamicArray<int> keys;
    if (order == "random")
    {
        keys.Reserve(count);
        for (std::size_t i = 0; i < count; ++i) { keys.PushBack(static_cast<int>(i)); }
        std::mt19937 random(seed);
        for (std::size_t i = count; i > 1; --i)
        {
            std::uniform_int_distribution<std::size_t> distribution(0, i - 1);
            std::size_t other = distribution(random);
            std::swap(keys.Get(i - 1), keys.Get(other));
        }
    }

    BinarySearchTree<int> tree;
    Clock::time_point start = Clock::now();
    for (std::size_t i = 0; i < count; ++i)
    {
        tree.Insert(order == "random" ? keys.Get(i) : static_cast<int>(i));
    }
    double time = Elapsed(start);
    Record(output, count, order, "insert", count, repeat, time, tree.GetCount());
    if (tree.GetCount() != count)
    {
        throw std::runtime_error("benchmark: wrong node count");
    }
    keys.Clear();

    std::size_t queries = std::min<std::size_t>(count, 10000);
    DynamicArray<int> present;
    present.Reserve(queries);
    std::mt19937 random(seed);
    std::uniform_int_distribution<int> distribution(0, static_cast<int>(count - 1));
    for (std::size_t i = 0; i < queries; ++i) { present.PushBack(distribution(random)); }

    std::size_t found = 0;
    start = Clock::now();
    for (std::size_t i = 0; i < queries; ++i) { found += tree.Contains(present.Get(i)); }
    time = Elapsed(start);
    Record(output, count, order, "search_present", queries, repeat, time, found);
    if (found != queries) { throw std::runtime_error("benchmark: search failed"); }

    found = 0;
    start = Clock::now();
    for (std::size_t i = 0; i < queries; ++i) { found += tree.Contains(static_cast<int>(count + i)); }
    time = Elapsed(start);
    Record(output, count, order, "search_absent", queries, repeat, time, found);
    if (found != 0) { throw std::runtime_error("benchmark: absent key found"); }

    if (count <= detailedLimit)
    {
        MeasureDetailed(output, tree, count, order, repeat);
    }

    std::size_t removed = 0;
    start = Clock::now();
    for (std::size_t i = 0; i < queries; ++i)
    {
        int value = static_cast<int>(static_cast<unsigned long long>(i) * count / queries);
        removed += tree.Remove(value);
    }
    time = Elapsed(start);
    Record(output, count, order, "remove", queries, repeat, time, removed);
    if (removed != queries || tree.GetCount() != count - queries || !tree.IsValid())
    {
        throw std::runtime_error("benchmark: invalid tree after removal");
    }
    tree.Clear();

    if (count <= detailedLimit && order == "sorted")
    {
        MeasureSets(output, count, "sorted_sets", repeat);
    }
}

int main(int argc, char** argv)
{
    try
    {
        if (argc > 7)
        {
            throw std::invalid_argument("Usage: benchmark [maxN] [csv] [repeats] [both|sorted|random] [detailedLimit] [seed]");
        }
        std::size_t maxCount = argc > 1 ? SizeFromString(argv[1]) : 1000000;
        std::string path = argc > 2 ? argv[2] : "results/timings.csv";
        std::size_t repeats = argc > 3 ? SizeFromString(argv[3]) : 3;
        std::string order = argc > 4 ? argv[4] : "both";
        std::size_t detailedLimit = argc > 5 ? SizeFromString(argv[5]) : 1000000;
        int seedValue = argc > 6 ? ValueFromString<int>(argv[6]) : 2026;
        if (maxCount < 1 || maxCount > 100000000 || repeats < 1 || seedValue < 0 ||
            (order != "both" && order != "sorted" && order != "random"))
        {
            throw std::invalid_argument("benchmark: invalid size, repeats, order, detailed limit or seed");
        }

        std::filesystem::path parent = std::filesystem::path(path).parent_path();
        if (!parent.empty()) { std::filesystem::create_directories(parent); }
        std::ofstream output(path);
        if (!output) { throw std::runtime_error("benchmark: cannot open CSV"); }
        output.exceptions(std::ios::badbit | std::ios::failbit);
        output << "n,order,operation,calls,repeat,time_ms,checksum\n";

        DynamicArray<std::size_t> counts;
        std::size_t sizes[] = {10000, 100000, 1000000, 10000000, 100000000};
        for (std::size_t count : sizes)
        {
            if (count <= maxCount) { counts.PushBack(count); }
        }
        if (counts.GetSize() == 0 || counts.Get(counts.GetSize() - 1) != maxCount) { counts.PushBack(maxCount); }
        for (std::size_t i = 0; i < counts.GetSize(); ++i)
        {
            std::size_t count = counts.Get(i);
            for (std::size_t repeat = 0; repeat < repeats; ++repeat)
            {
                if (order == "both" || order == "sorted")
                {
                    std::cout << "Measuring sorted, N=" << count << ", repeat=" << repeat + 1 << std::endl;
                    MeasureTree(output, count, "sorted", repeat + 1, detailedLimit, seedValue);
                }
                if (order == "both" || order == "random")
                {
                    std::cout << "Measuring random, N=" << count << ", repeat=" << repeat + 1 << std::endl;
                    MeasureTree(output, count, "random", repeat + 1, detailedLimit, seedValue);
                }
            }
        }
        std::cout << "Saved " << path << "; detailed operations up to N=" << detailedLimit << '\n';
    }
    catch (const std::exception& error)
    {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
