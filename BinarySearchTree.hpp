#ifndef BINARYSEARCHTREE_HPP
#define BINARYSEARCHTREE_HPP

#include <algorithm>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "Tree.hpp"
#include "Stack.hpp"

template<typename T>
bool CompareValues(const T& left, const T& right)
{
    return left < right;
}

template<typename T, typename K>
class BinarySearchTree : public Tree<T, K>
{
private:
    class Node
    {
    public:
        K key;
        T data;
        std::unique_ptr<Node> left;
        std::unique_ptr<Node> right;
        int height;

        Node(const K& key, const T& value) : key(key), data(value), height(1) { }
    };

    std::unique_ptr<Node> root;
    std::size_t count;
    bool (*compare)(const K&, const K&);
    std::size_t version;

    const Node* FindNode(const K& key) const;

    int NodeHeight(const Node* node) const;
    void UpdateHeight(Node* node);
    int GetBalance(const Node* node) const;
    void RotateLeft(std::unique_ptr<Node>& node);
    void RotateRight(std::unique_ptr<Node>& node);
    void Balance(std::unique_ptr<Node>& node);
    bool InsertNode(std::unique_ptr<Node>& node, const K& key, const T& value);
    bool RemoveNode(std::unique_ptr<Node>& node, const K& key);
    std::unique_ptr<Node> DetachMin(std::unique_ptr<Node>& node);

public:
    class Enumerator : public IEnumerator<T, K>
    {
    private:
        class Frame
        {
        public:
            const Node* node;
            std::size_t nextStep;

            explicit Frame(const Node* node) : node(node), nextStep(0) { }
        };

        Stack<Frame> frames;
        const Node* current;
        std::string steps;
        const BinarySearchTree<T, K>& tree;
        std::size_t expectedVersion;

        void CheckVersion() const;
        void CheckCurrent() const;

    public:
        // The tree must outlive this enumerator.
        explicit Enumerator(const BinarySearchTree<T, K>& tree, TraversalOrder order = TraversalOrder::LKP);

        bool MoveNext() override;
        const T& GetCurrent() const override;
        const K& GetCurrentKey() const override;
    };

    explicit BinarySearchTree(bool (*compare)(const K&, const K&) = CompareValues<K>);
    ~BinarySearchTree() override = default;

    bool Insert(const K& key, const T& value) override;
    bool Remove(const K& key) override;
    bool Contains(const K& key) const override;
    const T& Get(const K& key) const override;

    std::size_t GetCount() const override;
    int GetHeight() const override;
    const T& GetRoot() const override;
    const K& GetRootKey() const override;
    void Clear() override;
    std::unique_ptr<IEnumerator<T, K>> GetEnumerator(TraversalOrder order = TraversalOrder::LKP) const override;

    std::unique_ptr<Tree<T, K>> Map(T (*func)(const T&)) const override;
    std::unique_ptr<Tree<T, K>> Where(bool (*predicate)(const T&)) const override;
    T Reduce(T (*func)(const T&, const T&), const T& startValue) const override;
};

template<typename T, typename K>
BinarySearchTree<T, K>::BinarySearchTree(bool (*compare)(const K&, const K&))
    : count(0), compare(compare), version(0)
{
    if (compare == nullptr)
    {
        throw std::invalid_argument("BinarySearchTree: null comparator");
    }
}

template<typename T, typename K>
int BinarySearchTree<T, K>::NodeHeight(const Node* node) const
{
    return node == nullptr ? 0 : node->height;
}

template<typename T, typename K>
void BinarySearchTree<T, K>::UpdateHeight(Node* node)
{
    node->height = 1 + std::max(this->NodeHeight(node->left.get()), this->NodeHeight(node->right.get()));
}

template<typename T, typename K>
int BinarySearchTree<T, K>::GetBalance(const Node* node) const
{
    return this->NodeHeight(node->right.get()) - this->NodeHeight(node->left.get());
}

template<typename T, typename K>
void BinarySearchTree<T, K>::RotateLeft(std::unique_ptr<Node>& node)
{
    std::unique_ptr<Node> newRoot = std::move(node->right);
    node->right = std::move(newRoot->left);
    this->UpdateHeight(node.get());
    newRoot->left = std::move(node);
    this->UpdateHeight(newRoot.get());
    node = std::move(newRoot);
}

template<typename T, typename K>
void BinarySearchTree<T, K>::RotateRight(std::unique_ptr<Node>& node)
{
    std::unique_ptr<Node> newRoot = std::move(node->left);
    node->left = std::move(newRoot->right);
    this->UpdateHeight(node.get());
    newRoot->right = std::move(node);
    this->UpdateHeight(newRoot.get());
    node = std::move(newRoot);
}

template<typename T, typename K>
void BinarySearchTree<T, K>::Balance(std::unique_ptr<Node>& node)
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

template<typename T, typename K>
bool BinarySearchTree<T, K>::InsertNode(std::unique_ptr<Node>& node, const K& key, const T& value)
{
    if (node == nullptr)
    {
        node = std::make_unique<Node>(key, value);
        return true;
    }

    bool inserted;
    if (this->compare(key, node->key))
    {
        inserted = this->InsertNode(node->left, key, value);
    }
    else if (this->compare(node->key, key))
    {
        inserted = this->InsertNode(node->right, key, value);
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

template<typename T, typename K>
bool BinarySearchTree<T, K>::Insert(const K& key, const T& value)
{
    if (this->compare(key, key))
    {
        throw std::invalid_argument("BinarySearchTree::Insert: comparator must be strict");
    }
    if (this->count + 1 == 0)
    {
        throw std::length_error("BinarySearchTree::Insert: too many nodes");
    }

    bool inserted = this->InsertNode(this->root, key, value);
    if (inserted)
    {
        ++this->count;
        ++this->version;
    }
    return inserted;
}

template<typename T, typename K>
std::unique_ptr<typename BinarySearchTree<T, K>::Node> BinarySearchTree<T, K>::DetachMin(std::unique_ptr<Node>& node)
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

template<typename T, typename K>
bool BinarySearchTree<T, K>::RemoveNode(std::unique_ptr<Node>& node, const K& key)
{
    if (node == nullptr)
    {
        return false;
    }

    bool removed;
    if (this->compare(key, node->key))
    {
        removed = this->RemoveNode(node->left, key);
    }
    else if (this->compare(node->key, key))
    {
        removed = this->RemoveNode(node->right, key);
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

template<typename T, typename K>
bool BinarySearchTree<T, K>::Remove(const K& key)
{
    bool removed = this->RemoveNode(this->root, key);
    if (removed)
    {
        --this->count;
        ++this->version;
    }
    return removed;
}

template<typename T, typename K>
bool BinarySearchTree<T, K>::Contains(const K& key) const
{
    return this->FindNode(key) != nullptr;
}

template<typename T, typename K>
const T& BinarySearchTree<T, K>::Get(const K& key) const
{
    const Node* node = this->FindNode(key);
    if (node == nullptr)
    {
        throw std::out_of_range("BinarySearchTree::Get: key not found");
    }

    return node->data;
}

template<typename T, typename K>
std::size_t BinarySearchTree<T, K>::GetCount() const
{
    return this->count;
}

template<typename T, typename K>
int BinarySearchTree<T, K>::GetHeight() const
{
    return this->root == nullptr ? 0 : this->root->height;
}

template<typename T, typename K>
const T& BinarySearchTree<T, K>::GetRoot() const
{
    if (this->root == nullptr)
    {
        throw std::out_of_range("BinarySearchTree::GetRoot: tree is empty");
    }

    return this->root->data;
}

template<typename T, typename K>
const K& BinarySearchTree<T, K>::GetRootKey() const
{
    if (this->root == nullptr)
    {
        throw std::out_of_range("BinarySearchTree::GetRootKey: tree is empty");
    }

    return this->root->key;
}

template<typename T, typename K>
void BinarySearchTree<T, K>::Clear()
{
    if (this->root != nullptr)
    {
        this->root.reset();
        this->count = 0;
        ++this->version;
    }
}

template<typename T, typename K>
std::unique_ptr<IEnumerator<T, K>> BinarySearchTree<T, K>::GetEnumerator(TraversalOrder order) const
{
    return std::make_unique<Enumerator>(*this, order);
}

template<typename T, typename K>
const typename BinarySearchTree<T, K>::Node* BinarySearchTree<T, K>::FindNode(const K& key) const
{
    const Node* current = this->root.get();
    while (current != nullptr)
    {
        if (this->compare(key, current->key))
        {
            current = current->left.get();
        }
        else if (this->compare(current->key, key))
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

template<typename T, typename K>
std::unique_ptr<Tree<T, K>> BinarySearchTree<T, K>::Map(T (*func)(const T&)) const
{
    if (func == nullptr)
    {
        throw std::invalid_argument("BinarySearchTree::Map: null function");
    }

    std::unique_ptr<Tree<T, K>> result = std::make_unique<BinarySearchTree<T, K>>(this->compare);
    Enumerator enumerator(*this);
    while (enumerator.MoveNext())
    {
        T value = func(enumerator.GetCurrent());
        result->Insert(enumerator.GetCurrentKey(), value);
    }
    return result;
}

template<typename T, typename K>
std::unique_ptr<Tree<T, K>> BinarySearchTree<T, K>::Where(bool (*predicate)(const T&)) const
{
    if (predicate == nullptr)
    {
        throw std::invalid_argument("BinarySearchTree::Where: null predicate");
    }

    std::unique_ptr<Tree<T, K>> result = std::make_unique<BinarySearchTree<T, K>>(this->compare);
    Enumerator enumerator(*this);
    while (enumerator.MoveNext())
    {
        if (predicate(enumerator.GetCurrent()))
        {
            result->Insert(enumerator.GetCurrentKey(), enumerator.GetCurrent());
        }
    }
    return result;
}

template<typename T, typename K>
T BinarySearchTree<T, K>::Reduce(T (*func)(const T&, const T&), const T& startValue) const
{
    if (func == nullptr)
    {
        throw std::invalid_argument("BinarySearchTree::Reduce: null function");
    }

    T result = startValue;
    Enumerator enumerator(*this);
    while (enumerator.MoveNext())
    {
        result = func(enumerator.GetCurrent(), result);
    }
    return result;
}

template<typename T, typename K>
BinarySearchTree<T, K>::Enumerator::Enumerator(const BinarySearchTree<T, K>& tree, TraversalOrder order)
    : current(nullptr), tree(tree), expectedVersion(tree.version)
{
    switch (order)
    {
        case TraversalOrder::KLP:
            this->steps = "KLP";
            break;
        case TraversalOrder::KPL:
            this->steps = "KPL";
            break;
        case TraversalOrder::LKP:
            this->steps = "LKP";
            break;
        case TraversalOrder::LPK:
            this->steps = "LPK";
            break;
        case TraversalOrder::PLK:
            this->steps = "PLK";
            break;
        case TraversalOrder::PKL:
            this->steps = "PKL";
            break;
        default:
            throw std::invalid_argument("Enumerator: invalid traversal order");
    }
    if (tree.root != nullptr)
    {
        this->frames.Push(Frame(tree.root.get()));
    }
}

template<typename T, typename K>
void BinarySearchTree<T, K>::Enumerator::CheckVersion() const
{
    if (this->tree.version != this->expectedVersion)
    {
        throw std::logic_error("Enumerator: tree changed");
    }
}

template<typename T, typename K>
void BinarySearchTree<T, K>::Enumerator::CheckCurrent() const
{
    this->CheckVersion();
    if (this->current == nullptr)
    {
        throw std::out_of_range("Enumerator: no current element");
    }
}

template<typename T, typename K>
bool BinarySearchTree<T, K>::Enumerator::MoveNext()
{
    this->CheckVersion();
    this->current = nullptr;
    while (!this->frames.IsEmpty())
    {
        Frame& frame = this->frames.Top();
        if (frame.nextStep == this->steps.size())
        {
            this->frames.Pop();
            continue;
        }

        char action = this->steps[frame.nextStep];
        ++frame.nextStep;
        if (action == 'K')
        {
            this->current = frame.node;
            return true;
        }

        const Node* child = nullptr;
        if (action == 'L')
        {
            child = frame.node->left.get();
        }
        else
        {
            child = frame.node->right.get();
        }
        if (child != nullptr)
        {
            this->frames.Push(Frame(child));
        }
    }
    return false;
}

template<typename T, typename K>
const T& BinarySearchTree<T, K>::Enumerator::GetCurrent() const
{
    this->CheckCurrent();
    return this->current->data;
}

template<typename T, typename K>
const K& BinarySearchTree<T, K>::Enumerator::GetCurrentKey() const
{
    this->CheckCurrent();
    return this->current->key;
}

#endif
