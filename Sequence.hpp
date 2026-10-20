#ifndef SEQUENCE_HPP
#define SEQUENCE_HPP

#include <cstddef>

template<typename T>
class Sequence
{
public:
    virtual ~Sequence() = default;

    virtual const T& Get(std::size_t index) const = 0;
    virtual std::size_t GetLength() const = 0;
    virtual void Append(const T& value) = 0;
};

#endif
