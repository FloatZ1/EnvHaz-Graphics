#ifndef ENVHAZGRAPHICS_HASHED_STRINGS_HPP
#define ENVHAZGRAPHICS_HASHED_STRINGS_HPP

#include <cstddef>
#include <iostream>
namespace eHazGraphics_Utils
{

typedef std::size_t HashedString;


static long long computeHash(std::string const &s)
{
    const int p = 31;
    const int m = 1e9 + 9;
    long long hash_value = 0;
    long long p_pow = 1;
    for (char c : s)
    {
        hash_value = (hash_value + (c - 'a' + 1) * p_pow) % m;
        p_pow = (p_pow * p) % m;
    }

    return hash_value;
}
} // namespace eHazGraphics_Utils



#endif
