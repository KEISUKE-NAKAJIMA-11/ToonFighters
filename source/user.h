#pragma once
#include <sstream>
#include <bitset>
#include <assert.h>
#include <queue>


inline std::string strBit16(const int n)
{
    std::stringstream ss;
    ss << static_cast<std::bitset<16>>(n);
    return ss.str();
}


inline const float& clamp(const float& v, const float& lo, const float& hi) {
    assert(lo <= hi);

    return  (std::max)((std::min)(v, hi), lo);

}