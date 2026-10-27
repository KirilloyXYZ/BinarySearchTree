#ifndef TREE_HPP
#define TREE_HPP

#include <cstddef>
#include <memory>

#include "Sequence.hpp"

template<typename T>
class Tree
{
public:
    virtual ~Tree() = default;

    virtual bool Insert(const T& value) = 0;
    virtual bool Remove(const T& value) = 0;
    virtual bool Contains(const T& value) const = 0;

    virtual std::size_t GetCount() const = 0;
    virtual int GetHeight() const = 0;
    virtual const T& GetRoot() const = 0;
    virtual void Clear() = 0;
    virtual std::unique_ptr<Sequence<T>> GetItems() const = 0;
    virtual std::unique_ptr<Sequence<T>> GetItems(std::size_t limit) const = 0;

    virtual std::unique_ptr<Tree<T>> Map(T (*func)(const T&)) const = 0;
    virtual std::unique_ptr<Tree<T>> Where(bool (*predicate)(const T&)) const = 0;
    virtual T Reduce(T (*func)(const T&, const T&), const T& startValue) const = 0;
};

#endif
