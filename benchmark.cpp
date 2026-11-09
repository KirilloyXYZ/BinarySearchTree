#include <chrono>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>

#include "BinarySearchTree.hpp"
#include "DataGeneration.hpp"

template<typename Action>
double Measure(Action action)
{
    auto start = std::chrono::steady_clock::now();
    action();
    auto finish = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(finish - start).count();
}

long long ReadArgument(const char* text, long long minimum, long long maximum)
{
    std::string argument(text);
    std::size_t end = 0;
    long long value = std::stoll(argument, &end);
    if (end != argument.size() || value < minimum || value > maximum)
    {
        throw std::invalid_argument("Invalid argument range");
    }
    return value;
}

void RunBenchmark(std::ostream& output, std::size_t count, int repeats, unsigned int seed)
{
    const char* names[] = {"random", "ascending", "descending"};
    for (int order = 0; order < 3; ++order)
    {
        for (int repeat = 0; repeat < repeats; ++repeat)
        {
            unsigned int trialSeed = seed + static_cast<unsigned int>(repeat);
            DynamicArray<int> keys = GenerateKeys(count, 0, static_cast<int>(count - 1),
                                                  static_cast<DataOrder>(order), trialSeed);
            BinarySearchTree<int, int> tree;
            double insert = Measure([&]()
            {
                for (std::size_t i = 0; i < count; ++i) { tree.Insert(keys.Get(i), keys.Get(i)); }
            });
            int height = tree.GetHeight();
            std::size_t found = 0;
            double search = Measure([&]()
            {
                for (std::size_t i = 0; i < count; ++i)
                {
                    found += tree.Contains(static_cast<int>(i));
                    found += tree.Contains(static_cast<int>(count + i));
                }
            });
            std::uint64_t checksum = 0;
            std::size_t visited = 0;
            double traverse = Measure([&]()
            {
                std::unique_ptr<IEnumerator<int, int>> enumerator = tree.GetEnumerator();
                while (enumerator->MoveNext())
                {
                    checksum += enumerator->GetCurrent();
                    ++visited;
                }
            });
            std::size_t removed = 0;
            double remove = Measure([&]()
            {
                for (std::size_t i = 0; i < count; ++i) { removed += tree.Remove(keys.Get(i)); }
            });
            if (found != count || visited != count || removed != count || tree.GetCount() != 0 ||
                checksum != static_cast<std::uint64_t>(count) * (count - 1) / 2)
            {
                throw std::runtime_error("Benchmark correctness check failed");
            }
            output << count << ',' << names[order] << ',' << repeat + 1 << ',' << trialSeed << ',' << height << ','
                   << insert << ',' << search << ',' << remove << ',' << traverse << ',' << checksum << '\n';
            output.flush();
            std::cout << count << " / " << names[order] << " / " << repeat + 1 << ": "
                      << insert << " ms insert, " << search << " ms search, " << remove << " ms remove, "
                      << traverse << " ms traverse\n";
        }
    }
}

int main(int argc, char* argv[])
{
    try
    {
        if (argc > 5)
        {
            throw std::invalid_argument("Usage: lab1_bench [max_nodes] [repeats] [seed] [output.csv]");
        }
        std::size_t maximum = argc > 1 ? static_cast<std::size_t>(ReadArgument(argv[1], 1, 100000000)) : 1000000;
        int repeats = argc > 2 ? static_cast<int>(ReadArgument(argv[2], 1, 15)) : 3;
        unsigned int seed = argc > 3 ? static_cast<unsigned int>(ReadArgument(
            argv[3], 0, std::numeric_limits<unsigned int>::max())) : 17;
        std::filesystem::path path = argc > 4 ? argv[4] : "results/benchmark.csv";
        if (path.has_parent_path()) { std::filesystem::create_directories(path.parent_path()); }
        std::ofstream output(path);
        if (!output) { throw std::runtime_error("Cannot open benchmark output"); }
        output.exceptions(std::ios::badbit | std::ios::failbit);
        output << std::fixed << std::setprecision(6);
        output << "nodes,order,repeat,seed,height,insert_ms,search_ms,remove_ms,traverse_ms,checksum\n";
        std::size_t sizes[] = {10000, 30000, 100000, 300000, 1000000, 3000000, 10000000, 30000000, 100000000};
        for (std::size_t count : sizes)
        {
            if (count < maximum) { RunBenchmark(output, count, repeats, seed); }
        }
        RunBenchmark(output, maximum, repeats, seed);
        std::cout << "Saved: " << path << '\n';
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << "Benchmark failed: " << error.what() << '\n';
        return 1;
    }
}
