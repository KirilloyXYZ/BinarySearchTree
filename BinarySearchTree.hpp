#ifndef BINARYSEARCHTREE_HPP
#define BINARYSEARCHTREE_HPP

#include <algorithm>
#include <cstddef>
#include <memory>
#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "Tree.hpp"

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
    void PrintNode(const Node* node, std::ostream& output, std::string (*write)(const T&), std::size_t depth, std::size_t maxDepth) const;

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

    void Print(std::ostream& output, std::string (*write)(const T&), std::size_t maxDepth = 5) const override;
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
void BinarySearchTree<T>::PrintNode(const Node* node, std::ostream& output, std::string (*write)(const T&), std::size_t depth, std::size_t maxDepth) const
{
    if (node == nullptr)
    {
        return;
    }
    if (depth == maxDepth)
    {
        output << std::string(depth * 4, ' ') << "...\n";
        return;
    }

    this->PrintNode(node->right.get(), output, write, depth + 1, maxDepth);
    output << std::string(depth * 4, ' ') << write(node->data) << '\n';
    this->PrintNode(node->left.get(), output, write, depth + 1, maxDepth);
}

template<typename T>
void BinarySearchTree<T>::Print(std::ostream& output, std::string (*write)(const T&), std::size_t maxDepth) const
{
    if (write == nullptr || maxDepth < 1 || maxDepth > 10)
    {
        throw std::invalid_argument("BinarySearchTree::Print: null writer or depth outside 1..10");
    }
    if (this->root == nullptr)
    {
        output << "<empty>\n";
        return;
    }

    this->PrintNode(this->root.get(), output, write, 0, maxDepth);
}

#endif
