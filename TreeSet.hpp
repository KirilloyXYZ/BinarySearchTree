#ifndef TREESET_HPP
#define TREESET_HPP

#include "Set.hpp"
#include "BinarySearchTree.hpp"

template<typename T>
class TreeSet : public Set<T>
{
private:
    std::unique_ptr<Tree<T>> tree;

    explicit TreeSet(std::unique_ptr<Tree<T>> tree);
    void CheckCompare(const Set<T>& other) const;

public:
    explicit TreeSet(bool (*compare)(const T&, const T&) = CompareValues<T>);
    TreeSet(const T* items, std::size_t count, bool (*compare)(const T&, const T&) = CompareValues<T>);
    TreeSet(const TreeSet<T>& other);
    TreeSet<T>& operator=(const TreeSet<T>& other);
    ~TreeSet() override = default;

    bool Insert(const T& value) override;
    bool Remove(const T& value) override;
    bool Contains(const T& value) const override;
    std::size_t GetCount() const override;
    void Clear() override;
    bool (*GetCompare() const)(const T&, const T&) override;
    std::unique_ptr<Sequence<T>> GetItems() const override;
    std::unique_ptr<Sequence<T>> GetItems(std::size_t limit) const override;

    std::unique_ptr<Set<T>> Clone() const override;
    std::unique_ptr<Set<T>> Map(T (*func)(const T&)) const override;
    std::unique_ptr<Set<T>> Where(bool (*predicate)(const T&)) const override;
    T Reduce(T (*func)(const T&, const T&), const T& startValue) const override;

    std::unique_ptr<Set<T>> Union(const Set<T>& other) const override;
    std::unique_ptr<Set<T>> Intersection(const Set<T>& other) const override;
    std::unique_ptr<Set<T>> Difference(const Set<T>& other) const override;
    bool IsSubsetOf(const Set<T>& other) const override;
    bool Equals(const Set<T>& other) const override;
};

template<typename T>
TreeSet<T>::TreeSet(std::unique_ptr<Tree<T>> tree) : tree(std::move(tree))
{
    if (this->tree == nullptr)
    {
        throw std::invalid_argument("TreeSet: null tree");
    }
}

template<typename T>
TreeSet<T>::TreeSet(bool (*compare)(const T&, const T&))
    : tree(std::make_unique<BinarySearchTree<T>>(compare)) { }

template<typename T>
TreeSet<T>::TreeSet(const T* items, std::size_t count, bool (*compare)(const T&, const T&))
    : tree(std::make_unique<BinarySearchTree<T>>(items, count, compare)) { }

template<typename T>
TreeSet<T>::TreeSet(const TreeSet<T>& other) : tree(other.tree->Clone()) { }

template<typename T>
TreeSet<T>& TreeSet<T>::operator=(const TreeSet<T>& other)
{
    if (this != &other)
    {
        this->tree = other.tree->Clone();
    }
    return *this;
}

template<typename T>
bool TreeSet<T>::Insert(const T& value)
{
    return this->tree->Insert(value);
}

template<typename T>
bool TreeSet<T>::Remove(const T& value)
{
    return this->tree->Remove(value);
}

template<typename T>
bool TreeSet<T>::Contains(const T& value) const
{
    return this->tree->Contains(value);
}

template<typename T>
std::size_t TreeSet<T>::GetCount() const
{
    return this->tree->GetCount();
}

template<typename T>
void TreeSet<T>::Clear()
{
    this->tree->Clear();
}

template<typename T>
bool (*TreeSet<T>::GetCompare() const)(const T&, const T&)
{
    return this->tree->GetCompare();
}

template<typename T>
std::unique_ptr<Sequence<T>> TreeSet<T>::GetItems() const
{
    return this->tree->GetItems();
}

template<typename T>
std::unique_ptr<Sequence<T>> TreeSet<T>::GetItems(std::size_t limit) const
{
    return this->tree->GetItems(limit);
}

template<typename T>
std::unique_ptr<Set<T>> TreeSet<T>::Clone() const
{
    return std::unique_ptr<Set<T>>(new TreeSet<T>(this->tree->Clone()));
}

template<typename T>
std::unique_ptr<Set<T>> TreeSet<T>::Map(T (*func)(const T&)) const
{
    return std::unique_ptr<Set<T>>(new TreeSet<T>(this->tree->Map(func)));
}

template<typename T>
std::unique_ptr<Set<T>> TreeSet<T>::Where(bool (*predicate)(const T&)) const
{
    return std::unique_ptr<Set<T>>(new TreeSet<T>(this->tree->Where(predicate)));
}

template<typename T>
T TreeSet<T>::Reduce(T (*func)(const T&, const T&), const T& startValue) const
{
    return this->tree->Reduce(func, startValue);
}

template<typename T>
void TreeSet<T>::CheckCompare(const Set<T>& other) const
{
    if (this->GetCompare() != other.GetCompare())
    {
        throw std::invalid_argument("TreeSet: operations require the same comparator");
    }
}

template<typename T>
std::unique_ptr<Set<T>> TreeSet<T>::Union(const Set<T>& other) const
{
    this->CheckCompare(other);
    std::unique_ptr<Set<T>> result = this->Clone();
    std::unique_ptr<Sequence<T>> items = other.GetItems();
    for (std::size_t i = 0; i < items->GetLength(); ++i)
    {
        result->Insert(items->Get(i));
    }
    return result;
}

template<typename T>
std::unique_ptr<Set<T>> TreeSet<T>::Intersection(const Set<T>& other) const
{
    this->CheckCompare(other);
    std::unique_ptr<Set<T>> result = std::make_unique<TreeSet<T>>(this->GetCompare());
    std::unique_ptr<Sequence<T>> items = this->GetItems();
    for (std::size_t i = 0; i < items->GetLength(); ++i)
    {
        const T& value = items->Get(i);
        if (other.Contains(value))
        {
            result->Insert(value);
        }
    }
    return result;
}

template<typename T>
std::unique_ptr<Set<T>> TreeSet<T>::Difference(const Set<T>& other) const
{
    this->CheckCompare(other);
    std::unique_ptr<Set<T>> result = std::make_unique<TreeSet<T>>(this->GetCompare());
    std::unique_ptr<Sequence<T>> items = this->GetItems();
    for (std::size_t i = 0; i < items->GetLength(); ++i)
    {
        const T& value = items->Get(i);
        if (!other.Contains(value))
        {
            result->Insert(value);
        }
    }
    return result;
}

template<typename T>
bool TreeSet<T>::IsSubsetOf(const Set<T>& other) const
{
    this->CheckCompare(other);
    if (this->GetCount() > other.GetCount())
    {
        return false;
    }

    std::unique_ptr<Sequence<T>> items = this->GetItems();
    for (std::size_t i = 0; i < items->GetLength(); ++i)
    {
        if (!other.Contains(items->Get(i)))
        {
            return false;
        }
    }
    return true;
}

template<typename T>
bool TreeSet<T>::Equals(const Set<T>& other) const
{
    this->CheckCompare(other);
    return this->GetCount() == other.GetCount() && this->IsSubsetOf(other);
}

#endif
