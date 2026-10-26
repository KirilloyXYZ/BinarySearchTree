#ifndef TREEENUMERATOR_HPP
#define TREEENUMERATOR_HPP

#include <cstddef>
#include <memory>
#include <stack>
#include <stdexcept>

#include "IEnumerator.hpp"
#include "TraversalOrder.hpp"

template<typename T>
class BinarySearchTree;

// Node pointers are borrowed. Mutation or owner destruction invalidates this cursor.
// CheckVersion() detects both cases before any node pointer is dereferenced.
template<typename T>
class TreeEnumerator : public IEnumerator<T>
{
private:
    friend class BinarySearchTree<T>;

    class Frame
    {
    public:
        const typename BinarySearchTree<T>::Node* node;
        int step;

        explicit Frame(const typename BinarySearchTree<T>::Node* node) : node(node), step(0) { }
    };

    std::stack<Frame> frames;
    const typename BinarySearchTree<T>::Node* current;
    int actions[3];
    std::weak_ptr<const std::size_t> version;
    std::size_t expectedVersion;

    TreeEnumerator(const typename BinarySearchTree<T>::Node* root, TraversalOrder order,
                   const std::shared_ptr<std::size_t>& version);
    void CheckVersion() const;

public:
    bool MoveNext() override;
    const T& GetCurrent() const override;
};

template<typename T>
TreeEnumerator<T>::TreeEnumerator(const typename BinarySearchTree<T>::Node* root, TraversalOrder order,
                                  const std::shared_ptr<std::size_t>& version)
    : current(nullptr), version(version), expectedVersion(*version)
{
    // 0: visit the node, 1: descend left, 2: descend right.
    const int orders[6][3] = {
        {0, 1, 2}, // KLP
        {0, 2, 1}, // KPL
        {1, 0, 2}, // LKP
        {1, 2, 0}, // LPK
        {2, 1, 0}, // PLK
        {2, 0, 1}  // PKL
    };
    std::size_t index = static_cast<std::size_t>(order);
    if (index >= 6)
    {
        throw std::invalid_argument("TreeEnumerator: invalid traversal order");
    }
    for (int i = 0; i < 3; ++i)
    {
        this->actions[i] = orders[index][i];
    }
    if (root != nullptr)
    {
        this->frames.push(Frame(root));
    }
}

template<typename T>
void TreeEnumerator<T>::CheckVersion() const
{
    std::shared_ptr<const std::size_t> currentVersion = this->version.lock();
    if (currentVersion == nullptr || *currentVersion != this->expectedVersion)
    {
        throw std::logic_error("TreeEnumerator: tree changed or was destroyed");
    }
}

template<typename T>
bool TreeEnumerator<T>::MoveNext()
{
    this->CheckVersion();
    this->current = nullptr;
    while (!this->frames.empty())
    {
        Frame& frame = this->frames.top();
        if (frame.step == 3)
        {
            this->frames.pop();
            continue;
        }

        int action = this->actions[frame.step];
        ++frame.step;
        if (action == 0)
        {
            this->current = frame.node;
            return true;
        }

        const typename BinarySearchTree<T>::Node* child = action == 1
            ? frame.node->left.get() : frame.node->right.get();
        if (child != nullptr)
        {
            this->frames.push(Frame(child));
        }
    }
    return false;
}

template<typename T>
const T& TreeEnumerator<T>::GetCurrent() const
{
    this->CheckVersion();
    if (this->current == nullptr)
    {
        throw std::out_of_range("TreeEnumerator::GetCurrent: no current element");
    }
    return this->current->data;
}

#endif
