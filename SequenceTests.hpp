#ifndef SEQUENCETESTS_HPP
#define SEQUENCETESTS_HPP

#include <cassert>
#include <initializer_list>

#include "Sequence.hpp"

void TestSequenceBasics();
void TestSequenceLifetimeAndExceptions();

template<typename T>
void CheckItems(const Sequence<T>& items, std::initializer_list<T> expected)
{
    assert(items.GetLength() == expected.size());
    std::size_t index = 0;
    for (const T& value : expected)
    {
        assert(items.Get(index) == value);
        ++index;
    }
}

template<typename T>
void CheckSameItems(const Sequence<T>& left, const Sequence<T>& right)
{
    assert(left.GetLength() == right.GetLength());
    for (std::size_t i = 0; i < left.GetLength(); ++i)
    {
        assert(left.Get(i) == right.Get(i));
    }
}

#endif
