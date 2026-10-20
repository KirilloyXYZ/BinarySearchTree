#include "SequenceTests.hpp"

#include <memory>

#include "ArraySequence.hpp"

void TestSequenceBasics()
{
    std::unique_ptr<Sequence<int>> sequence = std::make_unique<ArraySequence<int>>();
    assert(sequence->GetLength() == 0);
    sequence->Append(10);
    sequence->Append(20);
    sequence->Append(sequence->Get(0));
    CheckItems(*sequence, {10, 20, 10});

    int values[] = {1, 2, 3};
    ArraySequence<int> original(values, 3);
    ArraySequence<int> copy(original);
    copy.Append(4);
    CheckItems(original, {1, 2, 3});
    CheckItems(copy, {1, 2, 3, 4});
    ArraySequence<int> assigned;
    assigned = copy;
    assigned.Append(5);
    CheckItems(copy, {1, 2, 3, 4});
    CheckItems(assigned, {1, 2, 3, 4, 5});
    ArraySequence<int> moved(std::move(assigned));
    assert(assigned.GetLength() == 0);
    CheckItems(moved, {1, 2, 3, 4, 5});
    assigned = std::move(moved);
    assert(moved.GetLength() == 0);
    CheckItems(assigned, {1, 2, 3, 4, 5});

    DynamicArray<int> array(values, 3);
    DynamicArray<int> arrayCopy(array);
    arrayCopy.Get(0) = 99;
    assert(array.Get(0) == 1);
    DynamicArray<int>& same = array;
    array = same;
    assert(array.GetSize() == 3 && array.Get(2) == 3);
    const int* address = &array.Get(0);
    DynamicArray<int> arrayMoved(std::move(array));
    assert(array.GetSize() == 0 && &arrayMoved.Get(0) == address);
    array.PushBack(100);
    assert(array.Get(0) == 100);
    array = std::move(arrayMoved);
    assert(arrayMoved.GetSize() == 0 && &array.Get(0) == address);
    array.Clear();
    array.Clear();
    assert(array.GetSize() == 0);
    array.Reserve(0);
    assert(array.GetSize() == 0);
    array.Reserve(100);
    for (std::size_t i = 0; i < 100; ++i) { array.PushBack(static_cast<int>(i)); }
    for (std::size_t i = 0; i < 100; ++i) { assert(array.Get(i) == static_cast<int>(i)); }

    DynamicArray<int> empty(nullptr, 0);
    assert(empty.GetSize() == 0);

    for (int mode = 0; mode < 4; ++mode)
    {
        bool thrown = false;
        try
        {
            if (mode == 0) { sequence->Get(sequence->GetLength() + 1); }
            if (mode == 1) { sequence->Get(sequence->GetLength()); }
            if (mode == 2) { array.Get(array.GetSize()); }
            if (mode == 3) { DynamicArray<int> invalid(nullptr, 1); }
        }
        catch (const std::out_of_range&) { thrown = true; }
        catch (const std::invalid_argument&) { thrown = true; }
        assert(thrown);
    }
}

class SequenceValue
{
public:
    static int alive;
    static int copiesLeft;
    int value;

    SequenceValue() : value(0) { ++alive; }
    explicit SequenceValue(int value) : value(value) { ++alive; }
    SequenceValue(const SequenceValue& other) : value(other.value)
    {
        if (copiesLeft == 0) { throw std::runtime_error("test sequence copy failure"); }
        if (copiesLeft > 0) { --copiesLeft; }
        ++alive;
    }
    SequenceValue& operator=(const SequenceValue& other)
    {
        if (copiesLeft == 0) { throw std::runtime_error("test sequence assignment failure"); }
        if (copiesLeft > 0) { --copiesLeft; }
        this->value = other.value;
        return *this;
    }
    ~SequenceValue() { --alive; }
};

int SequenceValue::alive = 0;
int SequenceValue::copiesLeft = -1;

void TestSequenceLifetimeAndExceptions()
{
    assert(SequenceValue::alive == 0);
    {
        DynamicArray<SequenceValue> items;
        items.PushBack(SequenceValue(10));
        SequenceValue::copiesLeft = 1;
        bool thrown = false;
        try { items.PushBack(items.Get(0)); }
        catch (const std::runtime_error&) { thrown = true; }
        SequenceValue::copiesLeft = -1;
        assert(thrown && items.GetSize() == 1 && items.Get(0).value == 10);
        assert(SequenceValue::alive == 1);

        items.PushBack(items.Get(0));
        assert(items.GetSize() == 2 && items.Get(1).value == 10);
        const SequenceValue* address = &items.Get(0);
        SequenceValue::copiesLeft = 1;
        thrown = false;
        try { items.Reserve(8); }
        catch (const std::runtime_error&) { thrown = true; }
        SequenceValue::copiesLeft = -1;
        assert(thrown && &items.Get(0) == address && SequenceValue::alive == 2);

        DynamicArray<SequenceValue> assigned;
        assigned.PushBack(SequenceValue(99));
        SequenceValue::copiesLeft = 1;
        thrown = false;
        try { assigned = items; }
        catch (const std::runtime_error&) { thrown = true; }
        SequenceValue::copiesLeft = -1;
        assert(thrown && assigned.GetSize() == 1 && assigned.Get(0).value == 99);
        assert(SequenceValue::alive == 3);

        assigned = items;
        assert(assigned.GetSize() == 2 && SequenceValue::alive == 4);
        SequenceValue::copiesLeft = 0;
        DynamicArray<SequenceValue> moved(std::move(assigned));
        SequenceValue::copiesLeft = -1;
        assert(assigned.GetSize() == 0 && moved.GetSize() == 2 && SequenceValue::alive == 4);
        items.Clear();
        assert(SequenceValue::alive == 2);

        std::unique_ptr<Sequence<SequenceValue>> sequence = std::make_unique<ArraySequence<SequenceValue>>();
        sequence->Append(SequenceValue(7));
        sequence->Append(sequence->Get(0));
        assert(sequence->GetLength() == 2 && sequence->Get(1).value == 7);
        assert(SequenceValue::alive == 4);
        sequence.reset();
        assert(SequenceValue::alive == 2);

        {
            ArraySequence<SequenceValue> reserved(4);
            assert(reserved.GetLength() == 0 && SequenceValue::alive == 6);
            reserved.Append(SequenceValue(42));
            assert(reserved.GetLength() == 1 && reserved.Get(0).value == 42);
            assert(SequenceValue::alive == 6);
        }
        assert(SequenceValue::alive == 2);
    }
    assert(SequenceValue::alive == 0);
}
