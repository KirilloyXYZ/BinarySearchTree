#ifndef TREE_HPP
#define TREE_HPP

#include <cstddef>
#include <memory>

#include "IEnumerable.hpp"

template<typename T, typename K>
class Tree : public IEnumerable<T, K>
{
public:
    virtual ~Tree() = default;

    virtual bool Insert(const K& key, const T& value) = 0;
    virtual bool Remove(const K& key) = 0;
    virtual bool Contains(const K& key) const = 0;
    virtual const T& Get(const K& key) const = 0;

    virtual std::size_t GetCount() const = 0;
    virtual int GetHeight() const = 0;
    virtual const T& GetRoot() const = 0;
    virtual const K& GetRootKey() const = 0;
    virtual void Clear() = 0;

    virtual std::unique_ptr<Tree<T, K>> Map(T (*func)(const T&)) const = 0;
    virtual std::unique_ptr<Tree<T, K>> Where(bool (*predicate)(const T&)) const = 0;
    virtual T Reduce(T (*func)(const T&, const T&), const T& startValue) const = 0;
};

#endif
