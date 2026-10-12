#ifndef VALUEFUNCTIONS_HPP
#define VALUEFUNCTIONS_HPP

#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>

template<typename T>
bool CompareValues(const T& left, const T& right)
{
    return left < right;
}

template<>
inline bool CompareValues<double>(const double& left, const double& right)
{
    if (std::isnan(left) || std::isnan(right))
    {
        throw std::invalid_argument("CompareValues: NaN cannot be ordered");
    }

    return left < right;
}

template<typename T>
std::string ValueToString(const T& value)
{
    std::ostringstream output;
    output << std::setprecision(std::numeric_limits<double>::max_digits10) << value;
    if (!output)
    {
        throw std::runtime_error("ValueToString: cannot write value");
    }

    return output.str();
}

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

template<>
inline std::string ValueToString<std::string>(const std::string& value)
{
    return value;
}

template<>
inline std::string ValueFromString<std::string>(const std::string& text)
{
    return text;
}

template<>
inline double ValueFromString<double>(const std::string& text)
{
    std::istringstream input(text);
    double value;
    if (!(input >> value) || !std::isfinite(value))
    {
        throw std::invalid_argument("ValueFromString: expected a finite real number");
    }

    input >> std::ws;
    if (!input.eof())
    {
        throw std::invalid_argument("ValueFromString: unexpected trailing characters");
    }

    return value;
}

#endif
