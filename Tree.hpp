#ifndef TREE_HPP
#define TREE_HPP

#include <cstddef>
#include <iosfwd>
#include <string>

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

    virtual void Print(std::ostream& output, std::string (*write)(const T&), std::size_t maxDepth = 5) const = 0;
};

#endif
