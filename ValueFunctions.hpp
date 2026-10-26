#ifndef VALUEFUNCTIONS_HPP
#define VALUEFUNCTIONS_HPP

#include <cmath>
#include <cstddef>
#include <sstream>
#include <stdexcept>
#include <string>

template<typename T>
T ValueFromString(const std::string& text)
{
    std::istringstream input(text);
    T value;
    if (!(input >> value))
    {
        throw std::invalid_argument("ValueFromString: invalid value");
    }

    input >> std::ws;
    if (!input.eof())
    {
        throw std::invalid_argument("ValueFromString: unexpected trailing characters");
    }

    return value;
}

inline std::size_t SizeFromString(const std::string& text)
{
    std::istringstream input(text);
    input >> std::ws;
    if (input.peek() == '-')
    {
        throw std::invalid_argument("SizeFromString: size cannot be negative");
    }

    return ValueFromString<std::size_t>(text);
}

inline std::string StringFromString(const std::string& text)
{
    return text;
}

inline double DoubleFromString(const std::string& text)
{
    double value = ValueFromString<double>(text);
    if (!std::isfinite(value))
    {
        throw std::invalid_argument("ValueFromString: expected a finite real number");
    }

    return value;
}

#endif
