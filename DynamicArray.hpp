#ifndef DYNAMICARRAY_HPP
#define DYNAMICARRAY_HPP

#include <cstddef>
#include <stdexcept>
#include <utility>


template<typename T>
class DynamicArray
{
private:
    T* items;
    std::size_t size;
    std::size_t capacity;

    void Reallocate(std::size_t newCapacity, const T* appended = nullptr);
    void Swap(DynamicArray<T>& other) noexcept;

public:
    DynamicArray();
    DynamicArray(const T* items, std::size_t count);
    DynamicArray(const DynamicArray<T>& other);
    DynamicArray(DynamicArray<T>&& other) noexcept;
    DynamicArray<T>& operator=(const DynamicArray<T>& other);
    DynamicArray<T>& operator=(DynamicArray<T>&& other) noexcept;
    ~DynamicArray();

    const T& Get(std::size_t index) const;
    T& Get(std::size_t index);
    std::size_t GetSize() const;
    void Reserve(std::size_t newCapacity);
    void PushBack(const T& value);
    void Clear() noexcept;
};

template<typename T>
DynamicArray<T>::DynamicArray() : items(nullptr), size(0), capacity(0) { }

template<typename T>
DynamicArray<T>::DynamicArray(const T* items, std::size_t count) : DynamicArray<T>()
{
    if (items == nullptr && count > 0)
    {
        throw std::invalid_argument("DynamicArray: invalid items or count");
    }

    try
    {
        this->Reserve(count);
        for (std::size_t i = 0; i < count; ++i)
        {
            this->PushBack(items[i]);
        }
    }
    catch (...)
    {
        this->Clear();
        throw;
    }
}

template<typename T>
DynamicArray<T>::DynamicArray(const DynamicArray<T>& other)
    : DynamicArray<T>(other.items, other.size) { }

template<typename T>
DynamicArray<T>::DynamicArray(DynamicArray<T>&& other) noexcept
    : items(other.items), size(other.size), capacity(other.capacity)
{
    other.items = nullptr;
    other.size = 0;
    other.capacity = 0;
}

template<typename T>
DynamicArray<T>& DynamicArray<T>::operator=(const DynamicArray<T>& other)
{
    if (this != &other)
    {
        DynamicArray<T> copy(other);
        this->Swap(copy);
    }
    return *this;
}

template<typename T>
DynamicArray<T>& DynamicArray<T>::operator=(DynamicArray<T>&& other) noexcept
{
    if (this != &other)
    {
        this->Clear();
        this->items = other.items;
        this->size = other.size;
        this->capacity = other.capacity;
        other.items = nullptr;
        other.size = 0;
        other.capacity = 0;
    }
    return *this;
}

template<typename T>
DynamicArray<T>::~DynamicArray()
{
    this->Clear();
}

template<typename T>
void DynamicArray<T>::Swap(DynamicArray<T>& other) noexcept
{
    std::swap(this->items, other.items);
    std::swap(this->size, other.size);
    std::swap(this->capacity, other.capacity);
}

template<typename T>
void DynamicArray<T>::Clear() noexcept
{
    delete[] this->items;
    this->items = nullptr;
    this->size = 0;
    this->capacity = 0;
}

template<typename T>
void DynamicArray<T>::Reallocate(std::size_t newCapacity, const T* appended)
{
    T* newItems = new T[newCapacity];
    std::size_t newSize = this->size;
    try
    {
        for (std::size_t i = 0; i < this->size; ++i)
        {
            newItems[i] = this->items[i];
        }
        if (appended != nullptr)
        {
            newItems[newSize] = *appended;
            ++newSize;
        }
    }
    catch (...)
    {
        delete[] newItems;
        throw;
    }

    this->Clear();
    this->items = newItems;
    this->size = newSize;
    this->capacity = newCapacity;
}

template<typename T>
void DynamicArray<T>::Reserve(std::size_t newCapacity)
{
    if (newCapacity > this->capacity)
    {
        this->Reallocate(newCapacity);
    }
}

template<typename T>
void DynamicArray<T>::PushBack(const T& value)
{
    if (this->size == this->capacity)
    {
        std::size_t newCapacity;
        if (this->capacity == 0)
        {
            newCapacity = 1;
        }
        else
        {
            newCapacity = this->capacity * 2;
            if (newCapacity <= this->capacity)
            {
                throw std::length_error("DynamicArray::PushBack: capacity overflow");
            }
        }
        this->Reallocate(newCapacity, &value);
    }
    else
    {
        this->items[this->size] = value;
        ++this->size;
    }
}

template<typename T>
const T& DynamicArray<T>::Get(std::size_t index) const
{
    if (index >= this->size)
    {
        throw std::out_of_range("DynamicArray::Get: index out of range");
    }
    return this->items[index];
}

template<typename T>
T& DynamicArray<T>::Get(std::size_t index)
{
    if (index >= this->size)
    {
        throw std::out_of_range("DynamicArray::Get: index out of range");
    }
    return this->items[index];
}

template<typename T>
std::size_t DynamicArray<T>::GetSize() const
{
    return this->size;
}

#endif
