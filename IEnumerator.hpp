#ifndef IENUMERATOR_HPP
#define IENUMERATOR_HPP

template<typename T>
class IEnumerator
{
public:
    virtual ~IEnumerator() = default;

    virtual bool MoveNext() = 0;
    // Valid only after a successful MoveNext() and while the container is unchanged.
    virtual const T& GetCurrent() const = 0;
};

#endif
