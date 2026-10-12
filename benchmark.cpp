#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>

#include "BinarySearchTree.hpp"
#include "TreeSet.hpp"

using Clock = std::chrono::steady_clock;

static int Negate(const int& value) { return -value; }
static bool Even(const int& value) { return value % 2 == 0; }
static int Xor(const int& value, const int& result) { return value ^ result; }

static double Elapsed(Clock::time_point start)
{
    return std::chrono::duration<double, std::milli>(Clock::now() - start).count();
}

static void Record(std::ostream& output, int count, const std::string& order,
                   const std::string& operation, int calls, int repeat, double milliseconds, long long checksum)
{
    output << count << ',' << order << ',' << operation << ',' << calls << ',' << repeat << ','
           << std::setprecision(12) << milliseconds << ',' << checksum << '\n';
    output.flush();
}

static void MeasureDetailed(std::ostream& output, BinarySearchTree<int>& tree,
                            int count, const std::string& order, int repeat)
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

    start = Clock::now();
    std::string text = tree.ToString(ValueToString<int>);
    time = Elapsed(start);
    Record(output, count, order, "serialize", 1, repeat, time, text.size());
    BinarySearchTree<int> restored;
    start = Clock::now();
    restored.ReadFromString(text, ValueFromString<int>);
    time = Elapsed(start);
    Record(output, count, order, "deserialize", 1, repeat, time, restored.GetCount());
    if (!contained || restored.GetCount() != count || !restored.IsValid())
    {
        throw std::runtime_error("benchmark: invalid detailed result");
    }
}

static void MeasureSets(std::ostream& output, int count, const std::string& order, int repeat)
{
    TreeSet<int> a, b;
    for (int i = 0; i < count; ++i)
    {
        a.Insert(i);
        b.Insert(i + count / 2);
    }

    Clock::time_point start = Clock::now();
    std::unique_ptr<Set<int>> result = a.Union(b);
    double time = Elapsed(start);
    int unionCount = result->GetCount();
    Record(output, count, order, "set_union", 1, repeat, time, unionCount);
    start = Clock::now();
    bool included = a.IsSubsetOf(*result);
    time = Elapsed(start);
    Record(output, count, order, "set_subset", 1, repeat, time, included);
    result.reset();

    start = Clock::now();
    result = a.Intersection(b);
    time = Elapsed(start);
    int commonCount = result->GetCount();
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

static void MeasureTree(std::ostream& output, int count, const std::string& order,
                        int repeat, int detailedLimit, unsigned int seed)
{
    std::vector<int> keys;
    if (order == "random")
    {
        keys.resize(count);
        std::iota(keys.begin(), keys.end(), 0);
        std::mt19937 random(seed);
        std::shuffle(keys.begin(), keys.end(), random);
    }

    BinarySearchTree<int> tree;
    Clock::time_point start = Clock::now();
    for (int i = 0; i < count; ++i)
    {
        tree.Insert(order == "random" ? keys[i] : i);
    }
    double time = Elapsed(start);
    Record(output, count, order, "insert", count, repeat, time, tree.GetCount());
    if (tree.GetCount() != count)
    {
        throw std::runtime_error("benchmark: wrong node count");
    }
    keys.clear();
    keys.shrink_to_fit();

    int queries = std::min(count, 10000);
    std::vector<int> present(queries);
    std::mt19937 random(seed);
    std::uniform_int_distribution<int> distribution(0, count - 1);
    for (int& value : present) { value = distribution(random); }

    int found = 0;
    start = Clock::now();
    for (int value : present) { found += tree.Contains(value); }
    time = Elapsed(start);
    Record(output, count, order, "search_present", queries, repeat, time, found);
    if (found != queries) { throw std::runtime_error("benchmark: search failed"); }

    found = 0;
    start = Clock::now();
    for (int i = 0; i < queries; ++i) { found += tree.Contains(count + i); }
    time = Elapsed(start);
    Record(output, count, order, "search_absent", queries, repeat, time, found);
    if (found != 0) { throw std::runtime_error("benchmark: absent key found"); }

    if (count <= detailedLimit)
    {
        MeasureDetailed(output, tree, count, order, repeat);
    }

    int removed = 0;
    start = Clock::now();
    for (int i = 0; i < queries; ++i)
    {
        int value = static_cast<int>(static_cast<long long>(i) * count / queries);
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
        int maxCount = argc > 1 ? ValueFromString<int>(argv[1]) : 1000000;
        std::string path = argc > 2 ? argv[2] : "results/timings.csv";
        int repeats = argc > 3 ? ValueFromString<int>(argv[3]) : 3;
        std::string order = argc > 4 ? argv[4] : "both";
        int detailedLimit = argc > 5 ? ValueFromString<int>(argv[5]) : 1000000;
        int seedValue = argc > 6 ? ValueFromString<int>(argv[6]) : 2026;
        if (maxCount < 1 || maxCount > 100000000 || repeats < 1 || detailedLimit < 0 || seedValue < 0 ||
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

        std::vector<int> counts;
        int sizes[] = {10000, 100000, 1000000, 10000000, 100000000};
        for (int count : sizes)
        {
            if (count <= maxCount) { counts.push_back(count); }
        }
        if (counts.empty() || counts.back() != maxCount) { counts.push_back(maxCount); }
        for (int count : counts)
        {
            for (int repeat = 1; repeat <= repeats; ++repeat)
            {
                if (order == "both" || order == "sorted")
                {
                    std::cout << "Measuring sorted, N=" << count << ", repeat=" << repeat << std::endl;
                    MeasureTree(output, count, "sorted", repeat, detailedLimit, seedValue);
                }
                if (order == "both" || order == "random")
                {
                    std::cout << "Measuring random, N=" << count << ", repeat=" << repeat << std::endl;
                    MeasureTree(output, count, "random", repeat, detailedLimit, seedValue);
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
