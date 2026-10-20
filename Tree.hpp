#ifndef TREE_HPP
#define TREE_HPP

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>

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
    virtual bool (*GetCompare() const)(const T&, const T&) = 0;
    virtual const T& GetRoot() const = 0;
    virtual void Clear() = 0;
    virtual bool IsValid() const = 0;
    virtual std::unique_ptr<Sequence<T>> GetItems() const = 0;
    virtual std::unique_ptr<Sequence<T>> GetItems(std::size_t limit) const = 0;

    virtual std::unique_ptr<Sequence<const T*>> GetStructure() const = 0;

    virtual std::unique_ptr<Tree<T>> Clone() const = 0;
    virtual std::unique_ptr<Tree<T>> Map(T (*func)(const T&)) const = 0;
    virtual std::unique_ptr<Tree<T>> Where(bool (*predicate)(const T&)) const = 0;
    virtual T Reduce(T (*func)(const T&, const T&), const T& startValue) const = 0;
    virtual std::unique_ptr<Tree<T>> GetSubtree(const T& value) const = 0;
    virtual bool ContainsSubtree(const Tree<T>& other) const = 0;

    virtual std::string ToString(std::string (*write)(const T&)) const = 0;
    virtual void ReadFromString(const std::string& text, T (*read)(const std::string&)) = 0;
    virtual void Print(std::ostream& output, std::string (*write)(const T&), std::size_t maxDepth = 5) const = 0;
};

#endif
