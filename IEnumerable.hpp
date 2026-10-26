#ifndef IENUMERABLE_HPP
#define IENUMERABLE_HPP

#include <memory>

#include "IEnumerator.hpp"
#include "TraversalOrder.hpp"

template<typename T>
class IEnumerable
{
public:
    virtual ~IEnumerable() = default;

    virtual std::unique_ptr<IEnumerator<T>> GetEnumerator(TraversalOrder order = TraversalOrder::LKP) const = 0;

    // The action must be callable and must not change this container.
    template<typename Action>
    void Traverse(TraversalOrder order, Action action) const
    {
        std::unique_ptr<IEnumerator<T>> enumerator = this->GetEnumerator(order);
        while (enumerator->MoveNext())
        {
            action(enumerator->GetCurrent());
        }
    }
};

#endif
