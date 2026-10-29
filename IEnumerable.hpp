#ifndef IENUMERABLE_HPP
#define IENUMERABLE_HPP

#include <memory>
#include <stdexcept>

#include "IEnumerator.hpp"
#include "TraversalOrder.hpp"

template<typename T, typename K>
class IEnumerable
{
public:
    virtual ~IEnumerable() = default;

    virtual std::unique_ptr<IEnumerator<T, K>> GetEnumerator(TraversalOrder order = TraversalOrder::LKP) const = 0;
    // The action must not change this container.
    void Traverse(TraversalOrder order, void (*action)(const K&, const T&)) const;
};

template<typename T, typename K>
void IEnumerable<T, K>::Traverse(TraversalOrder order, void (*action)(const K&, const T&)) const
{
    if (action == nullptr)
    {
        throw std::invalid_argument("IEnumerable::Traverse: null action");
    }

    std::unique_ptr<IEnumerator<T, K>> enumerator = this->GetEnumerator(order);
    while (enumerator->MoveNext())
    {
        action(enumerator->GetCurrentKey(), enumerator->GetCurrent());
    }
}

#endif
