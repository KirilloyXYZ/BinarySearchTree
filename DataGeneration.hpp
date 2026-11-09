#ifndef DATAGENERATION_HPP
#define DATAGENERATION_HPP

#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <utility>

#include "DynamicArray.hpp"

enum class DataOrder
{
    Random,
    Ascending,
    Descending
};

inline DynamicArray<int> GenerateKeys(std::size_t count, int minimum, int maximum,
                                     DataOrder order, unsigned int seed)
{
    if (minimum > maximum || (order != DataOrder::Random && order != DataOrder::Ascending &&
                             order != DataOrder::Descending))
    {
        throw std::invalid_argument("GenerateKeys: invalid range or order");
    }
    std::uint64_t span = static_cast<long long>(maximum) - minimum;
    if (count > span + 1) { throw std::invalid_argument("GenerateKeys: not enough distinct keys"); }

    DynamicArray<int> keys;
    keys.Reserve(count);
    for (std::size_t i = 0; i < count; ++i)
    {
        std::uint64_t offset = count > 1 ? span * i / (count - 1) : 0;
        keys.PushBack(static_cast<int>(static_cast<long long>(minimum) + static_cast<long long>(offset)));
    }
    if (order == DataOrder::Descending)
    {
        for (std::size_t i = 0; i < count / 2; ++i)
        {
            std::swap(keys.Get(i), keys.Get(count - 1 - i));
        }
    }
    else if (order == DataOrder::Random)
    {
        std::mt19937 random(seed);
        for (std::size_t i = count; i > 1; --i)
        {
            std::uniform_int_distribution<std::size_t> index(0, i - 1);
            std::swap(keys.Get(i - 1), keys.Get(index(random)));
        }
    }
    return keys;
}

#endif
