#ifndef ARRAYSEQUENCE_HPP
#define ARRAYSEQUENCE_HPP

#include <cstddef>

#include "Sequence.hpp"
#include "DynamicArray.hpp"

template<typename T>
class ArraySequence : public Sequence<T>
{
private:
    DynamicArray<T> items;

public:
    ArraySequence();
    explicit ArraySequence(std::size_t capacity);
    ArraySequence(const T* items, std::size_t count);
    ArraySequence(const ArraySequence<T>& other) = default;
    ArraySequence(ArraySequence<T>&& other) noexcept = default;
    ArraySequence<T>& operator=(const ArraySequence<T>& other) = default;
    ArraySequence<T>& operator=(ArraySequence<T>&& other) noexcept = default;
    ~ArraySequence() override = default;

    const T& Get(std::size_t index) const override;
    std::size_t GetLength() const override;
    void Append(const T& value) override;
};

template<typename T>
ArraySequence<T>::ArraySequence() { }

template<typename T>
ArraySequence<T>::ArraySequence(std::size_t capacity)
{
    this->items.Reserve(capacity);
}

template<typename T>
ArraySequence<T>::ArraySequence(const T* items, std::size_t count) : items(items, count) { }

template<typename T>
const T& ArraySequence<T>::Get(std::size_t index) const
{
    return this->items.Get(index);
}

template<typename T>
std::size_t ArraySequence<T>::GetLength() const
{
    return this->items.GetSize();
}

template<typename T>
void ArraySequence<T>::Append(const T& value)
{
    this->items.PushBack(value);
}

#endif
