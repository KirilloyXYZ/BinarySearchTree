#ifndef BINARYSEARCHTREE_HPP
#define BINARYSEARCHTREE_HPP

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>

#include "Tree.hpp"
#include "ArraySequence.hpp"

template<typename T>
bool CompareValues(const T& left, const T& right)
{
    return left < right;
}

template<typename T>
class BinarySearchTree : public Tree<T>
{
private:
    class Node
    {
    public:
        T data;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        int height;

        Node(const T& value) : data(value), height(1) { }
    };

    std::unique_ptr<Node> root;
    std::size_t count;
    bool (*compare)(const T&, const T&);

    const Node* FindNode(const T& value) const;
    void CollectItems(const Node* node, Sequence<T>& items, std::size_t limit) const;
    void MapNode(const Node* node, Tree<T>& result, T (*func)(const T&)) const;
    void WhereNode(const Node* node, Tree<T>& result, bool (*predicate)(const T&)) const;
    void ReduceNode(const Node* node, T& result, T (*func)(const T&, const T&)) const;

    int NodeHeight(const Node* node) const;
    void UpdateHeight(Node* node);
    int GetBalance(const Node* node) const;
    void RotateLeft(std::unique_ptr<Node>& node);
    void RotateRight(std::unique_ptr<Node>& node);
    void Balance(std::unique_ptr<Node>& node);
    bool InsertNode(std::unique_ptr<Node>& node, const T& value);
    bool RemoveNode(std::unique_ptr<Node>& node, const T& value);
    std::unique_ptr<Node> DetachMin(std::unique_ptr<Node>& node);

public:
    explicit BinarySearchTree(bool (*compare)(const T&, const T&) = CompareValues<T>);
    ~BinarySearchTree() override = default;

    bool Insert(const T& value) override;
    bool Remove(const T& value) override;
    bool Contains(const T& value) const override;

    std::size_t GetCount() const override;
    int GetHeight() const override;
    const T& GetRoot() const override;
    void Clear() override;
    std::unique_ptr<Sequence<T>> GetItems() const override;
    std::unique_ptr<Sequence<T>> GetItems(std::size_t limit) const override;

    std::unique_ptr<Tree<T>> Map(T (*func)(const T&)) const override;
    std::unique_ptr<Tree<T>> Where(bool (*predicate)(const T&)) const override;
    T Reduce(T (*func)(const T&, const T&), const T& startValue) const override;
};

template<typename T>
BinarySearchTree<T>::BinarySearchTree(bool (*compare)(const T&, const T&)) : count(0), compare(compare)
{
    if (compare == nullptr)
    {
        throw std::invalid_argument("BinarySearchTree: null comparator");
    }
}

template<typename T>
int BinarySearchTree<T>::NodeHeight(const Node* node) const
{
    return node == nullptr ? 0 : node->height;
}

template<typename T>
void BinarySearchTree<T>::UpdateHeight(Node* node)
{
    node->height = 1 + std::max(this->NodeHeight(node->left.get()), this->NodeHeight(node->right.get()));
}

template<typename T>
int BinarySearchTree<T>::GetBalance(const Node* node) const
{
    return this->NodeHeight(node->right.get()) - this->NodeHeight(node->left.get());
}

template<typename T>
void BinarySearchTree<T>::RotateLeft(std::unique_ptr<Node>& node)
{
    std::unique_ptr<Node> newRoot = std::move(node->right);
    node->right = std::move(newRoot->left);
    this->UpdateHeight(node.get());
    newRoot->left = std::move(node);
    this->UpdateHeight(newRoot.get());
    node = std::move(newRoot);
}

template<typename T>
void BinarySearchTree<T>::RotateRight(std::unique_ptr<Node>& node)
{
    std::unique_ptr<Node> newRoot = std::move(node->left);
    node->left = std::move(newRoot->right);
    this->UpdateHeight(node.get());
    newRoot->right = std::move(node);
    this->UpdateHeight(newRoot.get());
    node = std::move(newRoot);
}

template<typename T>
void BinarySearchTree<T>::Balance(std::unique_ptr<Node>& node)
{
    this->UpdateHeight(node.get());
    if (this->GetBalance(node.get()) > 1)
    {
        if (this->GetBalance(node->right.get()) < 0)
        {
            this->RotateRight(node->right);
        }
        this->RotateLeft(node);
    }
    else if (this->GetBalance(node.get()) < -1)
    {
        if (this->GetBalance(node->left.get()) > 0)
        {
            this->RotateLeft(node->left);
        }
        this->RotateRight(node);
    }
}

template<typename T>
bool BinarySearchTree<T>::InsertNode(std::unique_ptr<Node>& node, const T& value)
{
    if (node == nullptr)
    {
        node = std::make_unique<Node>(value);
        return true;
    }

    bool inserted;
    if (this->compare(value, node->data))
    {
        inserted = this->InsertNode(node->left, value);
    }
    else if (this->compare(node->data, value))
    {
        inserted = this->InsertNode(node->right, value);
    }
    else
    {
        return false;
    }

    if (inserted)
    {
        this->Balance(node);
    }
    return inserted;
}

template<typename T>
bool BinarySearchTree<T>::Insert(const T& value)
{
    if (this->compare(value, value))
    {
        throw std::invalid_argument("BinarySearchTree::Insert: comparator must be strict");
    }
    if (this->count + 1 == 0)
    {
        throw std::length_error("BinarySearchTree::Insert: too many nodes");
    }

    bool inserted = this->InsertNode(this->root, value);
    if (inserted)
    {
        ++this->count;
    }
    return inserted;
}

template<typename T>
std::unique_ptr<typename BinarySearchTree<T>::Node> BinarySearchTree<T>::DetachMin(std::unique_ptr<Node>& node)
{
    if (node->left == nullptr)
    {
        std::unique_ptr<Node> result = std::move(node);
        node = std::move(result->right);
        return result;
    }

    std::unique_ptr<Node> result = this->DetachMin(node->left);
    this->Balance(node);
    return result;
}

template<typename T>
bool BinarySearchTree<T>::RemoveNode(std::unique_ptr<Node>& node, const T& value)
{
    if (node == nullptr)
    {
        return false;
    }

    bool removed;
    if (this->compare(value, node->data))
    {
        removed = this->RemoveNode(node->left, value);
    }
    else if (this->compare(node->data, value))
    {
        removed = this->RemoveNode(node->right, value);
    }
    else
    {
        
        std::unique_ptr<Node> oldRoot = std::move(node);
        if (oldRoot->left == nullptr)
        {
            node = std::move(oldRoot->right);
        }
        else if (oldRoot->right == nullptr)
        {
            node = std::move(oldRoot->left);
        }
        else
        {
            node = this->DetachMin(oldRoot->right);
            node->left = std::move(oldRoot->left);
            node->right = std::move(oldRoot->right);
        }
        removed = true;
    }

    if (removed && node != nullptr)
    {
        this->Balance(node);
    }
    return removed;
}

template<typename T>
bool BinarySearchTree<T>::Remove(const T& value)
{
    bool removed = this->RemoveNode(this->root, value);
    if (removed)
    {
        --this->count;
    }
    return removed;
}

template<typename T>
bool BinarySearchTree<T>::Contains(const T& value) const
{
    return this->FindNode(value) != nullptr;
}

template<typename T>
std::size_t BinarySearchTree<T>::GetCount() const
{
    return this->count;
}

template<typename T>
int BinarySearchTree<T>::GetHeight() const
{
    return this->root == nullptr ? 0 : this->root->height;
}

template<typename T>
const T& BinarySearchTree<T>::GetRoot() const
{
    if (this->root == nullptr)
    {
        throw std::out_of_range("BinarySearchTree::GetRoot: tree is empty");
    }

    return this->root->data;
}

template<typename T>
void BinarySearchTree<T>::Clear()
{
    this->root.reset();
    this->count = 0;
}

template<typename T>
const typename BinarySearchTree<T>::Node* BinarySearchTree<T>::FindNode(const T& value) const
{
    const Node* current = this->root.get();
    while (current != nullptr)
    {
        if (this->compare(value, current->data))
        {
            current = current->left.get();
        }
        else if (this->compare(current->data, value))
        {
            current = current->right.get();
        }
        else
        {
            return current;
        }
    }

    return nullptr;
}

template<typename T>
void BinarySearchTree<T>::CollectItems(const Node* node, Sequence<T>& items, std::size_t limit) const
{
    if (node == nullptr || items.GetLength() >= limit)
    {
        return;
    }

    this->CollectItems(node->left.get(), items, limit);
    if (items.GetLength() < limit)
    {
        items.Append(node->data);
        this->CollectItems(node->right.get(), items, limit);
    }
}

template<typename T>
std::unique_ptr<Sequence<T>> BinarySearchTree<T>::GetItems() const
{
    return this->GetItems(this->count);
}

template<typename T>
std::unique_ptr<Sequence<T>> BinarySearchTree<T>::GetItems(std::size_t limit) const
{
    std::unique_ptr<Sequence<T>> result = std::make_unique<ArraySequence<T>>(std::min(this->count, limit));
    this->CollectItems(this->root.get(), *result, limit);
    return result;
}

template<typename T>
void BinarySearchTree<T>::MapNode(const Node* node, Tree<T>& result, T (*func)(const T&)) const
{
    if (node == nullptr)
    {
        return;
    }

    this->MapNode(node->left.get(), result, func);
    result.Insert(func(node->data));
    this->MapNode(node->right.get(), result, func);
}

template<typename T>
std::unique_ptr<Tree<T>> BinarySearchTree<T>::Map(T (*func)(const T&)) const
{
    if (func == nullptr)
    {
        throw std::invalid_argument("BinarySearchTree::Map: null function");
    }

    std::unique_ptr<Tree<T>> result = std::make_unique<BinarySearchTree<T>>(this->compare);
    this->MapNode(this->root.get(), *result, func);
    return result;
}

template<typename T>
void BinarySearchTree<T>::WhereNode(const Node* node, Tree<T>& result, bool (*predicate)(const T&)) const
{
    if (node == nullptr)
    {
        return;
    }

    this->WhereNode(node->left.get(), result, predicate);
    if (predicate(node->data))
    {
        result.Insert(node->data);
    }
    this->WhereNode(node->right.get(), result, predicate);
}

template<typename T>
std::unique_ptr<Tree<T>> BinarySearchTree<T>::Where(bool (*predicate)(const T&)) const
{
    if (predicate == nullptr)
    {
        throw std::invalid_argument("BinarySearchTree::Where: null predicate");
    }

    std::unique_ptr<Tree<T>> result = std::make_unique<BinarySearchTree<T>>(this->compare);
    this->WhereNode(this->root.get(), *result, predicate);
    return result;
}

template<typename T>
void BinarySearchTree<T>::ReduceNode(const Node* node, T& result, T (*func)(const T&, const T&)) const
{
    if (node == nullptr)
    {
        return;
    }

    this->ReduceNode(node->left.get(), result, func);
    result = func(node->data, result);
    this->ReduceNode(node->right.get(), result, func);
}

template<typename T>
T BinarySearchTree<T>::Reduce(T (*func)(const T&, const T&), const T& startValue) const
{
    if (func == nullptr)
    {
        throw std::invalid_argument("BinarySearchTree::Reduce: null function");
    }

    T result = startValue;
    this->ReduceNode(this->root.get(), result, func);
    return result;
}

#endif
