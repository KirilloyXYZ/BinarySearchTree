#ifndef SET_HPP
#define SET_HPP

#include <cstddef>
#include <memory>
#include <ostream>
#include <stdexcept>

#include "BinarySearchTree.hpp"

template<typename T>
class Set
{
private:
    BinarySearchTree<bool, T> tree;
    bool (*compare)(const T&, const T&);

    void CheckComparator(const Set<T>& other) const;
    void AddAll(const Set<T>& other);

public:
    explicit Set(bool (*compare)(const T&, const T&) = CompareValues<T>);

    bool Insert(const T& value);
    bool Remove(const T& value);
    bool Contains(const T& value) const;
    std::size_t GetCount() const;
    bool IsEmpty() const;
    void Clear();
    void Print(std::ostream& stream, std::size_t maxItems = 100) const;

    std::unique_ptr<Set<T>> Union(const Set<T>& other) const;
    std::unique_ptr<Set<T>> Intersection(const Set<T>& other) const;
    std::unique_ptr<Set<T>> Difference(const Set<T>& other) const;
    bool IsSubsetOf(const Set<T>& other) const;
    bool Equals(const Set<T>& other) const;
};

template<typename T>
Set<T>::Set(bool (*compare)(const T&, const T&)) : tree(compare), compare(compare) { }

template<typename T>
bool Set<T>::Insert(const T& value)
{
    return this->tree.Insert(value, true);
}

template<typename T>
bool Set<T>::Remove(const T& value)
{
    return this->tree.Remove(value);
}

template<typename T>
bool Set<T>::Contains(const T& value) const
{
    return this->tree.Contains(value);
}

template<typename T>
std::size_t Set<T>::GetCount() const
{
    return this->tree.GetCount();
}

template<typename T>
bool Set<T>::IsEmpty() const
{
    return this->GetCount() == 0;
}

template<typename T>
void Set<T>::Clear()
{
    this->tree.Clear();
}

template<typename T>
void Set<T>::CheckComparator(const Set<T>& other) const
{
    if (this->compare != other.compare)
    {
        throw std::invalid_argument("Set: comparators must be the same function");
    }
}

template<typename T>
void Set<T>::AddAll(const Set<T>& other)
{
    std::unique_ptr<IEnumerator<bool, T>> enumerator = other.tree.GetEnumerator();
    while (enumerator->MoveNext()) { this->Insert(enumerator->GetCurrentKey()); }
}

template<typename T>
std::unique_ptr<Set<T>> Set<T>::Union(const Set<T>& other) const
{
    this->CheckComparator(other);
    std::unique_ptr<Set<T>> result = std::make_unique<Set<T>>(this->compare);
    result->AddAll(*this);
    result->AddAll(other);
    return result;
}

template<typename T>
std::unique_ptr<Set<T>> Set<T>::Intersection(const Set<T>& other) const
{
    this->CheckComparator(other);
    std::unique_ptr<Set<T>> result = std::make_unique<Set<T>>(this->compare);
    std::unique_ptr<IEnumerator<bool, T>> enumerator = this->tree.GetEnumerator();
    while (enumerator->MoveNext())
    {
        const T& value = enumerator->GetCurrentKey();
        if (other.Contains(value)) { result->Insert(value); }
    }
    return result;
}

template<typename T>
std::unique_ptr<Set<T>> Set<T>::Difference(const Set<T>& other) const
{
    this->CheckComparator(other);
    std::unique_ptr<Set<T>> result = std::make_unique<Set<T>>(this->compare);
    std::unique_ptr<IEnumerator<bool, T>> enumerator = this->tree.GetEnumerator();
    while (enumerator->MoveNext())
    {
        const T& value = enumerator->GetCurrentKey();
        if (!other.Contains(value)) { result->Insert(value); }
    }
    return result;
}

template<typename T>
bool Set<T>::IsSubsetOf(const Set<T>& other) const
{
    this->CheckComparator(other);
    if (this->GetCount() > other.GetCount()) { return false; }
    std::unique_ptr<IEnumerator<bool, T>> enumerator = this->tree.GetEnumerator();
    while (enumerator->MoveNext())
    {
        if (!other.Contains(enumerator->GetCurrentKey())) { return false; }
    }
    return true;
}

template<typename T>
bool Set<T>::Equals(const Set<T>& other) const
{
    this->CheckComparator(other);
    return this->GetCount() == other.GetCount() && this->IsSubsetOf(other);
}

template<typename T>
void Set<T>::Print(std::ostream& stream, std::size_t maxItems) const
{
    if (maxItems == 0) { throw std::invalid_argument("Set::Print: item limit must be positive"); }
    stream << '{';
    std::size_t shown = 0;
    std::unique_ptr<IEnumerator<bool, T>> enumerator = this->tree.GetEnumerator();
    while (shown < maxItems && enumerator->MoveNext())
    {
        if (shown > 0) { stream << ", "; }
        stream << enumerator->GetCurrentKey();
        ++shown;
    }
    if (shown < this->GetCount()) { stream << ", ..."; }
    stream << '}';
}

#endif
