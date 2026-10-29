#ifndef IENUMERATOR_HPP
#define IENUMERATOR_HPP

template<typename T, typename K>
class IEnumerator
{
public:
    virtual ~IEnumerator() = default;
    virtual bool MoveNext() = 0;
    virtual const T& GetCurrent() const = 0;
    virtual const K& GetCurrentKey() const = 0;
};

#endif
