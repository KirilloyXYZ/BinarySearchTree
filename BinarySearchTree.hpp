#ifndef BINARYSEARCHTREE_HPP
#define BINARYSEARCHTREE_HPP

#include "Tree.hpp"
#include "ValueFunctions.hpp"

template<typename T>
class BinarySearchTree : public Tree<T>
{
private:
    using Node = typename Tree<T>::Node;

    int NodeHeight(const Node* node) const;
    void UpdateHeight(Node* node);
    int GetBalance(const Node* node) const;
    void RotateLeft(std::unique_ptr<Node>& node);
    void RotateRight(std::unique_ptr<Node>& node);
    void Balance(std::unique_ptr<Node>& node);
    bool InsertNode(std::unique_ptr<Node>& node, const T& value);
    bool RemoveNode(std::unique_ptr<Node>& node, const T& value);
    std::unique_ptr<Node> DetachMin(std::unique_ptr<Node>& node);

protected:
    std::unique_ptr<Tree<T>> CreateEmpty() const override;

public:
    explicit BinarySearchTree(typename Tree<T>::CompareFunction compare = CompareValues<T>);
    BinarySearchTree(const T* items, int count, typename Tree<T>::CompareFunction compare = CompareValues<T>);
    BinarySearchTree(const BinarySearchTree<T>& other);
    BinarySearchTree<T>& operator=(const BinarySearchTree<T>& other);
    ~BinarySearchTree() override = default;

    bool Insert(const T& value) override;
    bool Remove(const T& value) override;
    bool Contains(const T& value) const override;
};

template<typename T>
BinarySearchTree<T>::BinarySearchTree(typename Tree<T>::CompareFunction compare) : Tree<T>(compare) { }

template<typename T>
BinarySearchTree<T>::BinarySearchTree(const T* items, int count, typename Tree<T>::CompareFunction compare) : Tree<T>(compare)
{
    if (count < 0 || (items == nullptr && count > 0))
    {
        throw std::invalid_argument("BinarySearchTree: negative count or null array");
    }

    for (int i = 0; i < count; ++i)
    {
        this->Insert(items[i]);
    }
}

template<typename T>
BinarySearchTree<T>::BinarySearchTree(const BinarySearchTree<T>& other) : Tree<T>(other) { }

template<typename T>
BinarySearchTree<T>& BinarySearchTree<T>::operator=(const BinarySearchTree<T>& other)
{
    if (this != &other)
    {
        std::unique_ptr<Node> newRoot = this->CloneNode(other.root.get());
        this->root = std::move(newRoot);
        this->count = other.count;
        this->compare = other.compare;
    }

    return *this;
}

template<typename T>
std::unique_ptr<Tree<T>> BinarySearchTree<T>::CreateEmpty() const
{
    return std::make_unique<BinarySearchTree<T>>(this->compare);
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
    if (this->count == INT_MAX)
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
        // Keep the old owner alive while moving its child pointers.
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

#endif
