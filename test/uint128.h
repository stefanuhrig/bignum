/**
 * Copyright (c) 2024 Stefan Uhrig
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
//------------------------------------------------------------------------------
#ifndef BN_UINT128_H
#define BN_UINT128_H
//------------------------------------------------------------------------------
#include <cstdint>
#include <limits>
#include <ostream>
//------------------------------------------------------------------------------
namespace bn {
//------------------------------------------------------------------------------
struct uint128
{
public:
    uint128() = default;
    uint128(std::int32_t i);
    uint128(std::uint32_t i);
    uint128(std::int64_t i);
    uint128(std::uint64_t);

public:
    uint128& operator=(std::int32_t i);
    uint128& operator=(std::uint32_t i);
    uint128& operator=(std::int64_t i);
    uint128& operator=(std::uint64_t);

public:
    explicit operator bool() const;
    explicit operator std::int32_t() const;
    explicit operator std::uint32_t() const;
    explicit operator std::int64_t() const;
    explicit operator std::uint64_t() const;

public:
    uint128& operator+=(uint128 other);
    uint128& operator-=(uint128 other);
    uint128& operator*=(uint128 other);
    uint128& operator/=(uint128 other);
    uint128& operator%=(uint128 other);
    uint128 operator++(int);
    uint128 operator--(int);
    uint128& operator<<=(std::size_t shift);
    uint128& operator>>=(std::size_t shift);
    uint128& operator&=(uint128 other);
    uint128& operator|=(uint128 other);
    uint128& operator^=(uint128 other);
    uint128& operator++();
    uint128& operator--();

public:
    std::uint64_t lo() const;
    std::uint64_t hi() const;
    std::uint64_t& lo();
    std::uint64_t& hi();

    std::size_t clz() const;

private:
    static std::size_t clz(std::uint64_t u);

private:
    std::uint64_t l;
    std::uint64_t h;
};
//------------------------------------------------------------------------------
bool operator==(uint128 u, uint128 v);
bool operator!=(uint128 u, uint128 v);
bool operator<(uint128 u, uint128 v);
bool operator>(uint128 u, uint128 v);
bool operator<=(uint128 u, uint128 v);
bool operator>=(uint128 u, uint128 v);
//------------------------------------------------------------------------------
uint128 operator+(uint128 u, uint128 v);
uint128 operator-(uint128 u, uint128 v);
uint128 operator*(uint128 u, uint128 v);
uint128 operator/(uint128 u, uint128 v);
uint128 operator%(uint128 u, uint128 v);
uint128 divRem(uint128 u, uint128 v, uint128& rem);
uint128 operator<<(uint128 u, std::size_t shift);
uint128 operator>>(uint128 u, std::size_t shift);
uint128 operator&(uint128 u, uint128 v);
uint128 operator|(uint128 u, uint128 v);
uint128 operator^(uint128 u, uint128 v);
//------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& out, uint128 u);
//------------------------------------------------------------------------------
inline uint128::uint128(std::int32_t i)
    : l(static_cast<std::uint64_t>(i))
    , h((i < 0) ? std::numeric_limits<std::uint64_t>::max() : 0)
{
}
//------------------------------------------------------------------------------
inline uint128::uint128(std::uint32_t i) : l(i), h(0)
{
}
//------------------------------------------------------------------------------
inline uint128::uint128(std::int64_t i)
    : l(static_cast<std::uint64_t>(i))
    , h((i < 0) ? std::numeric_limits<std::uint64_t>::max() : 0)
{
}
//------------------------------------------------------------------------------
inline uint128::uint128(std::uint64_t i) : l(i), h(0)
{
}
//------------------------------------------------------------------------------
inline uint128& uint128::operator=(std::int32_t i)
{
    l = static_cast<std::uint64_t>(i);
    h = (i < 0) ? std::numeric_limits<std::uint64_t>::max() : 0;
    return *this;
}
//------------------------------------------------------------------------------
inline uint128& uint128::operator=(std::uint32_t i)
{
    l = i;
    h = 0;
    return *this;
}
//------------------------------------------------------------------------------
inline uint128& uint128::operator=(std::int64_t i)
{
    l = static_cast<std::uint64_t>(i);
    h = (i < 0) ? std::numeric_limits<std::uint64_t>::max() : 0;
    return *this;
}
//------------------------------------------------------------------------------
inline uint128& uint128::operator=(std::uint64_t i)
{
    l = i;
    h = 0;
    return *this;
}
//------------------------------------------------------------------------------
inline uint128::operator bool() const
{
    return (l != 0) || (h != 0);
}
//------------------------------------------------------------------------------
inline uint128::operator std::int32_t() const
{
    return static_cast<std::int32_t>(l);
}
//------------------------------------------------------------------------------
inline uint128::operator std::uint32_t() const
{
    return static_cast<std::uint32_t>(l);
}
//------------------------------------------------------------------------------
inline uint128::operator std::int64_t() const
{
    return static_cast<std::int64_t>(l);
}
//------------------------------------------------------------------------------
inline uint128::operator std::uint64_t() const
{
    return l;
}
//------------------------------------------------------------------------------
inline uint128& uint128::operator+=(uint128 other)
{
    *this = *this + other;
    return *this;
}
//------------------------------------------------------------------------------
inline uint128& uint128::operator-=(uint128 other)
{
    *this = *this - other;
    return *this;
}
//------------------------------------------------------------------------------
inline uint128& uint128::operator*=(uint128 other)
{
    *this = *this * other;
    return *this;
}
//------------------------------------------------------------------------------
inline uint128& uint128::operator/=(uint128 other)
{
    *this = *this / other;
    return *this;
}
//------------------------------------------------------------------------------
inline uint128& uint128::operator%=(uint128 other)
{
    *this = *this % other;
    return *this;
}
//------------------------------------------------------------------------------
inline uint128 uint128::operator++(int)
{
    uint128 r = *this;
    *this = *this + 1;
    return r;
}
//------------------------------------------------------------------------------
inline uint128 uint128::operator--(int)
{
    uint128 r = *this;
    *this = *this - 1;
    return r;
}
//------------------------------------------------------------------------------
inline uint128& uint128::operator<<=(std::size_t shift)
{
    *this = *this << shift;
    return *this;
}
//------------------------------------------------------------------------------
inline uint128& uint128::operator>>=(std::size_t shift)
{
    *this = *this >> shift;
    return *this;
}
//------------------------------------------------------------------------------
inline uint128& uint128::operator&=(uint128 other)
{
    *this = *this & other;
    return *this;
}
//------------------------------------------------------------------------------
inline uint128& uint128::operator|=(uint128 other)
{
    *this = *this | other;
    return *this;
}
//------------------------------------------------------------------------------
inline uint128& uint128::operator^=(uint128 other)
{
    *this = *this ^ other;
    return *this;
}
//------------------------------------------------------------------------------
inline uint128& uint128::operator++()
{
    *this = *this + 1;
    return *this;
}
//------------------------------------------------------------------------------
inline uint128& uint128::operator--()
{
    *this = *this - 1;
    return *this;
}
//------------------------------------------------------------------------------
inline std::uint64_t uint128::lo() const
{
    return l;
}
//------------------------------------------------------------------------------
inline std::uint64_t uint128::hi() const
{
    return h;
}
//------------------------------------------------------------------------------
inline std::uint64_t& uint128::lo()
{
    return l;
}
//------------------------------------------------------------------------------
inline std::uint64_t& uint128::hi()
{
    return h;
}
//------------------------------------------------------------------------------
inline std::size_t uint128::clz() const
{
    return (h == 0) ? (64 + clz(l)) : clz(h);
}
//------------------------------------------------------------------------------
inline std::size_t uint128::clz(std::uint64_t u)
{
    if (u == 0) {
        return 64;
    }
    size_t ret = 0;
    std::uint64_t mask = std::numeric_limits<std::uint64_t>::max();
    for (std::uint64_t i = 32; i != 0; i >>= 1) {
        mask <<= i;
        if ((u & mask) == 0) {
            ret += i;
            u <<= i;
        }
    }
    return ret;
}
//------------------------------------------------------------------------------
inline bool operator==(uint128 u, uint128 v)
{
    return (u.lo() == v.lo()) && (u.hi() == v.hi());
}
//------------------------------------------------------------------------------
inline bool operator!=(uint128 u, uint128 v)
{
    return !(u == v);
}
//------------------------------------------------------------------------------
inline bool operator<(uint128 u, uint128 v)
{
    if (u.hi() != v.hi()) {
        return u.hi() < v.hi();
    }
    return u.lo() < v.lo();
}
//------------------------------------------------------------------------------
inline bool operator>(uint128 u, uint128 v)
{
    if (u.hi() != v.hi()) {
        return u.hi() > v.hi();
    }
    return u.lo() > v.lo();
}
//------------------------------------------------------------------------------
inline bool operator<=(uint128 u, uint128 v)
{
    return !(u > v);
}
//------------------------------------------------------------------------------
inline bool operator>=(uint128 u, uint128 v)
{
    return !(u < v);
}
//------------------------------------------------------------------------------
inline uint128 operator+(uint128 u, uint128 v)
{
    uint128 r;
    r.lo() = u.lo() + v.lo();
    r.hi() = u.hi() + v.hi() + (r.lo() < u.lo());
    return r;
}
//------------------------------------------------------------------------------
inline uint128 operator-(uint128 u, uint128 v)
{
    uint128 r;
    r.lo() = u.lo() - v.lo();
    r.hi() = u.hi() - v.hi() - (r.lo() > u.lo());
    return r;
}
//------------------------------------------------------------------------------
inline uint128 operator*(uint128 u, uint128 v)
{
    std::uint64_t u32[4] = {
        u.lo() & std::numeric_limits<std::uint32_t>::max(), u.lo() >> 32,
        u.hi() & std::numeric_limits<std::uint32_t>::max(), u.hi() >> 32};
    std::uint64_t v32[4] = {
        v.lo() & std::numeric_limits<std::uint32_t>::max(), v.lo() >> 32,
        v.hi() & std::numeric_limits<std::uint32_t>::max(), v.hi() >> 32};
    std::uint64_t r32[4];
    std::uint64_t carry = 0;
    for (int i = 0; i < 4; ++i) {
        std::uint64_t t = v32[0] * u32[i] + carry;
        r32[i] = t & std::numeric_limits<std::uint32_t>::max();
        carry = t >> 32;
    }
    carry = 0;
    for (int i = 0; i < 3; ++i) {
        std::uint64_t t = v32[1] * u32[i] + r32[i + 1] + carry;
        r32[i] = t & std::numeric_limits<std::uint32_t>::max();
        carry = t >> 32;
    }
    carry = 0;
    for (int i = 0; i < 2; ++i) {
        std::uint64_t t = v32[2] * u32[i] + r32[i + 2] + carry;
        r32[i] = t & std::numeric_limits<std::uint32_t>::max();
        carry = t >> 32;
    }
    std::uint64_t t = v32[3] * u32[0] + r32[3];
    r32[3] = t & std::numeric_limits<std::uint32_t>::max();
    uint128 r;
    r.lo() = r32[0] | (r32[1] << 32);
    r.hi() = r32[2] | (r32[3] << 32);
    return r;
}
//------------------------------------------------------------------------------
inline uint128 operator/(uint128 u, uint128 v)
{
    uint128 rem;
    return divRem(u, v, rem);
}
//------------------------------------------------------------------------------
inline uint128 operator%(uint128 u, uint128 v)
{
    uint128 rem;
    divRem(u, v, rem);
    return rem;
}
//------------------------------------------------------------------------------
inline uint128 divRem(uint128 u, uint128 v, uint128& rem)
{
    if (v > u) {
        rem = u;
        return 0;
    }
    if (u == v) {
        rem = 0;
        return 1;
    }

    uint128 den = v;
    uint128 q = 0;

    const std::size_t shift = v.clz() - u.clz();
    den <<= shift;

    for (std::size_t i = 0; i <= shift; ++i) {
        q <<= 1;
        if (u >= den) {
            u -= den;
            q |= 1;
        }
        den >>= 1;
    }

    rem = u;
    return q;
}
//------------------------------------------------------------------------------
inline uint128 operator<<(uint128 u, std::size_t shift)
{
    uint128 r;
    if (shift >= 128) {
        r.lo() = 0;
        r.hi() = 0;
    } else if (shift >= 64) {
        r.lo() = 0;
        r.hi() = u.lo() << (shift % 64);
    } else {
        r.lo() = u.lo() << shift;
        r.hi() = (u.hi() << shift) | (u.lo() >> (64 - shift));
    }
    return r;
}
//------------------------------------------------------------------------------
uint128 operator>>(uint128 u, std::size_t shift)
{
    uint128 r;
    if (shift >= 128) {
        r.lo() = 0;
        r.hi() = 0;
    } else if (shift >= 64) {
        r.lo() = u.hi() >> (shift % 64);
        r.hi() = 0;
    } else {
        r.lo() = (u.lo() >> shift) | (u.hi() << (64 - shift));
        r.hi() = u.hi() >> shift;
    }
    return r;
}
//------------------------------------------------------------------------------
inline uint128 operator&(uint128 u, uint128 v)
{
    uint128 r;
    r.lo() = u.lo() & v.lo();
    r.hi() = u.hi() & v.hi();
    return r;
}
//------------------------------------------------------------------------------
inline uint128 operator|(uint128 u, uint128 v)
{
    uint128 r;
    r.lo() = u.lo() | v.lo();
    r.hi() = u.hi() | v.hi();
    return r;
}
//------------------------------------------------------------------------------
inline uint128 operator^(uint128 u, uint128 v)
{
    uint128 r;
    r.lo() = u.lo() ^ v.lo();
    r.hi() = u.hi() ^ v.hi();
    return r;
}
//------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& out, uint128 u)
{
    char buffer[64];
    char* curr = buffer;
    if (u == 0) {
        *curr++ = '0';
    } else {
        while (u) {
            uint128 rem;
            u = divRem(u, 10, rem);
            *curr++ = '0' + static_cast<unsigned>(rem);
        }
    }
    std::reverse(buffer, curr);
    *curr = '\0';
    out << buffer;
    return out;
}
//------------------------------------------------------------------------------
}  // namespace bn
//------------------------------------------------------------------------------
#endif
