#ifndef SET_HPP
#define SET_HPP

#include <cstddef>
#include <memory>

#include "Sequence.hpp"

template<typename T>
class Set
{
public:
    virtual ~Set() = default;

    virtual bool Insert(const T& value) = 0;
    virtual bool Remove(const T& value) = 0;
    virtual bool Contains(const T& value) const = 0;
    virtual std::size_t GetCount() const = 0;
    virtual void Clear() = 0;
    virtual bool (*GetCompare() const)(const T&, const T&) = 0;
    virtual std::unique_ptr<Sequence<T>> GetItems() const = 0;
    virtual std::unique_ptr<Sequence<T>> GetItems(std::size_t limit) const = 0;

    virtual std::unique_ptr<Set<T>> Clone() const = 0;
    virtual std::unique_ptr<Set<T>> Map(T (*func)(const T&)) const = 0;
    virtual std::unique_ptr<Set<T>> Where(bool (*predicate)(const T&)) const = 0;
    virtual T Reduce(T (*func)(const T&, const T&), const T& startValue) const = 0;

    virtual std::unique_ptr<Set<T>> Union(const Set<T>& other) const = 0;
    virtual std::unique_ptr<Set<T>> Intersection(const Set<T>& other) const = 0;
    virtual std::unique_ptr<Set<T>> Difference(const Set<T>& other) const = 0;
    virtual bool IsSubsetOf(const Set<T>& other) const = 0;
    virtual bool Equals(const Set<T>& other) const = 0;
};

#endif
