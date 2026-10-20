#ifndef BINARYSEARCHTREE_HPP
#define BINARYSEARCHTREE_HPP

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <iomanip>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

#include "Tree.hpp"
#include "ArraySequence.hpp"
#include "ValueFunctions.hpp"

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
    std::unique_ptr<Node> CloneNode(const Node* node) const;
    bool EqualNodes(const Node* node, const Sequence<const T*>& structure, std::size_t& index) const;
    std::size_t CountNodes(const Node* node) const;
    int ValidateNode(const Node* node, const T* lower, const T* upper, std::size_t& size) const;
    void CollectItems(const Node* node, Sequence<T>& items, std::size_t limit) const;
    void CollectStructure(const Node* node, Sequence<const T*>& structure) const;
    void MapNode(const Node* node, Tree<T>& result, T (*func)(const T&)) const;
    void WhereNode(const Node* node, Tree<T>& result, bool (*predicate)(const T&)) const;
    void ReduceNode(const Node* node, T& result, T (*func)(const T&, const T&)) const;
    void WriteNode(const Node* node, std::ostream& output, std::string (*write)(const T&)) const;
    std::unique_ptr<Node> ReadNode(std::istream& input, T (*read)(const std::string&), std::size_t depth, std::size_t& size) const;
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
    BinarySearchTree(const T* items, std::size_t count, bool (*compare)(const T&, const T&) = CompareValues<T>);
    BinarySearchTree(const BinarySearchTree<T>& other);
    BinarySearchTree<T>& operator=(const BinarySearchTree<T>& other);
    ~BinarySearchTree() override = default;

    bool Insert(const T& value) override;
    bool Remove(const T& value) override;
    bool Contains(const T& value) const override;

    std::size_t GetCount() const override;
    int GetHeight() const override;
    bool (*GetCompare() const)(const T&, const T&) override;
    const T& GetRoot() const override;
    void Clear() override;
    bool IsValid() const override;
    std::unique_ptr<Sequence<T>> GetItems() const override;
    std::unique_ptr<Sequence<T>> GetItems(std::size_t limit) const override;
    std::unique_ptr<Sequence<const T*>> GetStructure() const override;

    std::unique_ptr<Tree<T>> Clone() const override;
    std::unique_ptr<Tree<T>> Map(T (*func)(const T&)) const override;
    std::unique_ptr<Tree<T>> Where(bool (*predicate)(const T&)) const override;
    T Reduce(T (*func)(const T&, const T&), const T& startValue) const override;
    std::unique_ptr<Tree<T>> GetSubtree(const T& value) const override;
    bool ContainsSubtree(const Tree<T>& other) const override;

    std::string ToString(std::string (*write)(const T&)) const override;
    void ReadFromString(const std::string& text, T (*read)(const std::string&)) override;
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
BinarySearchTree<T>::BinarySearchTree(const T* items, std::size_t count, bool (*compare)(const T&, const T&)) : BinarySearchTree<T>(compare)
{
    if (items == nullptr && count > 0)
    {
        throw std::invalid_argument("BinarySearchTree: null array with nonzero count");
    }

    for (std::size_t i = 0; i < count; ++i)
    {
        this->Insert(items[i]);
    }
}

template<typename T>
BinarySearchTree<T>::BinarySearchTree(const BinarySearchTree<T>& other)
    : count(other.count), compare(other.compare)
{
    this->root = this->CloneNode(other.root.get());
}

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
bool (*BinarySearchTree<T>::GetCompare() const)(const T&, const T&)
{
    return this->compare;
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
std::unique_ptr<typename BinarySearchTree<T>::Node> BinarySearchTree<T>::CloneNode(const Node* node) const
{
    if (node == nullptr)
    {
        return nullptr;
    }

    std::unique_ptr<Node> result = std::make_unique<Node>(node->data);
    result->left = this->CloneNode(node->left.get());
    result->right = this->CloneNode(node->right.get());
    result->height = node->height;
    return result;
}

template<typename T>
std::unique_ptr<Tree<T>> BinarySearchTree<T>::Clone() const
{
    return std::make_unique<BinarySearchTree<T>>(*this);
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
void BinarySearchTree<T>::CollectStructure(const Node* node, Sequence<const T*>& structure) const
{
    if (node == nullptr)
    {
        structure.Append(nullptr);
        return;
    }

    structure.Append(&node->data);
    this->CollectStructure(node->left.get(), structure);
    this->CollectStructure(node->right.get(), structure);
}

template<typename T>
std::unique_ptr<Sequence<const T*>> BinarySearchTree<T>::GetStructure() const
{
    // Preorder contains n values and n + 1 empty subtrees.
    std::size_t capacity = this->count * 2 + 1;
    if (capacity <= this->count)
    {
        throw std::length_error("BinarySearchTree::GetStructure: too many nodes");
    }

    std::unique_ptr<Sequence<const T*>> result = std::make_unique<ArraySequence<const T*>>(capacity);
    this->CollectStructure(this->root.get(), *result);
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

template<typename T>
std::size_t BinarySearchTree<T>::CountNodes(const Node* node) const
{
    if (node == nullptr)
    {
        return 0;
    }

    return 1 + this->CountNodes(node->left.get()) + this->CountNodes(node->right.get());
}

template<typename T>
std::unique_ptr<Tree<T>> BinarySearchTree<T>::GetSubtree(const T& value) const
{
    const Node* node = this->FindNode(value);
    if (node == nullptr)
    {
        throw std::out_of_range("BinarySearchTree::GetSubtree: root not found");
    }

    std::unique_ptr<BinarySearchTree<T>> result = std::make_unique<BinarySearchTree<T>>(this->compare);
    result->root = this->CloneNode(node);
    result->count = this->CountNodes(node);
    return result;
}

template<typename T>
bool BinarySearchTree<T>::EqualNodes(const Node* node, const Sequence<const T*>& structure, std::size_t& index) const
{
    if (index >= structure.GetLength())
    {
        return false;
    }

    const T* value = structure.Get(index);
    ++index;
    if (node == nullptr || value == nullptr)
    {
        return node == nullptr && value == nullptr;
    }
    if (this->compare(node->data, *value) || this->compare(*value, node->data))
    {
        return false;
    }

    return this->EqualNodes(node->left.get(), structure, index) &&
           this->EqualNodes(node->right.get(), structure, index);
}

template<typename T>
bool BinarySearchTree<T>::ContainsSubtree(const Tree<T>& other) const
{
    if (this->compare != other.GetCompare())
    {
        throw std::invalid_argument("BinarySearchTree::ContainsSubtree: different comparators");
    }
    if (other.GetCount() == 0)
    {
        return true;
    }
    if (other.GetCount() > this->count)
    {
        return false;
    }

    const Node* node = this->FindNode(other.GetRoot());
    if (node == nullptr)
    {
        return false;
    }

    std::unique_ptr<Sequence<const T*>> structure = other.GetStructure();
    std::size_t index = 0;
    return this->EqualNodes(node, *structure, index) && index == structure->GetLength();
}

template<typename T>
int BinarySearchTree<T>::ValidateNode(const Node* node, const T* lower, const T* upper, std::size_t& size) const
{
    if (node == nullptr)
    {
        return 0;
    }
    if ((lower != nullptr && !this->compare(*lower, node->data)) ||
        (upper != nullptr && !this->compare(node->data, *upper)))
    {
        return -1;
    }

    int leftHeight = this->ValidateNode(node->left.get(), lower, &node->data, size);
    int rightHeight = this->ValidateNode(node->right.get(), &node->data, upper, size);
    if (leftHeight < 0 || rightHeight < 0 || std::abs(leftHeight - rightHeight) > 1)
    {
        return -1;
    }

    int height = 1 + std::max(leftHeight, rightHeight);
    if (node->height != height)
    {
        return -1;
    }

    ++size;
    return height;
}

template<typename T>
bool BinarySearchTree<T>::IsValid() const
{
    std::size_t size = 0;
    return this->ValidateNode(this->root.get(), nullptr, nullptr, size) >= 0 && size == this->count;
}

template<typename T>
void BinarySearchTree<T>::WriteNode(const Node* node, std::ostream& output, std::string (*write)(const T&)) const
{
    if (node == nullptr)
    {
        output << '#';
        return;
    }

    output << '(' << std::quoted(write(node->data)) << ' ';
    this->WriteNode(node->left.get(), output, write);
    output << ' ';
    this->WriteNode(node->right.get(), output, write);
    output << ')';
}

template<typename T>
std::string BinarySearchTree<T>::ToString(std::string (*write)(const T&)) const
{
    if (write == nullptr)
    {
        throw std::invalid_argument("BinarySearchTree::ToString: null writer");
    }

    std::ostringstream output;
    this->WriteNode(this->root.get(), output, write);
    return output.str();
}

template<typename T>
std::unique_ptr<typename BinarySearchTree<T>::Node> BinarySearchTree<T>::ReadNode(std::istream& input, T (*read)(const std::string&), std::size_t depth, std::size_t& size) const
{
    input >> std::ws;
    if (input.peek() == '#')
    {
        input.get();
        return nullptr;
    }
    // Limit recursion when parsing malformed input.
    if (depth >= 128 || input.get() != '(')
    {
        throw std::invalid_argument("BinarySearchTree::ReadFromString: invalid node or excessive depth");
    }

    input >> std::ws;
    std::string value;
    if (input.peek() != '"' || !(input >> std::quoted(value)))
    {
        throw std::invalid_argument("BinarySearchTree::ReadFromString: expected a quoted value");
    }
    if (size + 1 == 0)
    {
        throw std::length_error("BinarySearchTree::ReadFromString: too many nodes");
    }

    std::unique_ptr<Node> node = std::make_unique<Node>(read(value));
    if (this->compare(node->data, node->data))
    {
        throw std::invalid_argument("BinarySearchTree::ReadFromString: invalid comparison rule");
    }
    ++size;
    node->left = this->ReadNode(input, read, depth + 1, size);
    node->right = this->ReadNode(input, read, depth + 1, size);
    input >> std::ws;
    if (input.get() != ')')
    {
        throw std::invalid_argument("BinarySearchTree::ReadFromString: expected closing parenthesis");
    }

    int leftHeight = node->left == nullptr ? 0 : node->left->height;
    int rightHeight = node->right == nullptr ? 0 : node->right->height;
    node->height = 1 + std::max(leftHeight, rightHeight);
    return node;
}

template<typename T>
void BinarySearchTree<T>::ReadFromString(const std::string& text, T (*read)(const std::string&))
{
    if (read == nullptr)
    {
        throw std::invalid_argument("BinarySearchTree::ReadFromString: null reader");
    }

    std::istringstream input(text);
    std::size_t size = 0;
    std::unique_ptr<Node> newRoot = this->ReadNode(input, read, 0, size);
    input >> std::ws;
    std::size_t checkedSize = 0;
    if (!input.eof() || this->ValidateNode(newRoot.get(), nullptr, nullptr, checkedSize) < 0 || checkedSize != size)
    {
        throw std::invalid_argument("BinarySearchTree::ReadFromString: invalid AVL tree or trailing data");
    }

    this->root = std::move(newRoot);
    this->count = size;
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
