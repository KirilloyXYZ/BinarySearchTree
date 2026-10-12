#ifndef TREE_HPP
#define TREE_HPP

#include <algorithm>
#include <climits>
#include <cstdlib>
#include <iomanip>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

template<typename T>
class Tree
{
public:
    using CompareFunction = bool (*)(const T&, const T&);
    using WriteFunction = std::string (*)(const T&);
    using ReadFunction = T (*)(const std::string&);

protected:
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
    int count;
    CompareFunction compare;

    explicit Tree(CompareFunction compare);
    Tree(const Tree<T>& other);

    virtual std::unique_ptr<Tree<T>> CreateEmpty() const = 0;
    const Node* FindNode(const T& value) const;
    std::unique_ptr<Node> CloneNode(const Node* node) const;

private:
    bool EqualNodes(const Node* left, const Node* right) const;
    int CountNodes(const Node* node) const;
    int ValidateNode(const Node* node, const T* lower, const T* upper, int& size) const;
    void CollectItems(const Node* node, std::vector<T>& items, int limit) const;
    void MapNode(const Node* node, Tree<T>& result, T (*func)(const T&)) const;
    void WhereNode(const Node* node, Tree<T>& result, bool (*predicate)(const T&)) const;
    void ReduceNode(const Node* node, T& result, T (*func)(const T&, const T&)) const;
    void WriteNode(const Node* node, std::ostream& output, WriteFunction write) const;
    std::unique_ptr<Node> ReadNode(std::istream& input, ReadFunction read, int depth, int& size) const;
    void PrintNode(const Node* node, std::ostream& output, WriteFunction write, int depth, int maxDepth) const;

public:
    virtual ~Tree() = default;

    virtual bool Insert(const T& value) = 0;
    virtual bool Remove(const T& value) = 0;
    virtual bool Contains(const T& value) const = 0;

    virtual int GetCount() const;
    virtual int GetHeight() const;
    virtual CompareFunction GetCompare() const;
    virtual const T& GetRoot() const;
    virtual void Clear();
    virtual bool IsValid() const;
    virtual std::vector<T> GetItems(int limit = INT_MAX) const;

    virtual std::unique_ptr<Tree<T>> Clone() const;
    virtual std::unique_ptr<Tree<T>> Map(T (*func)(const T&)) const;
    virtual std::unique_ptr<Tree<T>> Where(bool (*predicate)(const T&)) const;
    virtual T Reduce(T (*func)(const T&, const T&), const T& startValue) const;
    virtual std::unique_ptr<Tree<T>> GetSubtree(const T& value) const;
    virtual bool ContainsSubtree(const Tree<T>& other) const;

    virtual std::string ToString(WriteFunction write) const;
    virtual void ReadFromString(const std::string& text, ReadFunction read);
    virtual void Print(std::ostream& output, WriteFunction write, int maxDepth = 5) const;
};

template<typename T>
Tree<T>::Tree(CompareFunction compare) : count(0), compare(compare)
{
    if (compare == nullptr)
    {
        throw std::invalid_argument("Tree: null comparator");
    }
}

template<typename T>
Tree<T>::Tree(const Tree<T>& other) : count(other.count), compare(other.compare)
{
    this->root = this->CloneNode(other.root.get());
}

template<typename T>
int Tree<T>::GetCount() const
{
    return this->count;
}

template<typename T>
int Tree<T>::GetHeight() const
{
    return this->root == nullptr ? 0 : this->root->height;
}

template<typename T>
typename Tree<T>::CompareFunction Tree<T>::GetCompare() const
{
    return this->compare;
}

template<typename T>
const T& Tree<T>::GetRoot() const
{
    if (this->root == nullptr)
    {
        throw std::out_of_range("Tree::GetRoot: tree is empty");
    }

    return this->root->data;
}

template<typename T>
void Tree<T>::Clear()
{
    this->root.reset();
    this->count = 0;
}

template<typename T>
const typename Tree<T>::Node* Tree<T>::FindNode(const T& value) const
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
std::unique_ptr<typename Tree<T>::Node> Tree<T>::CloneNode(const Node* node) const
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
std::unique_ptr<Tree<T>> Tree<T>::Clone() const
{
    std::unique_ptr<Tree<T>> result = this->CreateEmpty();
    result->root = this->CloneNode(this->root.get());
    result->count = this->count;
    return result;
}

template<typename T>
void Tree<T>::CollectItems(const Node* node, std::vector<T>& items, int limit) const
{
    if (node == nullptr || static_cast<int>(items.size()) >= limit)
    {
        return;
    }

    this->CollectItems(node->left.get(), items, limit);
    if (static_cast<int>(items.size()) < limit)
    {
        items.push_back(node->data);
        this->CollectItems(node->right.get(), items, limit);
    }
}

template<typename T>
std::vector<T> Tree<T>::GetItems(int limit) const
{
    if (limit < 0)
    {
        throw std::invalid_argument("Tree::GetItems: negative limit");
    }

    std::vector<T> result;
    result.reserve(std::min(this->count, limit));
    this->CollectItems(this->root.get(), result, limit);
    return result;
}

template<typename T>
void Tree<T>::MapNode(const Node* node, Tree<T>& result, T (*func)(const T&)) const
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
std::unique_ptr<Tree<T>> Tree<T>::Map(T (*func)(const T&)) const
{
    if (func == nullptr)
    {
        throw std::invalid_argument("Tree::Map: null function");
    }

    std::unique_ptr<Tree<T>> result = this->CreateEmpty();
    this->MapNode(this->root.get(), *result, func);
    return result;
}

template<typename T>
void Tree<T>::WhereNode(const Node* node, Tree<T>& result, bool (*predicate)(const T&)) const
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
std::unique_ptr<Tree<T>> Tree<T>::Where(bool (*predicate)(const T&)) const
{
    if (predicate == nullptr)
    {
        throw std::invalid_argument("Tree::Where: null predicate");
    }

    std::unique_ptr<Tree<T>> result = this->CreateEmpty();
    this->WhereNode(this->root.get(), *result, predicate);
    return result;
}

template<typename T>
void Tree<T>::ReduceNode(const Node* node, T& result, T (*func)(const T&, const T&)) const
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
T Tree<T>::Reduce(T (*func)(const T&, const T&), const T& startValue) const
{
    if (func == nullptr)
    {
        throw std::invalid_argument("Tree::Reduce: null function");
    }

    T result = startValue;
    this->ReduceNode(this->root.get(), result, func);
    return result;
}

template<typename T>
int Tree<T>::CountNodes(const Node* node) const
{
    if (node == nullptr)
    {
        return 0;
    }

    return 1 + this->CountNodes(node->left.get()) + this->CountNodes(node->right.get());
}

template<typename T>
std::unique_ptr<Tree<T>> Tree<T>::GetSubtree(const T& value) const
{
    const Node* node = this->FindNode(value);
    if (node == nullptr)
    {
        throw std::out_of_range("Tree::GetSubtree: root not found");
    }

    std::unique_ptr<Tree<T>> result = this->CreateEmpty();
    result->root = this->CloneNode(node);
    result->count = this->CountNodes(node);
    return result;
}

template<typename T>
bool Tree<T>::EqualNodes(const Node* left, const Node* right) const
{
    if (left == nullptr || right == nullptr)
    {
        return left == right;
    }

    if (this->compare(left->data, right->data) || this->compare(right->data, left->data))
    {
        return false;
    }

    return this->EqualNodes(left->left.get(), right->left.get()) &&
           this->EqualNodes(left->right.get(), right->right.get());
}

template<typename T>
bool Tree<T>::ContainsSubtree(const Tree<T>& other) const
{
    if (this->compare != other.compare)
    {
        throw std::invalid_argument("Tree::ContainsSubtree: different comparators");
    }
    if (other.root == nullptr)
    {
        return true;
    }

    return this->EqualNodes(this->FindNode(other.root->data), other.root.get());
}

template<typename T>
int Tree<T>::ValidateNode(const Node* node, const T* lower, const T* upper, int& size) const
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
bool Tree<T>::IsValid() const
{
    int size = 0;
    return this->ValidateNode(this->root.get(), nullptr, nullptr, size) >= 0 && size == this->count;
}

template<typename T>
void Tree<T>::WriteNode(const Node* node, std::ostream& output, WriteFunction write) const
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
std::string Tree<T>::ToString(WriteFunction write) const
{
    if (write == nullptr)
    {
        throw std::invalid_argument("Tree::ToString: null writer");
    }

    std::ostringstream output;
    this->WriteNode(this->root.get(), output, write);
    return output.str();
}

template<typename T>
std::unique_ptr<typename Tree<T>::Node> Tree<T>::ReadNode(std::istream& input, ReadFunction read, int depth, int& size) const
{
    input >> std::ws;
    if (input.peek() == '#')
    {
        input.get();
        return nullptr;
    }
    // An AVL with at most INT_MAX nodes is much shallower than 64 levels.
    if (depth >= 64 || input.get() != '(')
    {
        throw std::invalid_argument("Tree::ReadFromString: invalid node or excessive depth");
    }

    input >> std::ws;
    std::string value;
    if (input.peek() != '"' || !(input >> std::quoted(value)))
    {
        throw std::invalid_argument("Tree::ReadFromString: expected a quoted value");
    }
    if (size == INT_MAX)
    {
        throw std::length_error("Tree::ReadFromString: too many nodes");
    }

    std::unique_ptr<Node> node = std::make_unique<Node>(read(value));
    if (this->compare(node->data, node->data))
    {
        throw std::invalid_argument("Tree::ReadFromString: invalid comparison rule");
    }
    ++size;
    node->left = this->ReadNode(input, read, depth + 1, size);
    node->right = this->ReadNode(input, read, depth + 1, size);
    input >> std::ws;
    if (input.get() != ')')
    {
        throw std::invalid_argument("Tree::ReadFromString: expected closing parenthesis");
    }

    int leftHeight = node->left == nullptr ? 0 : node->left->height;
    int rightHeight = node->right == nullptr ? 0 : node->right->height;
    node->height = 1 + std::max(leftHeight, rightHeight);
    return node;
}

template<typename T>
void Tree<T>::ReadFromString(const std::string& text, ReadFunction read)
{
    if (read == nullptr)
    {
        throw std::invalid_argument("Tree::ReadFromString: null reader");
    }

    std::istringstream input(text);
    int size = 0;
    std::unique_ptr<Node> newRoot = this->ReadNode(input, read, 0, size);
    input >> std::ws;
    int checkedSize = 0;
    if (!input.eof() || this->ValidateNode(newRoot.get(), nullptr, nullptr, checkedSize) < 0 || checkedSize != size)
    {
        throw std::invalid_argument("Tree::ReadFromString: invalid AVL tree or trailing data");
    }

    this->root = std::move(newRoot);
    this->count = size;
}

template<typename T>
void Tree<T>::PrintNode(const Node* node, std::ostream& output, WriteFunction write, int depth, int maxDepth) const
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
void Tree<T>::Print(std::ostream& output, WriteFunction write, int maxDepth) const
{
    if (write == nullptr || maxDepth < 1 || maxDepth > 10)
    {
        throw std::invalid_argument("Tree::Print: null writer or depth outside 1..10");
    }
    if (this->root == nullptr)
    {
        output << "<empty>\n";
        return;
    }

    this->PrintNode(this->root.get(), output, write, 0, maxDepth);
}

#endif
