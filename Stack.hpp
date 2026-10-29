#ifndef STACK_HPP
#define STACK_HPP

#include <memory>
#include <stdexcept>
#include <utility>

template<typename T>
class Stack
{
private:
    class Node
    {
    public:
        T data;
        std::unique_ptr<Node> next;

        explicit Node(const T& value) : data(value) { }
    };

    std::unique_ptr<Node> head;

public:
    Stack() = default;
    Stack(const Stack<T>& other) = delete;
    Stack<T>& operator=(const Stack<T>& other) = delete;
    ~Stack();

    void Push(const T& value);
    void Pop();
    T& Top();
    const T& Top() const;
    bool IsEmpty() const;
};

template<typename T>
Stack<T>::~Stack()
{
    while (!this->IsEmpty())
    {
        this->Pop();
    }
}

template<typename T>
void Stack<T>::Push(const T& value)
{
    std::unique_ptr<Node> node = std::make_unique<Node>(value);
    node->next = std::move(this->head);
    this->head = std::move(node);
}

template<typename T>
void Stack<T>::Pop()
{
    if (this->IsEmpty())
    {
        throw std::out_of_range("Stack::Pop: stack is empty");
    }

    std::unique_ptr<Node> oldHead = std::move(this->head);
    this->head = std::move(oldHead->next);
}

template<typename T>
T& Stack<T>::Top()
{
    if (this->IsEmpty())
    {
        throw std::out_of_range("Stack::Top: stack is empty");
    }

    return this->head->data;
}

template<typename T>
const T& Stack<T>::Top() const
{
    if (this->IsEmpty())
    {
        throw std::out_of_range("Stack::Top: stack is empty");
    }

    return this->head->data;
}

template<typename T>
bool Stack<T>::IsEmpty() const
{
    return this->head == nullptr;
}

#endif
