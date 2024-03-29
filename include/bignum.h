/*
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
/*******************************************************************************
 * @mainpage  A C++-11 single-header generic arbitrary-precision number library
 *
 * This is a C++-11 compliant generic single-header arbitrary-precision number
 * library offering natural (@ref bn::Unsigned), integer (@ref bn::Signed) and
 * rational (@ref bn::Rational) number types and corresponding mathematical
 * operations.
 *
 * Design goals of this library are
 *   - intuitive and easy usage
 *   - providing all functionality in a single header so that it can easily be
 *     consumed by C++ projects
 *   - maximum compatibility with C++-11 compliant compilers and all platforms
 *
 * This library does not contain optimized algorithms for any particular
 * platform and uses mainly naive "schoolbook" methods. With regards to
 * performance, it cannot compete with mature arbitrary-precision libraries like
 * GMP.
 *
 * The given time complexity is the worst case complexity. n is the number of
 * digits in a number. If multiple numbers are involved in an operation, n is
 * chosen to be the number of digits in the longest number.
 *
 * If you'd like to improve the performance on a particular architecture, you
 * can customize the digit type and the seven primitive operations on which all
 * algorithms are based.
 ******************************************************************************/
//------------------------------------------------------------------------------
#ifndef BN_BIGNUM_H
#define BN_BIGNUM_H
//------------------------------------------------------------------------------
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <new>
#include <ostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
//------------------------------------------------------------------------------
namespace bn {
//------------------------------------------------------------------------------
namespace impl {
//------------------------------------------------------------------------------
// Digit type.
//
// If your platform offers wider types, widening the type might improve
// performance.
//------------------------------------------------------------------------------
#if defined DIGIT_T && defined DDIGIT_T
using digit_t = DIGIT_T;
#else
using digit_t = std::uint32_t;
#endif
//------------------------------------------------------------------------------
static_assert(std::is_unsigned<digit_t>::value, "digit_t must be unsigned");
static_assert(!std::is_const<digit_t>::value, "digit_t must not be const");
static_assert(!std::is_same<bool, digit_t>::value, "digit_t must not be bool");
//------------------------------------------------------------------------------
constexpr unsigned bitsPerDigit = 8 * sizeof(digit_t);
//------------------------------------------------------------------------------
inline constexpr unsigned computeMaxDecDigitsPerDigit(digit_t val)
{
    return (val >= 10) ? 1 + computeMaxDecDigitsPerDigit(val / 10) : 0;
}
constexpr unsigned maxDecDigitsPerDigit =
    computeMaxDecDigitsPerDigit(std::numeric_limits<digit_t>::max());
//------------------------------------------------------------------------------
inline constexpr digit_t computeMaxPow10PerDigit(digit_t val)
{
    return (val >= 10) ? 10 * computeMaxPow10PerDigit(val / 10) : 1;
}
constexpr digit_t maxPow10PerDigit =
    computeMaxPow10PerDigit(std::numeric_limits<digit_t>::max());
//------------------------------------------------------------------------------
// The seven primitive operations on which all algorithms are based.
//------------------------------------------------------------------------------
/*
 * Computes a + b + carry.
 *
 * @param a      A digit.
 * @param b      A digit.
 * @param carry  The carry flag. Must be updated to reflect whether the addition
 *               overflowed.
 * @return       Returns the result of a + b + carry.
 */
digit_t addCarry(digit_t a, digit_t b, bool& carry);

/*
 * Computes a - b - borrow.
 *
 * @param a       A digit.
 * @param b       A digit.
 * @param borrow  The borrow flag. Must be updated to reflect whether the
 *                subtraction underflowed.
 * @return        Returns the result of a - b - borrow.
 */
digit_t subBorrow(digit_t a, digit_t b, bool& borrow);

/*
 * Computes a*b + carry.
 *
 * @param a      A digit.
 * @param b      A digit.
 * @param carry  The carry. Must be updated to the new carry value, which is
 *               (a*b + carry) / base.
 * @return       Returns (a*b + carry) % base.
 */
digit_t multiplyAdd(digit_t a, digit_t b, digit_t& carry);

/*
 * Computes a*b + c + carry.
 *
 * @param a      A digit.
 * @param b      A digit.
 * @param c      A digit.
 * @param carry  The carry. Must be updated to the new carry value, which is
 *               (a*b + c + carry) / base.
 * @return       Returns (a*b + c + carry) % base.
 */
digit_t multiplyAdd2(digit_t a, digit_t b, digit_t c, digit_t& carry);

/*
 * Computes (remainder*base + a) / b.
 *
 * The quotient will fit into one digit.
 *
 * @param a
 * @param b
 * @param remainder  The remainder. Must be updated to the new remainder value,
 *                   which is ((remainder*base + a) % b.
 * @return           Returns the quotient, which (remainder*base + a) / b.
 */
digit_t divideRemainder(digit_t a, digit_t b, digit_t& remainder);

/*
 * Counts the number of leading zero bits in the passed digit.
 *
 * @param val  A digit. Will not be 0.
 * @return     Returns the number of leading zero bits in the passed digit.
 */
std::size_t countLeadingZeroes(digit_t val);

/*
 * Counts the number of trailing zero bits in the passed digit.
 *
 * @param val  A digit. Will not be 0.
 * @return     Returns the number of trailing zero bits in the passed digit.
 */
std::size_t countTrailingZeroes(digit_t val);
//------------------------------------------------------------------------------
template<typename T>
typename std::enable_if<std::is_unsigned<T>::value, std::size_t>::type
    countLeadingZeroes(T val);
//------------------------------------------------------------------------------
template<typename T>
typename std::enable_if<std::is_unsigned<T>::value, std::size_t>::type
    countTrailingZeroes(T val);
//------------------------------------------------------------------------------
class Store;
//------------------------------------------------------------------------------
}  // namespace impl
//------------------------------------------------------------------------------
template<typename T>
struct EnableUserDefinedIntegral;
class Unsigned;
class Signed;
class Rational;
//------------------------------------------------------------------------------
namespace impl {
//------------------------------------------------------------------------------
// class Store
//------------------------------------------------------------------------------
class Store final
{
public:
    static constexpr std::size_t smemsize =
        (64 - 2 * sizeof(std::size_t) - sizeof(void*)) / sizeof(impl::digit_t);

public:
    Store() noexcept;
    Store(const Store& other);
    Store(Store&& other) noexcept;
    ~Store();

    Store& operator=(const Store& other);
    Store& operator=(Store&& other) noexcept;

public:
    std::size_t size() const noexcept;
    const impl::digit_t& operator[](std::size_t i) const noexcept;
    impl::digit_t& operator[](std::size_t i) noexcept;
    void resize(std::size_t newsize);

private:
    static void deallocate(void* ptr);
    static impl::digit_t* alloc_digits(std::size_t count);

private:
    impl::digit_t smem[smemsize];
    impl::digit_t* mem;
    std::size_t cap;
    std::size_t sz;
};
//------------------------------------------------------------------------------
}  // namespace impl
//------------------------------------------------------------------------------
/*******************************************************************************
 * A natural number of arbitrary precision.
 ******************************************************************************/
class Unsigned final
{
public:
    /**
     * Default constructor.
     *
     * The number is initialized to 0.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Unsigned() noexcept;

    /**
     * Copy constructor.
     *
     * @param other  The number to copy.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Unsigned(const Unsigned& other);

    /**
     * Move constructor.
     *
     * @param other  The number to move.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Unsigned(Unsigned&& other) noexcept;

    /**
     * Constructs a number from signed 32-bit integer.
     *
     * @param i  The integer to construct the number from.
     *
     * @exception std::invalid_argument  Thrown if i is negative.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Unsigned(std::int32_t i);

    /**
     * Constructs a number from an unsigned 32-bit integer.
     *
     * @param i  The unsigned integer to construct the number from.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Unsigned(std::uint32_t i);

    /**
     * Constructs a number from signed 64-bit integer.
     *
     * @param i  The integer to construct the number from. Will throw a
     *
     * @exception std::invalid_argument  Thrown if i is negative.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Unsigned(std::int64_t i);

    /**
     * Constructs a number from an unsigned 64-bit integer.
     *
     * @param i  The unsigned integer to construct the number from.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Unsigned(std::uint64_t i);

    /**
     * Constructs a number from a user-defined integral.
     *
     * @tparam T  The type to create the number from. The type must be right-
                  shiftable and castable to bn::impl::digit_t.
     * @param i   The integral to create the number from. Must not be negative.
     */
    template<
        typename T,
        typename std::enable_if<EnableUserDefinedIntegral<T>::value, bool>::
            type = true>
    explicit Unsigned(T i);

    /**
     * Constructs a number from a null-terminated string.
     *
     * @param dec  The null-terminated string to construct the number from.
     *
     * @exception std::invalid_argument  Thrown if the passed string is empty or
     *                                   contains a character that is not a
     *                                   digit from 0 to 9.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    Unsigned(const char* dec);

    /**
     * Copy assigns another number to this number.
     *
     * @param other  Another number.
     * @return       Returns a reference to this number.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Unsigned& operator=(const Unsigned& other);

    /**
     * Move assigns another number to this number.
     *
     * @param other  Another number.
     * @return       Returns a reference to this number.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Unsigned& operator=(Unsigned&& other) noexcept;

    /**
     * Generates a number consisting of the given number of random bits.
     *
     * The number is not guaranteed to have the given number of bits as the
     * random leading bits could be 0.
     *
     * @tparam Generator  The type of random engine to use (e.g. std::mt19937).
     * @param numBits     The number of random bits to generate.
     * @param gen         Reference to the random engine to use.
     * @return            Returns the generated random number.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    template<typename Generator>
    static Unsigned random(std::size_t numBits, Generator& gen);

public:
    /**
     * Pre-increment operator.
     *
     * Increases this number by 1.
     *
     * @return Returns a reference to this number.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Unsigned& operator++();

    /**
     * Post-increment operator.
     *
     * Increases this number by 1.
     *
     * @return Returns the value of the number before increasing.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Unsigned operator++(int);

    /**
     * Pre-decrement operator.
     *
     * Decreases this number by 1.
     *
     * @return Returns a reference to this number.
     *
     * @exception std::logic_error  Thrown if this number is 0.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Unsigned& operator--();

    /**
     * Post-decrement operator.
     *
     * Decreases this number by 1.
     *
     * @return Returns the value of the number before decreasing.
     *
     * @exception std::logic_error  Thrown if this number is 0.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Unsigned operator--(int);

    /**
     * Bitwise OR operator.
     *
     * @param v  Number to OR this number with.
     * @return   Returns a reference to this number.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Unsigned& operator|=(const Unsigned& v);

    /**
     * Bitwise AND operator.
     *
     * @param v  Number to AND this number with.
     * @return   Returns a reference to this number.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Unsigned& operator&=(const Unsigned& v);

    /**
     * Bitwise XOR operator.
     *
     * @param v  Number to XOR this number with.
     * @return   Returns a reference to this number.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Unsigned& operator^=(const Unsigned& v);

    /**
     * Bitwise left shift.
     *
     * @param s  The number of bits to shift to the left.
     * @return   Returns a reference to this number.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Unsigned& operator<<=(std::size_t s);

    /**
     * Bitwise right shift.
     *
     * @param s  The number of bits to shift to the left.
     * @return   Returns a reference to this number.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Unsigned& operator>>=(std::size_t s);

    /**
     * Adds the passed number to this number.
     *
     * @param v  The number to add.
     * @return   Returns a reference to this number.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Unsigned& operator+=(const Unsigned& v);

    /**
     * Substracts the passed number to this number.
     *
     * @param v  The number to subtract.
     * @return   Returns a reference to this number.
     *
     * @par  Runtime complexity
     *       O(n)
     *
     * @exception std::invalid_argument  Thrown if the v is greater than this
     *                                   number.
     */
    Unsigned& operator-=(const Unsigned& v);

    /**
     * Multiplies this number with the passed number.
     *
     * @param v  The number to multiply with.
     * @return   Returns a reference to this number.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    Unsigned& operator*=(const Unsigned& v);

    /**
     * Divides this number by the passed number.
     *
     * @param v  The number to divide by.
     * @return   Returns a reference to this number.
     *
     * @exception std::invalid_argument  Thrown if v is 0.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    Unsigned& operator/=(const Unsigned& v);

    /**
     * Computes the remainder when dividing this number by the passed number.
     *
     * @param v  The number to divide by.
     * @return   Returns a reference to this number.
     *
     * @exception std::invalid_argument  Thrown if v is 0.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    Unsigned& operator%=(const Unsigned& v);

    /**
     * Divides this number by the passed number and returns the remainder of the
     * division.
     *
     * @param v  The number to divide by.
     * @return   Returns the number of the division.
     *
     * @exception std::invalid_argument  Thrown if v is 0.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    Unsigned div(const Unsigned& v);

    /**
     * Checks whether this number is 0.
     *
     * @return  Returns true if this number is 0, false otherwise.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    bool empty() const;

    /**
     * Returns the number of bits this number consists of.
     *
     * The number of bits is equal to the position of the highest bit plus one.
     *
     * @return  Returns the number of bits this number consists of.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    std::size_t bits() const;

    /**
     * Returns the number of trailing zero bits.
     *
     * If this number is 0, the function will return 0.
     *
     * @return  Returns the number of trailing zero bits.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    std::size_t ctz() const;

    /**
     * Converts this number to an unsigned 64-bit integer.
     *
     * @return  This number as an unsigned 64-bit integer.
     *
     * @exception std::overflow_error  Thrown if this number does not fit in an
     *                                 unsigned 64-bit integer.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    explicit operator std::uint64_t() const;

    /**
     * Returns the string representation of this number in base 10.
     *
     * @return  Returns the string representation of this number in base 10.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    std::string str() const;

    /**
     * Returns the number of digits in this number, which is the number of
     * bn::impl::digit_t elements in this number.
     *
     * @return  Returns the number of digits in this number, which is the number
     *          of bn::impl::digit_t elements in this number.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    std::size_t digits() const;

public:
    struct QR;

private:
    template<int S, typename T>
    static typename std::enable_if<S >= 8 * sizeof(T), T>::type
        safeRightShift(T val);
    template<int S, typename T>
        static typename std::enable_if
        < S<8 * sizeof(T), T>::type safeRightShift(T val);
    template<typename T>
    void initFromIntegral(T val);

    template<std::size_t BPD, typename Generator>
    static typename std::enable_if<(BPD < 8 * sizeof(unsigned)), Unsigned>::type
        generateRandom(std::size_t numBits, Generator& gen);
    template<std::size_t BPD, typename Generator>
    static
        typename std::enable_if<(BPD == 8 * sizeof(unsigned)), Unsigned>::type
        generateRandom(std::size_t numBits, Generator& gen);
    template<std::size_t BPD, typename Generator>
    static typename std::enable_if<(BPD > 8 * sizeof(unsigned)), Unsigned>::type
        generateRandom(std::size_t numBits, Generator& gen);
    void randomGenMaskHighest(std::size_t mb);

    std::size_t countLeadingZeroes() const;

    void addDigit(impl::digit_t d);
    void subtractDigit(impl::digit_t d);
    void multiplyByDigit(impl::digit_t d);
    impl::digit_t divideByDigitReturnRem(impl::digit_t d);

    static impl::digit_t findDivQuotient(
        impl::digit_t un,
        impl::digit_t un1,
        impl::digit_t un2,
        impl::digit_t vn1,
        impl::digit_t vn2);
    static impl::digit_t findDivQuotient(
        const Unsigned& u,
        std::size_t lz,
        impl::digit_t vn1,
        impl::digit_t vn2,
        std::size_t j);

    void removeLeadingZeroDigits();

private:
    friend bool operator==(const Unsigned& u, const Unsigned& v);
    friend bool operator!=(const Unsigned& u, const Unsigned& v);
    friend bool operator<(const Unsigned& u, const Unsigned& v);
    friend bool operator>=(const Unsigned& u, const Unsigned& v);
    friend bool operator>(const Unsigned& u, const Unsigned& v);
    friend bool operator<=(const Unsigned& u, const Unsigned& v);

    friend Unsigned operator|(const Unsigned& u, const Unsigned& v);
    friend Unsigned operator&(const Unsigned& u, const Unsigned& v);
    friend Unsigned operator^(const Unsigned& u, const Unsigned& v);

    friend Unsigned operator<<(const Unsigned& u, std::size_t s);
    friend Unsigned operator>>(const Unsigned& u, std::size_t s);

    friend Unsigned operator+(const Unsigned& u, const Unsigned& v);
    friend Unsigned operator-(const Unsigned& u, const Unsigned& v);
    friend Unsigned operator*(const Unsigned& u, const Unsigned& v);
    friend Unsigned operator/(const Unsigned& u, const Unsigned& v);
    friend Unsigned operator%(const Unsigned& u, const Unsigned& v);

    friend Unsigned::QR div(const Unsigned& u, const Unsigned& v);

    friend Unsigned
        powmod(const Unsigned& u, Unsigned exp, const Unsigned& mod);

    friend class Rational;

private:
    impl::Store digit;
};
/*******************************************************************************
 * Result of a division.
 ******************************************************************************/
struct Unsigned::QR
{
    /// The quotient.
    Unsigned quot;
    /// The remainder.
    Unsigned rem;
};

/**
 * Equal comparison.
 *
 * @param u  First number.
 * @param v  Second number.
 * @return   Returns true if the numbers are equal, false otherwise.
 *
 * @par  Runtime complexity
 *       O(n)
 */
bool operator==(const Unsigned& u, const Unsigned& v);

/**
 * Inequal comparison.
 *
 * @param u  First number.
 * @param v  Second number.
 * @return   Returns true if the number are not equal, false otherwise.
 *
 * @par  Runtime complexity
 *       O(n)
 */
bool operator!=(const Unsigned& u, const Unsigned& v);

/**
 * Less than comparison.
 *
 * @param u  First number.
 * @param v  Second number.
 * @return   Returns true if the first number is less than the second number,
 *           false otherwise.
 *
 * @par  Runtime complexity
 *       O(n)
 */
bool operator<(const Unsigned& u, const Unsigned& v);

/**
 * Greater than or equal comparison.
 *
 * @param u  First number.
 * @param v  Second number.
 * @return   Returns true if the first number is greater than or equal to the
 *           second number, false otherwise.
 *
 * @par  Runtime complexity
 *       O(n)
 */
bool operator>=(const Unsigned& u, const Unsigned& v);

/**
 * Greater than comparison.
 *
 * @param u  First number.
 * @param v  Second number.
 * @return   Returns true if the first number is greater than the second number,
 *           false otherwise.
 *
 * @par  Runtime complexity
 *       O(n)
 */
bool operator>(const Unsigned& u, const Unsigned& v);

/**
 * Less than or equal comparison.
 *
 * @param u  First number.
 * @param v  Second number.
 * @return   Returns true if the first number is less than or equal to the
 *           second number, false otherwise.
 *
 * @par  Runtime complexity
 *       O(n)
 */
bool operator<=(const Unsigned& u, const Unsigned& v);

/**
 * Bitwise OR operator.
 *
 * @param u  First number.
 * @param v  Second number.
 * @return   Returns the result of a bitwise OR of both numbers.
 *
 * @par  Runtime complexity
 *       O(n)
 */
Unsigned operator|(const Unsigned& u, const Unsigned& v);

/**
 * Bitwise AND operator.
 *
 * @param u  First number.
 * @param v  Second number.
 * @return   Returns the result of a bitwise AND of both numbers.
 *
 * @par  Runtime complexity
 *       O(n)
 */
Unsigned operator&(const Unsigned& u, const Unsigned& v);

/**
 * Bitwise XOR operator.
 *
 * @param u  First number.
 * @param v  Second number.
 * @return   Returns the result of a bitwise XOR of both numbers.
 *
 * @par  Runtime complexity
 *       O(n)
 */
Unsigned operator^(const Unsigned& u, const Unsigned& v);

/**
 * Bitwise left shift operator.
 *
 * @param u  The number to shift.
 * @param s  The number of bits to shift to the left.
 * @return   Returns the shifted number.
 *
 * @par  Runtime complexity
 *       O(n)
 */
Unsigned operator<<(const Unsigned& u, std::size_t s);

/**
 * Bitwise right shift operator.
 *
 * @param u  The number to shift.
 * @param s  The number of bits to shift to the right.
 * @return   Returns the shifted number.
 *
 * @par  Runtime complexity
 *       O(n)
 */
Unsigned operator>>(const Unsigned& u, std::size_t s);

/**
 * Adds two numbers.
 *
 * @param u  First summand.
 * @param v  Second summand.
 * @return   Returns the sum.
 *
 * @par  Runtime complexity
 *       O(n)
 */
Unsigned operator+(const Unsigned& u, const Unsigned& v);

/**
 * Subtracts a number from a number.
 *
 * @param u  Minuend.
 * @param v  Subtrahend.
 * @return   Returns the difference.
 *
 * @exception std::invalid_argument  Thrown if the subtrahend is greater than
 *                                   the minuend.
 *
 * @par  Runtime complexity
 *       O(n)
 */
Unsigned operator-(const Unsigned& u, const Unsigned& v);

/**
 * Multiplies two numbers with each other.
 *
 * @param u  First factor.
 * @param v  Second factor.
 * @return   Returns the product.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
Unsigned operator*(const Unsigned& u, const Unsigned& v);

/**
 * Divides a number by another.
 *
 * @param u  Divident.
 * @param v  Divisor.
 * @return   Returns the quotient.
 *
 * @exception std::invalid_argument  Thrown if the divisor is 0.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
Unsigned operator/(const Unsigned& u, const Unsigned& v);

/**
 * Computes the remainder of a division.
 *
 * @param u  Divident.
 * @param v  Divisor.
 * @return   Returns the remainder.
 *
 * @exception std::invalid_argument  Thrown if the divisor is 0.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
Unsigned operator%(const Unsigned& u, const Unsigned& v);

/**
 * Divides a number by another.
 *
 * @param u  Divident.
 * @param v  Divisor.
 * @return   Returns the quotient and remainder.
 *
 * @exception std::invalid_argument  Thrown if the divisor is 0.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
Unsigned::QR div(const Unsigned& u, const Unsigned& v);

/**
 * Computes a power using fast exponentation.
 *
 * @param u    The base.
 * @param exp  The exponent.
 * @return     Returns the power.
 *
 * @par  Runtime complexity
 *       O(n^2*exp^3)
 */
Unsigned pow(const Unsigned& u, std::size_t exp);

/**
 * Computes a power modulo a number using fast exponentation.
 *
 * @param u    The base.
 * @param exp  The exponent.
 * @param mod  The modulus.
 * @return     Returns the power.
 *
 * @par  Runtime complexity
 *       O(exp*mod^2)
 */
Unsigned powmod(const Unsigned& u, Unsigned exp, const Unsigned& mod);

/**
 * Computes the rounded-down square root.
 *
 * @param u  A number.
 * @return   The rounded-down square root.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
Unsigned sqrt(const Unsigned& u);

/**
 * Computes the greatest common divisor of two numbers using the Euclidean
 * algorithm.
 *
 * If one of the numbers is 0, the other number is returned.
 *
 * @param u  The first number.
 * @param v  The seond number.
 * @return   Returns the greatest common divisor.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
Unsigned egcd(const Unsigned& u, const Unsigned& v);

/**
 * Computes the greatest common divisor of two numbers using a binary algorithm.
 *
 * If one of the numbers is 0, the other number is returned.
 *
 * @param u  The first number.
 * @param v  The seond number.
 * @return   Returns the greatest common divisor.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
Unsigned bgcd(const Unsigned& u, const Unsigned& v);

/**
 * Computes the greatest common divisor of two numbers.
 *
 * If one of the numbers is 0, the other number is returned. This function uses
 * the binary algorithm.
 *
 * @param u  The first number.
 * @param v  The seond number.
 * @return   Returns the greatest common divisor.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
Unsigned gcd(const Unsigned& u, const Unsigned& v);

/**
 * Writes a number in base 10 to an output stream.
 *
 * @param out  An output stream.
 * @param u    The number.
 * @return     Returns the output stream.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
std::ostream& operator<<(std::ostream& out, const Unsigned& u);

/**
 * Writes a quotient and remainder to an output stream.
 *
 * @param out  An output stream.
 * @param qr   The quotient and remainder.
 * @return     Returns the output stream.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
std::ostream& operator<<(std::ostream& out, const Unsigned::QR& qr);

/*******************************************************************************
 * An integer of arbitrary precision.
 ******************************************************************************/
class Signed
{
public:
    /**
     * Default constructor.
     *
     * Initializes the integer to 0.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Signed() noexcept;

    /**
     * Copy constructor.
     *
     * @param other  The integer to copy.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Signed(const Signed& other);

    /**
     * Move constructor.
     *
     * @param other  The integer to move.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Signed(Signed&& other) noexcept;

    /**
     * Copy constructor from a natural number.
     *
     * @param other  The natural number to copy.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Signed(const Unsigned& other);

    /**
     * Move constructor from a natural number.
     *
     * @param other  The natural number to move.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Signed(Unsigned&& other) noexcept;

    /**
     * Constructs an integer from a signed 32-bit integer.
     *
     * @param i  The signed integer to construct the number from.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Signed(std::int32_t i);

    /**
     * Constructs an integer from an unsigned 32-bit integer.
     *
     * @param i  The unsigned integer to construct the number from.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Signed(std::uint32_t i);

    /**
     * Constructs an integer from a signed 64-bit integer.
     *
     * @param i  The signed integer to construct the number from.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Signed(std::int64_t i);

    /**
     * Constructs an integer from an unsigned 64-bit integer.
     *
     * @param i  The unsigned integer to construct the number from.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Signed(std::uint64_t i);

    /**
     * Constructs an integer from a null-terminated string.
     *
     * @param dec  The null-terminated string to construct the integer from.
     *
     * @exception std::invalid_argument  Thrown if the string is empty or does
     *                                   not match the regular expression
     *                                   -?[0-9]+.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    Signed(const char* dec);

    /**
     * Copy assignment.
     *
     * @param other  The integer to assign.
     * @return       Returns a reference to this integer.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Signed& operator=(const Signed& other);

    /**
     * Move assignment.
     *
     * @param other  The integer to assign.
     * @return       Returns a reference to this integer.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Signed& operator=(Signed&& other) noexcept;

public:
    /**
     * Returns the sign of this integer.
     *
     * @return  Returns -1 if the integer is negative, 1 if the integer is
     *          positive and 0 if the integer is 0.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    int sgn() const;

    /**
     * Returns the absolute value of this integer as natural number.
     *
     * @return  Return the absolute value of this integer.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    const Unsigned& abs() const;

    /**
     * Pre-increment operator.
     *
     * Increases this integer by 1.
     *
     * @return  Returns a reference to this integer.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Signed& operator++();

    /**
     * Post-increment operator.
     *
     * Increases this integer by 1.
     *
     * @return  Returns the value of the integer before increasing.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Signed operator++(int);

    /**
     * Pre-decrement operator.
     *
     * Decreases this integer by 1.
     *
     * @return  Returns a reference to this integer.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Signed& operator--();

    /**
     * Post-decrement operator.
     *
     * Decreases this integer by 1.
     *
     * @return  Returns the value of the integer before decreasing.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Signed operator--(int);

    /**
     * Adds the passed integer to this integer.
     *
     * @param v  The integer to add.
     * @return   Returns a reference to this integer.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Signed& operator+=(const Signed& v);

    /**
     * Substracts the passed integer to this integer.
     *
     * @param v  The integer to subtract.
     * @return   Returns a reference to this integer.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Signed& operator-=(const Signed& v);

    /**
     * Multiplies this integer with the passed integer.
     *
     * @param v  The integer to multiply with.
     * @return   Returns a reference to this integer.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    Signed& operator*=(const Signed& v);

    /**
     * Divides this integer by the passed integer.
     *
     * @param v  The integer to divide by.
     * @return   Returns a reference to this integer.
     *
     * @exception std::invalid_argument  Thrown if v is 0.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    Signed& operator/=(const Signed& v);

    /**
     * Computes the remainder when dividing this integer by the passed integer.
     *
     * @param v  The integer to divide by.
     * @return   Returns a reference to this integer.
     *
     * @exception std::invalid_argument  Thrown if v is 0.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    Signed& operator%=(const Signed& v);

    /**
     * Divides this integer by the passed integer and returns the remainder of
     * the division.
     *
     * @param v  The integer to divide by.
     * @return   Returns the integer of the division.
     *
     * @exception std::invalid_argument  Thrown if v is 0.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    Signed div(const Signed& v);

    /**
     * Returns the string representation of this integer in base 10.
     *
     * @return  Returns the string representation of this integer in base 10.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    std::string str() const;

public:
    struct QR;

private:
    friend bool operator==(const Signed& u, const Signed& v);
    friend bool operator!=(const Signed& u, const Signed& v);
    friend bool operator<(const Signed& u, const Signed& v);
    friend bool operator>=(const Signed& u, const Signed& v);
    friend bool operator>(const Signed& u, const Signed& v);
    friend bool operator<=(const Signed& u, const Signed& v);

    friend Signed operator-(const Signed& u);

    friend Signed operator+(const Signed& u, const Signed& v);
    friend Signed operator-(const Signed& u, const Signed& v);
    friend Signed operator*(const Signed& u, const Signed& v);
    friend Signed operator/(const Signed& u, const Signed& v);
    friend Signed operator%(const Signed& u, const Signed& v);

    friend QR div(const Signed& u, const Signed& v);

    friend std::ostream& operator<<(std::ostream& out, const Signed& s);

    friend class Rational;
    friend Rational operator/(const Rational& u, const Rational& v);

private:
    Unsigned val;
    int8_t sign;
};

/*******************************************************************************
 * Result of an integer division.
 ******************************************************************************/
struct Signed::QR
{
    /// The quotient.
    Signed quot;
    /// The remainder. The remainder will have the same sign as the divident.
    Signed rem;
};

/**
 * Equal comparison.
 *
 * @param u  First integer.
 * @param v  Second integer.
 * @return   Returns true if the integers are equal, false otherwise.
 *
 * @par  Runtime complexity
 *       O(n)
 */
bool operator==(const Signed& u, const Signed& v);

/**
 * Inequal comparison.
 *
 * @param u  First integer.
 * @param v  Second integer.
 * @return   Returns true if the integer are not equal, false otherwise.
 *
 * @par  Runtime complexity
 *       O(n)
 */
bool operator!=(const Signed& u, const Signed& v);

/**
 * Less than comparison.
 *
 * @param u  First integer.
 * @param v  Second integer.
 * @return   Returns true if the first integer is less than the second integer,
 *           false otherwise.
 *
 * @par  Runtime complexity
 *       O(n)
 */
bool operator<(const Signed& u, const Signed& v);

/**
 * Greater than or equal comparison.
 *
 * @param u  First integer.
 * @param v  Second integer.
 * @return   Returns true if the first integer is greater than or equal to the
 *           second integer, false otherwise.
 *
 * @par  Runtime complexity
 *       O(n)
 */
bool operator>=(const Signed& u, const Signed& v);

/**
 * Greater than comparison.
 *
 * @param u  First integer.
 * @param v  Second integer.
 * @return   Returns true if the first integer is greater than the second
 *           integer, false otherwise.
 *
 * @par  Runtime complexity
 *       O(n)
 */
bool operator>(const Signed& u, const Signed& v);

/**
 * Less than or equal comparison.
 *
 * @param u  First integer.
 * @param v  Second integer.
 * @return   Returns true if the first integer is less than or equal to the
 *           second integer, false otherwise.
 *
 * @par  Runtime complexity
 *       O(n)
 */
bool operator<=(const Signed& u, const Signed& v);

/**
 * Negates an integer.
 *
 * @param u  The integer to negate.
 * @return   The negated integer.
 *
 * @par  Runtime complexity
 *       O(n)
 */
Signed operator-(const Signed& u);

/**
 * Adds two integers.
 *
 * @param u  First summand.
 * @param v  Second summand.
 * @return   Returns the sum.
 *
 * @par  Runtime complexity
 *       O(n)
 */
Signed operator+(const Signed& u, const Signed& v);

/**
 * Subtracts an integer from an integer.
 *
 * @param u  Minuend.
 * @param v  Subtrahend.
 * @return   Returns the difference.
 *
 * @par  Runtime complexity
 *       O(n)
 */
Signed operator-(const Signed& u, const Signed& v);

/**
 * Multiplies two integers with each other.
 *
 * @param u  First factor.
 * @param v  Second factor.
 * @return   Returns the product.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
Signed operator*(const Signed& u, const Signed& v);

/**
 * Divides an integer by another.
 *
 * @param u  Divident.
 * @param v  Divisor.
 * @return   Returns the quotient.
 *
 * @exception std::invalid_argument  Thrown if the divisor is 0.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
Signed operator/(const Signed& u, const Signed& v);

/**
 * Computes the remainder of a division.
 *
 * @param u  Divident.
 * @param v  Divisor. if the divisor is 0.
 * @return   Returns the remainder.
 *
 * @exception std::invalid_argument  Thrown if the divisor is 0.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
Signed operator%(const Signed& u, const Signed& v);

/**
 * Divides an integer by another.
 *
 * @param u  Divident.
 * @param v  Divisor.
 * @return   Returns the quotient and remainder.
 *
 * @exception std::invalid_argument  Thrown if the divisor is 0.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
Signed::QR div(const Signed& u, const Signed& v);

/**
 * Writes an integer in base 10 to an output stream.
 *
 * @param out  An output stream.
 * @param s    An integer.
 * @return     Returns the output stream.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
std::ostream& operator<<(std::ostream& out, const Signed& s);

/*******************************************************************************
 * A rational number.
 *
 * The numerator is an integer and the denominator a natural number, which must
 * not be 0.
 *
 * This class maintains the invariant that the greatest common divisor of
 * numerator and denominator is always 1. This requires a reduction step after
 * most operations. As the reduction costs O(n^2), the costs of those operations
 * is at least O(n^2).
 ******************************************************************************/
class Rational
{
public:
    /**
     * Constructor.
     *
     * Initializes the value to 0.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Rational() noexcept;

    /**
     * Constructor.
     *
     * The rational number will be automically reduced.
     *
     * @param num  The numerator.
     * @param den  The denominator.
     *
     * @exception std::invalid_argument  Thrown if the denominator is 0.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    Rational(const Signed& num, const Unsigned& den);

    /**
     * Constructor.
     *
     * The rational number will be automically reduced.
     *
     * @param num  The numerator.
     * @param den  The denominator.
     *
     * @exception std::invalid_argument  Thrown if the denominator is 0.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    Rational(Signed&& num, Unsigned&& den);

    /**
     * Constructs a rational from a double-precision floating point number.
     *
     * The rational will have exactly the same value as the floating point
     * number.
     *
     * @param d  A double-precision floating point number.
     *
     * @exception  std::invalid_argument  Thrown if the passed floating point
     *                                    number is not finite.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Rational(double d);

    /**
     * Constructor.
     *
     * @param v  The value this rational is initialized to.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Rational(const Unsigned& v);

    /**
     * Constructor.
     *
     * @param v  The value this rational is initialized to.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Rational(Unsigned&& v);

    /**
     * Constructor.
     *
     * @param v  The value this rational is initialized to.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Rational(const Signed& v);

    /**
     * Constructor.
     *
     * @param v  The value this rational is initialized to.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Rational(Signed&& v);

    /**
     * Copy constructor.
     *
     * @param other  A rational number.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Rational(const Rational& other) = default;

    /**
     * Move constructor.
     *
     * @param other  A rational number.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Rational(Rational&& other) noexcept = default;

    /**
     * Copy assignment.
     *
     * @param other  A rational number.
     * @return       Returns a reference to this rational number.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Rational& operator=(const Rational& other) = default;

    /**
     * Move assignment.
     *
     * @param other  A rational number.
     * @return       Returns a reference to this rational number.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    Rational& operator=(Rational&& other) noexcept = default;

public:
    /**
     * Returns the numerator.
     *
     * @return  Returns the numerator.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    const Signed& numerator() const;

    /**
     * Returns the denominator.
     *
     * @return  Returns the denominator.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    const Unsigned& denominator() const;

    /**
     * Returns the reciprocal of this rational number.
     *
     * @return  Returns the reciprocal of this rational number.
     *
     * @exception std::logic_error  Thrown if the numerator is 0.
     *
     * @par  Runtime complexity
     *       O(n)
     */
    Rational reciprocal() const;

    /**
     * Adds the passed rational number to this rational number.
     *
     * @param v  The rational number to add.
     * @return   Returns a reference to this rational number.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    Rational& operator+=(const Rational& v);

    /**
     * Substracts the passed rational number to this rational number.
     *
     * @param v  The rational number to subtract.
     * @return   Returns a reference to this rational number.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    Rational& operator-=(const Rational& v);

    /**
     * Multiplies this rational number with the passed rational number.
     *
     * @param v  The rational number to multiply with.
     * @return   Returns a reference to this rational number.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    Rational& operator*=(const Rational& v);

    /**
     * Divides this rational number by the passed rational number.
     *
     * @param v  The rational number to divide by. Will throw an
     *           std::invalid_argument exception if the rational number is 0.
     * @return   Returns a reference to this rational number.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    Rational& operator/=(const Rational& v);

    /**
     * Converts this rational number to the closest double-precision floating
     * point number.
     *
     * The convertion will use the round to nearest rounding mode. Ties will be
     * rounded to the nearest even digit.
     *
     * If this rational number is too huge to be converted to double-precision
     * floating point number, infinity is returned.
     *
     * @return  Returns the closest double-precision floating point number of
     *          this rational number.
     *
     * @par  Runtime complexity
     *       O(1)
     */
    explicit operator double() const;

    /**
     * Returns a string representation of this rational number to base 10 in
     * format numerator "/" denominator.
     *
     * @return  Returns a string representation of this rational number to base
     * 10 in format numerator "/" denominator.
     *
     * @par  Runtime complexity
     *       O(n^2)
     */
    std::string str() const;

private:
    void reduce();

private:
    friend bool operator==(const Rational& u, const Rational& v);
    friend bool operator!=(const Rational& u, const Rational& v);
    friend bool operator<(const Rational& u, const Rational& v);
    friend bool operator>=(const Rational& u, const Rational& v);
    friend bool operator>(const Rational& u, const Rational& v);
    friend bool operator<=(const Rational& u, const Rational& v);

    friend Rational operator-(const Rational& u);

    friend Rational operator+(const Rational& u, const Rational& v);
    friend Rational operator-(const Rational& u, const Rational& v);
    friend Rational operator*(const Rational& u, const Rational& v);
    friend Rational operator/(const Rational& u, const Rational& v);

    friend std::ostream& operator<<(std::ostream& out, const Rational& u);

private:
    Signed num;
    Unsigned den;
};

/**
 * Equal comparison.
 *
 * @param u  First rational number.
 * @param v  Second rational number.
 * @return   Returns true if the rational numbers are equal, false otherwise.
 *
 * @par  Runtime complexity
 *       O(n)
 */
bool operator==(const Rational& u, const Rational& v);

/**
 * Inequal comparison.
 *
 * @param u  First rational number.
 * @param v  Second rational number.
 * @return   Returns true if the rational number are not equal, false otherwise.
 *
 * @par  Runtime complexity
 *       O(n)
 */
bool operator!=(const Rational& u, const Rational& v);

/**
 * Less than comparison.
 *
 * @param u  First rational number.
 * @param v  Second rational number.
 * @return   Returns true if the first rational number is less than the second
 *           rational number, false otherwise.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
bool operator<(const Rational& u, const Rational& v);

/**
 * Greater than or equal comparison.
 *
 * @param u  First rational number.
 * @param v  Second rational number.
 * @return   Returns true if the first rational number is greater than or equal
 *           to the second rational number, false otherwise.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
bool operator>=(const Rational& u, const Rational& v);

/**
 * Greater than comparison.
 *
 * @param u  First rational number.
 * @param v  Second rational number.
 * @return   Returns true if the first rational number is greater than the
 *           second rational number, false otherwise.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
bool operator>(const Rational& u, const Rational& v);

/**
 * Less than or equal comparison.
 *
 * @param u  First rational number.
 * @param v  Second rational number.
 * @return   Returns true if the first rational number is less than or equal to
 *           the second rational number, false otherwise.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
bool operator<=(const Rational& u, const Rational& v);

/**
 * Negates a rational number.
 *
 * @param u  The rational number to negate.
 * @return   The negated rational number.
 *
 * @par  Runtime complexity
 *       O(n)
 */
Rational operator-(const Rational& u);

/**
 * Adds two rational numbers.
 *
 * @param u  First summand.
 * @param v  Second summand.
 * @return   Returns the sum.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
Rational operator+(const Rational& u, const Rational& v);

/**
 * Subtracts a rational number from a rational number.
 *
 * @param u  Minuend.
 * @param v  Subtrahend.
 * @return   Returns the difference.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
Rational operator-(const Rational& u, const Rational& v);

/**
 * Multiplies two rational numbers with each other.
 *
 * @param u  First factor.
 * @param v  Second factor.
 * @return   Returns the product.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
Rational operator*(const Rational& u, const Rational& v);

/**
 * Divides a rational number by another.
 *
 * @param u  Divident.
 * @param v  Divisor. if the divisor is 0, a std::invalid_argument execption is
 *           thrown.
 * @return   Returns the quotient.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
Rational operator/(const Rational& u, const Rational& v);

/**
 * Writes a rational number in base 10 to an output stream.
 *
 * The format of the number ist numerator "/" denominator.
 *
 * @param out  An output stream.
 * @param u    A rational number.
 * @return     Returns the output stream.
 *
 * @par  Runtime complexity
 *       O(n^2)
 */
std::ostream& operator<<(std::ostream& out, const Rational& u);

//------------------------------------------------------------------------------
namespace impl {
//------------------------------------------------------------------------------
inline Store::Store() noexcept : mem(smem), cap(smemsize), sz(0)
{
}
//------------------------------------------------------------------------------
inline Store::Store(const Store& other)
{
    if (other.sz <= smemsize) {
        memcpy(smem, other.mem, other.sz * sizeof(impl::digit_t));
        mem = smem;
        cap = smemsize;
        sz = other.sz;
    } else {
        mem = alloc_digits(other.sz);
        memcpy(mem, other.mem, other.sz * sizeof(impl::digit_t));
        cap = other.sz;
        sz = other.sz;
    }
}
//------------------------------------------------------------------------------
inline Store::Store(Store&& other) noexcept
{
    if (other.cap == smemsize) {
        memcpy(smem, other.smem, other.sz * sizeof(impl::digit_t));
        mem = smem;
        cap = other.cap;
        sz = other.sz;
    } else {
        mem = other.mem;
        cap = other.cap;
        sz = other.sz;
        other.mem = other.smem;
        other.cap = smemsize;
        other.sz = 0;
    }
}
//------------------------------------------------------------------------------
inline Store::~Store()
{
    if (cap != smemsize) {
        deallocate(mem);
    }
}
//------------------------------------------------------------------------------
inline Store& Store::operator=(const Store& other)
{
    if (other.sz <= smemsize) {
        if (cap >= (2 * smemsize)) {
            deallocate(mem);
            mem = smem;
            cap = smemsize;
        }
    } else if (other.sz <= cap) {
        if (cap >= (2 * other.sz)) {
            impl::digit_t* temp = alloc_digits(other.sz);
            deallocate(mem);
            mem = temp;
            cap = other.sz;
        }
    } else {
        impl::digit_t* temp = alloc_digits(other.sz);
        if (cap != smemsize) {
            deallocate(mem);
        }
        mem = temp;
    }
    memcpy(mem, other.mem, other.sz * sizeof(impl::digit_t));
    sz = other.sz;
    return *this;
}
//------------------------------------------------------------------------------
inline Store& Store::operator=(Store&& other) noexcept
{
    if (other.cap == smemsize) {
        memcpy(smem, other.smem, other.sz * sizeof(impl::digit_t));
        if (cap == smemsize) {
            sz = other.sz;
        } else {
            other.mem = mem;
            mem = smem;
            other.cap = cap;
            cap = smemsize;
            std::swap(sz, other.sz);
        }
    } else {
        if (cap == smemsize) {
            memcpy(other.smem, smem, sz * sizeof(impl::digit_t));
            mem = other.mem;
            other.mem = other.smem;
            cap = other.cap;
            other.cap = smemsize;
            std::swap(sz, other.sz);
        } else {
            std::swap(mem, other.mem);
            std::swap(cap, other.cap);
            std::swap(sz, other.sz);
        }
    }
    return *this;
}
//------------------------------------------------------------------------------
inline std::size_t Store::size() const noexcept
{
    return sz;
}
//------------------------------------------------------------------------------
inline const impl::digit_t& Store::operator[](std::size_t i) const noexcept
{
    assert(i < sz);
    return mem[i];
}
//------------------------------------------------------------------------------
inline impl::digit_t& Store::operator[](std::size_t i) noexcept
{
    assert(i < sz);
    return mem[i];
}
//------------------------------------------------------------------------------
inline void Store::resize(std::size_t newsize)
{
    if (newsize <= smemsize) {
        if (cap >= (2 * smemsize)) {
            memcpy(smem, mem, newsize * sizeof(impl::digit_t));
            deallocate(mem);
            mem = smem;
            cap = smemsize;
        }
        sz = newsize;
    } else if (newsize <= cap) {
        if (cap >= (2 * newsize)) {
            impl::digit_t* temp = alloc_digits(newsize);
            memcpy(temp, mem, newsize * sizeof(impl::digit_t));
            deallocate(mem);
            mem = temp;
            cap = newsize;
        }
        sz = newsize;
    } else {
        impl::digit_t* temp = alloc_digits(newsize);
        memcpy(temp, mem, sz * sizeof(impl::digit_t));
        if (cap != smemsize) {
            deallocate(mem);
        }
        mem = temp;
        cap = newsize;
        sz = newsize;
    }
}
//------------------------------------------------------------------------------
inline void Store::deallocate(void* ptr)
{
    std::free(ptr);
}
//------------------------------------------------------------------------------
inline impl::digit_t* Store::alloc_digits(std::size_t count)
{
    constexpr std::size_t maxDigits =
        static_cast<std::size_t>(std::numeric_limits<ptrdiff_t>::max())
        / impl::bitsPerDigit;
    if (count > maxDigits) {
        throw std::bad_alloc();
    }
    const size_t numBytes = count * sizeof(impl::digit_t);
    void* mem = std::malloc(numBytes);
    if (mem == nullptr) {
        throw std::bad_alloc();
    }
    return static_cast<impl::digit_t*>(mem);
}
//------------------------------------------------------------------------------
}  // namespace impl
//------------------------------------------------------------------------------
inline Unsigned::Unsigned() noexcept
{
}
//------------------------------------------------------------------------------
inline Unsigned::Unsigned(const Unsigned& other) : digit(other.digit)
{
}
//------------------------------------------------------------------------------
inline Unsigned::Unsigned(Unsigned&& other) noexcept
    : digit(std::move(other.digit))
{
}
//------------------------------------------------------------------------------
inline Unsigned::Unsigned(std::int32_t i)
{
    if (i < 0) {
        throw std::invalid_argument("i is negative");
    }
    initFromIntegral(static_cast<std::uint32_t>(i));
}
//------------------------------------------------------------------------------
inline Unsigned::Unsigned(std::uint32_t i)
{
    initFromIntegral(i);
}
//------------------------------------------------------------------------------
inline Unsigned::Unsigned(std::int64_t i)
{
    if (i < 0) {
        throw std::invalid_argument("i is negative");
    }
    initFromIntegral(static_cast<std::uint64_t>(i));
}
//------------------------------------------------------------------------------
inline Unsigned::Unsigned(std::uint64_t i)
{
    initFromIntegral(i);
}
//------------------------------------------------------------------------------
template<
    typename T,
    typename std::enable_if<EnableUserDefinedIntegral<T>::value, bool>::type>
Unsigned::Unsigned(T i)
{
    initFromIntegral(i);
}
//------------------------------------------------------------------------------
inline Unsigned::Unsigned(const char* dec)
{
    if (*dec == '\0') {
        throw std::invalid_argument("dec is empty");
    }

    while (*dec) {
        const char* end = dec;
        std::size_t count = 0;
        while (*end && (count < impl::maxDecDigitsPerDigit)) {
            ++count;
            ++end;
        }
        impl::digit_t mul;
        if (count == impl::maxDecDigitsPerDigit) {
            mul = impl::maxPow10PerDigit;
        } else {
            mul = 1;
            for (std::size_t i = 0; i < count; ++i) {
                mul *= 10;
            }
        }
        multiplyByDigit(mul);
        impl::digit_t add = 0;
        for (const char* curr = dec; curr != end; ++curr) {
            if ((*curr < '0') || (*curr > '9')) {
                throw std::invalid_argument("invalid digit in string");
            }
            add = 10 * add + (*curr - '0');
        }
        addDigit(add);
        dec = end;
    }
}
//------------------------------------------------------------------------------
inline Unsigned& Unsigned::operator=(const Unsigned& other)
{
    digit = other.digit;
    return *this;
}
//------------------------------------------------------------------------------
inline Unsigned& Unsigned::operator=(Unsigned&& other) noexcept
{
    digit = std::move(other.digit);
    return *this;
}
//------------------------------------------------------------------------------
template<typename Generator>
Unsigned Unsigned::random(std::size_t numBits, Generator& gen)
{
    if (numBits == 0) {
        return Unsigned();
    }
    return generateRandom<impl::bitsPerDigit>(numBits, gen);
}
//------------------------------------------------------------------------------
inline Unsigned& Unsigned::operator++()
{
    addDigit(1);
    return *this;
}
//------------------------------------------------------------------------------
inline Unsigned Unsigned::operator++(int)
{
    Unsigned ret = *this;
    ++(*this);
    return ret;
}
//------------------------------------------------------------------------------
inline Unsigned& Unsigned::operator--()
{
    subtractDigit(1);
    return *this;
}
//------------------------------------------------------------------------------
inline Unsigned Unsigned::operator--(int)
{
    Unsigned ret = *this;
    --(*this);
    return ret;
}
//------------------------------------------------------------------------------
inline Unsigned& Unsigned::operator|=(const Unsigned& v)
{
    const std::size_t n = digit.size();
    const std::size_t m = v.digit.size();
    if (n < m) {
        digit.resize(m);
        for (std::size_t i = 0; i < n; ++i) {
            digit[i] |= v.digit[i];
        }
        for (std::size_t i = n; i < m; ++i) {
            digit[i] = v.digit[i];
        }
    } else {
        for (std::size_t i = 0; i < m; ++i) {
            digit[i] |= v.digit[i];
        }
    }
    return *this;
}
//------------------------------------------------------------------------------
inline Unsigned& Unsigned::operator&=(const Unsigned& v)
{
    const std::size_t n = std::min(digit.size(), v.digit.size());
    digit.resize(n);
    for (std::size_t i = 0; i < n; ++i) {
        digit[i] &= v.digit[i];
    }
    removeLeadingZeroDigits();
    return *this;
}
//------------------------------------------------------------------------------
inline Unsigned& Unsigned::operator^=(const Unsigned& v)
{
    const std::size_t n = digit.size();
    const std::size_t m = v.digit.size();
    if (n < m) {
        digit.resize(m);
        for (std::size_t i = 0; i < n; ++i) {
            digit[i] ^= v.digit[i];
        }
        for (std::size_t i = n; i < m; ++i) {
            digit[i] = v.digit[i];
        }
    } else {
        for (std::size_t i = 0; i < m; ++i) {
            digit[i] ^= v.digit[i];
        }
    }
    removeLeadingZeroDigits();
    return *this;
}
//------------------------------------------------------------------------------
inline Unsigned& Unsigned::operator<<=(std::size_t s)
{
    if (s == 0) {
        return *this;
    }
    const std::size_t n = digit.size();
    if (n == 0) {
        return *this;
    }
    const std::size_t ds = s / impl::bitsPerDigit;
    const std::size_t lbs = s % impl::bitsPerDigit;
    if (lbs == 0) {
        digit.resize(n + ds);
        for (size_t i = 0; i < n; ++i) {
            digit[i + ds] = digit[i];
        }
        for (size_t i = 0; i < ds; ++i) {
            digit[i] = 0;
        }
        return *this;
    }
    const std::size_t rbs = impl::bitsPerDigit - lbs;
    const std::size_t lz = countLeadingZeroes();
    if (lbs > lz) {
        digit.resize(n + ds + 1);
        digit[n + ds] = digit[n - 1] >> rbs;
    } else {
        digit.resize(n + ds);
    }
    for (std::size_t i = 0; i < n - 1; ++i) {
        digit[ds + n - i - 1] =
            (digit[n - i - 1] << lbs) | (digit[n - i - 2] >> rbs);
    }
    digit[ds] = digit[0] << lbs;
    for (std::size_t i = 0; i < ds; ++i) {
        digit[i] = 0;
    }
    return *this;
}
//------------------------------------------------------------------------------
inline Unsigned& Unsigned::operator>>=(std::size_t s)
{
    if (s == 0) {
        return *this;
    }
    const std::size_t n = digit.size();
    if (n == 0) {
        return *this;
    }
    const std::size_t lz = countLeadingZeroes();
    const std::size_t nb = n * impl::bitsPerDigit - lz;
    if (s >= nb) {
        digit.resize(0);
        return *this;
    }
    const std::size_t ds = s / impl::bitsPerDigit;
    const std::size_t rbs = s % impl::bitsPerDigit;
    if (rbs == 0) {
        const std::size_t m = n - ds;
        for (std::size_t i = 0; i < m; ++i) {
            digit[i] = digit[ds + i];
        }
        digit.resize(m);
        return *this;
    }
    const std::size_t lbs = impl::bitsPerDigit - rbs;
    const std::size_t m = (nb - s - 1) / impl::bitsPerDigit + 1;
    for (std::size_t i = 0; i < n - ds - 1; ++i) {
        digit[i] = (digit[i + ds] >> rbs) | (digit[i + ds + 1] << lbs);
    }
    if (lz < lbs) {
        digit[n - ds - 1] = digit[n - 1] >> rbs;
    }
    digit.resize(m);
    return *this;
}
//------------------------------------------------------------------------------
inline Unsigned& Unsigned::operator+=(const Unsigned& v)
{
    const std::size_t n = digit.size();
    const std::size_t m = v.digit.size();
    if (m <= n) {
        bool carry = false;
        for (std::size_t i = 0; i < m; ++i) {
            digit[i] = impl::addCarry(digit[i], v.digit[i], carry);
        }
        for (std::size_t i = m; (i < n) && carry; ++i) {
            digit[i] = impl::addCarry(digit[i], 0, carry);
        }
        if (carry) {
            digit.resize(n + 1);
            digit[n] = 1;
        }
    } else {
        digit.resize(m + 1);
        bool carry = false;
        for (std::size_t i = 0; i < n; ++i) {
            digit[i] = impl::addCarry(digit[i], v.digit[i], carry);
        }
        for (std::size_t i = n; i < m; ++i) {
            digit[i] = impl::addCarry(v.digit[i], 0, carry);
        }
        if (carry) {
            digit[m] = 1;
        } else {
            digit.resize(m);
        }
    }
    return *this;
}
//------------------------------------------------------------------------------
inline Unsigned& Unsigned::operator-=(const Unsigned& v)
{
    const std::size_t n = digit.size();
    const std::size_t m = v.digit.size();
    if (m > n) {
        throw std::invalid_argument("minuend is larger than subtrahend");
    }
    bool borrow = 0;
    for (std::size_t i = 0; i < m; ++i) {
        digit[i] = impl::subBorrow(digit[i], v.digit[i], borrow);
    }
    std::size_t i;
    for (i = m; (i < n) && borrow; ++i) {
        digit[i] = impl::subBorrow(digit[i], 0, borrow);
    }
    if (borrow) {
        throw std::invalid_argument("minuend is larger than subtrahend");
    }
    removeLeadingZeroDigits();
    return *this;
}
//------------------------------------------------------------------------------
inline Unsigned& Unsigned::operator*=(const Unsigned& v)
{
    Unsigned w = *this * v;
    *this = std::move(w);
    return *this;
}
//------------------------------------------------------------------------------
inline Unsigned& Unsigned::operator/=(const Unsigned& v)
{
    Unsigned w = *this / v;
    *this = std::move(w);
    return *this;
}
//------------------------------------------------------------------------------
inline Unsigned& Unsigned::operator%=(const Unsigned& v)
{
    Unsigned w = *this % v;
    *this = std::move(w);
    return *this;
}
//------------------------------------------------------------------------------
inline Unsigned Unsigned::div(const Unsigned& v)
{
    Unsigned::QR qr = ::bn::div(*this, v);
    *this = std::move(qr.quot);
    return qr.rem;
}
//------------------------------------------------------------------------------
inline bool Unsigned::empty() const
{
    return digit.size() == 0;
}
//------------------------------------------------------------------------------
inline std::size_t Unsigned::bits() const
{
    return impl::bitsPerDigit * digit.size() - countLeadingZeroes();
}
//------------------------------------------------------------------------------
inline std::size_t Unsigned::ctz() const
{
    std::size_t ret = 0;
    std::size_t i = 0;
    while ((i < digit.size()) && (digit[i] == 0)) {
        ret += impl::bitsPerDigit;
        ++i;
    }
    if (i < digit.size()) {
        ret += impl::countTrailingZeroes(digit[i]);
    }
    return ret;
}
//------------------------------------------------------------------------------
inline Unsigned::operator std::uint64_t() const
{
    if (bits() > 64) {
        throw std::overflow_error("this does not fit in a uint64_t");
    }
    const std::size_t n = digit.size();
    std::uint64_t ret = 0;
    for (std::size_t i = 0; i < n; ++i) {
        ret |= static_cast<uint64_t>(digit[i]) << (i * impl::bitsPerDigit);
    }
    return ret;
}
//------------------------------------------------------------------------------
inline std::string Unsigned::str() const
{
    if (digit.size() == 0) {
        return std::string("0");
    }
    Unsigned temp = *this;
    constexpr double log256 = 2.4082399653118496;
    std::size_t numBytes = sizeof(impl::digit_t) * temp.digit.size();
    std::size_t reserveDigits =
        static_cast<std::size_t>(std::ceil(numBytes * log256))
        + impl::maxDecDigitsPerDigit;
    std::string s;
    s.reserve(reserveDigits);
    while (temp.digit.size() > 0) {
        impl::digit_t mod = temp.divideByDigitReturnRem(impl::maxPow10PerDigit);
        for (unsigned i = 0; i < impl::maxDecDigitsPerDigit; ++i) {
            s.push_back('0' + static_cast<char>(mod % 10));
            mod /= 10;
        }
    }
    while (s.back() == '0') {
        s.pop_back();
    }
    std::reverse(s.begin(), s.end());
    return s;
}
//------------------------------------------------------------------------------
inline std::size_t Unsigned::digits() const
{
    return digit.size();
}
//------------------------------------------------------------------------------
template<int S, typename T>
typename std::enable_if<S >= 8 * sizeof(T), T>::type Unsigned::safeRightShift(T)
{
    return 0;
}
//------------------------------------------------------------------------------
template<int S, typename T>
    typename std::enable_if
    < S<8 * sizeof(T), T>::type Unsigned::safeRightShift(T val)
{
    return val >> S;
}
//------------------------------------------------------------------------------
template<typename T>
void Unsigned::initFromIntegral(T val)
{
    const std::size_t n =
        std::max<std::size_t>(sizeof(val) / sizeof(impl::digit_t), 1);
    digit.resize(n);
    std::size_t i = 0;
    for (; val != 0; ++i, val = safeRightShift<impl::bitsPerDigit>(val)) {
        digit[i] = static_cast<impl::digit_t>(val);
    }
    digit.resize(i);
}
//------------------------------------------------------------------------------
template<std::size_t BPD, typename Generator>
typename std::enable_if<(BPD < 8 * sizeof(unsigned)), Unsigned>::type
    Unsigned::generateRandom(std::size_t numBits, Generator& gen)
{
    static_assert(
        (8 * sizeof(unsigned)) % BPD == 0,
        "bitsPerDigit must divide 8*sizeof(unsigned)");
    Unsigned w;
    const std::size_t n = (numBits - 1) / BPD + 1;
    w.digit.resize(n);
    unsigned r;
    std::size_t rbits = 0;
    std::uniform_int_distribution<unsigned> dis;
    for (std::size_t i = 0; i < n; ++i) {
        if (rbits == 0) {
            r = dis(gen);
            rbits = 8 * sizeof(unsigned);
        }
        w.digit[i] = static_cast<impl::digit_t>(r);
        r >>= BPD;
        rbits -= BPD;
    }
    w.randomGenMaskHighest(numBits % BPD);
    w.removeLeadingZeroDigits();
    return w;
}
//------------------------------------------------------------------------------
template<std::size_t BPD, typename Generator>
typename std::enable_if<(BPD == 8 * sizeof(unsigned)), Unsigned>::type
    Unsigned::generateRandom(std::size_t numBits, Generator& gen)
{
    Unsigned w;
    const std::size_t n = (numBits - 1) / BPD + 1;
    w.digit.resize(n);
    std::uniform_int_distribution<unsigned> dis;
    for (std::size_t i = 0; i < n; ++i) {
        w.digit[i] = dis(gen);
    }
    w.randomGenMaskHighest(numBits % BPD);
    w.removeLeadingZeroDigits();
    return w;
}
//------------------------------------------------------------------------------
template<std::size_t BPD, typename Generator>
typename std::enable_if<(BPD > 8 * sizeof(unsigned)), Unsigned>::type
    Unsigned::generateRandom(std::size_t numBits, Generator& gen)
{
    static_assert(
        BPD % (8 * sizeof(unsigned)) == 0,
        "8*sizeof(unsigned) must divide bitsPerDigit");
    Unsigned w;
    const std::size_t n = (numBits - 1) / BPD + 1;
    w.digit.resize(n);
    std::uniform_int_distribution<unsigned> dis;
    for (std::size_t i = 0; i < n; ++i) {
        w.digit[i] = dis(gen);
        for (std::size_t j = 1; j < BPD / (8 * sizeof(unsigned)); ++j) {
            w.digit[i] = (w.digit[i] << (8 * sizeof(unsigned))) | dis(gen);
        }
    }
    w.randomGenMaskHighest(numBits % BPD);
    w.removeLeadingZeroDigits();
    return w;
}
//------------------------------------------------------------------------------
inline void Unsigned::randomGenMaskHighest(std::size_t mb)
{
    if (mb == 0) {
        return;
    }
    impl::digit_t mask = 1;
    for (std::size_t i = 1; i < mb; ++i) {
        mask = (mask << 1) | 1;
    }
    const std::size_t n = digit.size();
    digit[n - 1] &= mask;
    removeLeadingZeroDigits();
}
//------------------------------------------------------------------------------
inline std::size_t Unsigned::countLeadingZeroes() const
{
    const std::size_t n = digit.size();
    if (n == 0) {
        return 0;
    }
    return ::bn::impl::countLeadingZeroes(digit[n - 1]);
}
//------------------------------------------------------------------------------
inline void Unsigned::addDigit(impl::digit_t d)
{
    const std::size_t n = digit.size();
    if (n == 0) {
        if (d != 0) {
            digit.resize(1);
            digit[0] = d;
        }
        return;
    }
    bool carry = false;
    digit[0] = impl::addCarry(digit[0], d, carry);
    for (std::size_t i = 1; (i < n) && carry; ++i) {
        digit[i] = impl::addCarry(digit[i], 0, carry);
    }
    if (carry) {
        digit.resize(n + 1);
        digit[n] = 1;
    }
}
//------------------------------------------------------------------------------
inline void Unsigned::subtractDigit(impl::digit_t d)
{
    const std::size_t n = digit.size();
    if ((n == 0) || ((n == 1) && (d > digit[0]))) {
        throw std::logic_error("result would be negative");
    }
    bool borrow = false;
    digit[0] = impl::subBorrow(digit[0], d, borrow);
    for (std::size_t i = 1; (i < n) && borrow; ++i) {
        digit[i] = impl::subBorrow(digit[i], 0, borrow);
    }
    removeLeadingZeroDigits();
}
//------------------------------------------------------------------------------
inline void Unsigned::multiplyByDigit(impl::digit_t d)
{
    const std::size_t n = digit.size();
    impl::digit_t carry = 0;
    for (size_t i = 0; i < n; ++i) {
        digit[i] = impl::multiplyAdd(digit[i], d, carry);
    }
    if (carry != 0) {
        digit.resize(n + 1);
        digit[n] = carry;
    }
}
//------------------------------------------------------------------------------
inline impl::digit_t Unsigned::divideByDigitReturnRem(impl::digit_t d)
{
    assert(d != 0);
    if (d == 1) {
        return 0;
    }
    const std::size_t n = digit.size();
    impl::digit_t remainder = 0;
    for (std::size_t i = n; i != 0; --i) {
        digit[i - 1] = impl::divideRemainder(digit[i - 1], d, remainder);
    }
    if (digit[n - 1] == 0) {
        digit.resize(n - 1);
    }
    return remainder;
}
//------------------------------------------------------------------------------
inline impl::digit_t Unsigned::findDivQuotient(
    impl::digit_t un,
    impl::digit_t un1,
    impl::digit_t un2,
    impl::digit_t vn1,
    impl::digit_t vn2)
{
    assert(vn1 & (static_cast<impl::digit_t>(1) << (impl::bitsPerDigit - 1)));

    if (un < vn1) {
        impl::digit_t r = un;
        impl::digit_t q = impl::divideRemainder(un1, vn1, r);

        impl::digit_t carry = 0;
        impl::digit_t p1 = impl::multiplyAdd(q, vn2, carry);
        if ((carry < r) || ((carry == r) && (p1 <= un2))) {
            return q;
        }

        bool acarry = false;
        r = impl::addCarry(r, vn1, acarry);
        if (acarry) {
            return q - 1;
        }

        bool borrow = false;
        p1 = impl::subBorrow(p1, vn2, borrow);
        carry -= borrow;
        if ((carry < r) || ((carry == r) && (p1 <= un2))) {
            return q - 1;
        }
        return q - 2;
    }

    bool borrow = false;
    un1 = impl::subBorrow(un1, vn1, borrow);
    un -= borrow;
    if (un < vn1) {
        impl::digit_t r = un;
        impl::digit_t q = impl::divideRemainder(un1, vn1, r);

        bool acarry = false;
        r = impl::addCarry(r, vn1, acarry);
        if (acarry) {
            return q;
        }

        impl::digit_t carry = 0;
        impl::digit_t p1 = impl::multiplyAdd(q, vn2, carry);
        if ((carry < r) || ((carry == r) && (p1 <= un2))) {
            return q;
        }
        return q - 1;
    }

    un1 -= vn1;
    impl::digit_t r = un - 1;
    impl::digit_t q = impl::divideRemainder(un1, vn1, r);
    return q;
}
//------------------------------------------------------------------------------
inline impl::digit_t Unsigned::findDivQuotient(
    const Unsigned& u,
    std::size_t ls,
    impl::digit_t vn1,
    impl::digit_t vn2,
    std::size_t i)
{
    assert(u.digit.size() >= 3);
    assert(i < u.digit.size());
    if (ls == 0) {
        return findDivQuotient(
            u.digit[i], u.digit[i - 1], u.digit[i - 2], vn1, vn2);
    }
    const std::size_t rs = impl::bitsPerDigit - ls;
    impl::digit_t un = (u.digit[i] << ls) | (u.digit[i - 1] >> rs);
    impl::digit_t un1 = (u.digit[i - 1] << ls) | (u.digit[i - 2] >> rs);
    impl::digit_t un2 = (u.digit[i - 2] << ls);
    if ((i - 2) > 0) {
        un2 |= u.digit[i - 3] >> rs;
    }
    return findDivQuotient(un, un1, un2, vn1, vn2);
}
//------------------------------------------------------------------------------
inline void Unsigned::removeLeadingZeroDigits()
{
    std::size_t n = digit.size();
    while ((n > 0) && (digit[n - 1] == 0)) {
        --n;
    }
    digit.resize(n);
}
//------------------------------------------------------------------------------
inline bool operator==(const Unsigned& u, const Unsigned& v)
{
    if (u.digit.size() != v.digit.size()) {
        return false;
    }
    const std::size_t n = u.digit.size();
    for (std::size_t i = 0; i < n; ++i) {
        if (u.digit[i] != v.digit[i]) {
            return false;
        }
    }
    return true;
}
//------------------------------------------------------------------------------
inline bool operator!=(const Unsigned& u, const Unsigned& v)
{
    return !(u == v);
}
//------------------------------------------------------------------------------
inline bool operator<(const Unsigned& u, const Unsigned& v)
{
    if (u.digit.size() != v.digit.size()) {
        return u.digit.size() < v.digit.size();
    }
    const std::size_t n = u.digit.size();
    for (std::size_t i = n; i != 0; --i) {
        if (u.digit[i - 1] == v.digit[i - 1]) {
            continue;
        }
        return u.digit[i - 1] < v.digit[i - 1];
    }
    return false;
}
//------------------------------------------------------------------------------
inline bool operator>=(const Unsigned& u, const Unsigned& v)
{
    return !(u < v);
}
//------------------------------------------------------------------------------
inline bool operator>(const Unsigned& u, const Unsigned& v)
{
    return v < u;
}
//------------------------------------------------------------------------------
inline bool operator<=(const Unsigned& u, const Unsigned& v)
{
    return !(v < u);
}
//------------------------------------------------------------------------------
inline Unsigned operator|(const Unsigned& pu, const Unsigned& pv)
{
    const Unsigned& u = pu.digit.size() >= pv.digit.size() ? pu : pv;
    const Unsigned& v = pu.digit.size() >= pv.digit.size() ? pv : pu;
    const std::size_t n = u.digit.size();
    const std::size_t m = v.digit.size();
    Unsigned w;
    w.digit.resize(n);
    for (std::size_t i = 0; i < m; ++i) {
        w.digit[i] = u.digit[i] | v.digit[i];
    }
    for (std::size_t i = m; i < n; ++i) {
        w.digit[i] = u.digit[i];
    }
    return w;
}
//------------------------------------------------------------------------------
inline Unsigned operator&(const Unsigned& u, const Unsigned& v)
{
    Unsigned w;
    const std::size_t n = std::min(u.digit.size(), v.digit.size());
    w.digit.resize(n);
    for (std::size_t i = 0; i < n; ++i) {
        w.digit[i] = u.digit[i] & v.digit[i];
    }
    w.removeLeadingZeroDigits();
    return w;
}
//------------------------------------------------------------------------------
inline Unsigned operator^(const Unsigned& pu, const Unsigned& pv)
{
    const Unsigned& u = pu.digit.size() >= pv.digit.size() ? pu : pv;
    const Unsigned& v = pu.digit.size() >= pv.digit.size() ? pv : pu;
    const std::size_t n = u.digit.size();
    const std::size_t m = v.digit.size();
    Unsigned w;
    w.digit.resize(n);
    for (std::size_t i = 0; i < m; ++i) {
        w.digit[i] = u.digit[i] ^ v.digit[i];
    }
    for (std::size_t i = m; i < n; ++i) {
        w.digit[i] = u.digit[i];
    }
    w.removeLeadingZeroDigits();
    return w;
}
//------------------------------------------------------------------------------
inline Unsigned operator<<(const Unsigned& u, std::size_t s)
{
    if (s == 0) {
        return u;
    }
    const std::size_t n = u.digit.size();
    if (n == 0) {
        return u;
    }
    const std::size_t ds = s / impl::bitsPerDigit;
    const std::size_t lbs = s % impl::bitsPerDigit;
    Unsigned w;
    if (lbs == 0) {
        w.digit.resize(n + ds);
        for (size_t i = 0; i < n; ++i) {
            w.digit[i + ds] = u.digit[i];
        }
        for (size_t i = 0; i < ds; ++i) {
            w.digit[i] = 0;
        }
        return w;
    }
    const std::size_t rbs = impl::bitsPerDigit - lbs;
    const std::size_t lz = u.countLeadingZeroes();
    if (lbs > lz) {
        w.digit.resize(n + ds + 1);
        w.digit[n + ds] = u.digit[n - 1] >> rbs;
    } else {
        w.digit.resize(n + ds);
    }
    for (std::size_t i = 0; i < n - 1; ++i) {
        w.digit[ds + n - i - 1] =
            (u.digit[n - i - 1] << lbs) | (u.digit[n - i - 2] >> rbs);
    }
    w.digit[ds] = u.digit[0] << lbs;
    for (std::size_t i = 0; i < ds; ++i) {
        w.digit[i] = 0;
    }
    return w;
}
//------------------------------------------------------------------------------
inline Unsigned operator>>(const Unsigned& u, std::size_t s)
{
    if (s == 0) {
        return u;
    }
    const std::size_t n = u.digit.size();
    if (n == 0) {
        return u;
    }
    Unsigned w;
    const std::size_t lz = u.countLeadingZeroes();
    const std::size_t nb = n * impl::bitsPerDigit - lz;
    if (s >= nb) {
        return w;
    }
    const std::size_t ds = s / impl::bitsPerDigit;
    const std::size_t rbs = s % impl::bitsPerDigit;
    if (rbs == 0) {
        const std::size_t m = n - ds;
        w.digit.resize(m);
        for (std::size_t i = 0; i < m; ++i) {
            w.digit[i] = u.digit[ds + i];
        }
        return w;
    }
    const std::size_t lbs = impl::bitsPerDigit - rbs;
    const std::size_t m = (nb - s - 1) / impl::bitsPerDigit + 1;
    w.digit.resize(m);
    for (std::size_t i = 0; i < n - ds - 1; ++i) {
        w.digit[i] = (u.digit[i + ds] >> rbs) | (u.digit[i + ds + 1] << lbs);
    }
    if (lz < lbs) {
        w.digit[n - ds - 1] = u.digit[n - 1] >> rbs;
    }
    return w;
}
//------------------------------------------------------------------------------
inline Unsigned operator+(const Unsigned& pu, const Unsigned& pv)
{
    const Unsigned& u = pu.digit.size() >= pv.digit.size() ? pu : pv;
    const Unsigned& v = pu.digit.size() >= pv.digit.size() ? pv : pu;
    const std::size_t n = u.digit.size();
    const std::size_t m = v.digit.size();
    Unsigned w;
    w.digit.resize(n + 1);
    bool carry = false;
    for (std::size_t i = 0; i < m; ++i) {
        w.digit[i] = impl::addCarry(u.digit[i], v.digit[i], carry);
    }
    std::size_t i;
    for (i = m; (i < n) && carry; ++i) {
        w.digit[i] = impl::addCarry(u.digit[i], 0, carry);
    }
    for (; i < n; ++i) {
        w.digit[i] = u.digit[i];
    }
    if (carry) {
        w.digit[n] = 1;
    } else {
        w.digit.resize(n);
    }
    return w;
}
//------------------------------------------------------------------------------
inline Unsigned operator-(const Unsigned& u, const Unsigned& v)
{
    const std::size_t n = u.digit.size();
    const std::size_t m = v.digit.size();
    if (m > n) {
        throw std::invalid_argument("minuend is larger than subtrahend");
    }
    Unsigned w;
    w.digit.resize(n);
    bool borrow = false;
    for (std::size_t i = 0; i < m; ++i) {
        w.digit[i] = impl::subBorrow(u.digit[i], v.digit[i], borrow);
    }
    std::size_t i;
    for (i = m; (i < n) && borrow; ++i) {
        w.digit[i] = impl::subBorrow(u.digit[i], 0, borrow);
    }
    for (; i < n; ++i) {
        w.digit[i] = u.digit[i];
    }
    if (borrow != 0) {
        throw std::invalid_argument("minuend is larger than subtrahend");
    }
    w.removeLeadingZeroDigits();
    return w;
}
//------------------------------------------------------------------------------
inline Unsigned operator*(const Unsigned& u, const Unsigned& v)
{
    const std::size_t n = u.digit.size();
    const std::size_t m = v.digit.size();
    const std::size_t nm = n + m;
    Unsigned w;
    w.digit.resize(nm);
    for (std::size_t i = 0; i < nm; ++i) {
        w.digit[i] = 0;
    }
    for (std::size_t i = 0; i < m; ++i) {
        impl::digit_t carry = 0;
        for (std::size_t j = 0; j < n; ++j) {
            w.digit[i + j] = impl::multiplyAdd2(
                u.digit[j], v.digit[i], w.digit[i + j], carry);
        }
        w.digit[i + n] += carry;
    }
    w.removeLeadingZeroDigits();
    return w;
}
//------------------------------------------------------------------------------
inline Unsigned operator/(const Unsigned& u, const Unsigned& v)
{
    return div(u, v).quot;
}
//------------------------------------------------------------------------------
inline Unsigned operator%(const Unsigned& u, const Unsigned& v)
{
    return div(u, v).rem;
}
//------------------------------------------------------------------------------
inline Unsigned::QR div(const Unsigned& u, const Unsigned& v)
{
    const std::size_t n = v.digit.size();
    if (n == 0) {
        throw std::invalid_argument("division by 0");
    }
    if (n > u.digit.size()) {
        return Unsigned::QR{Unsigned(), u};
    }
    if (n == 1) {
        Unsigned quot = u;
        Unsigned rem;
        rem.digit.resize(1);
        rem.digit[0] = quot.divideByDigitReturnRem(v.digit[0]);
        rem.removeLeadingZeroDigits();
        return Unsigned::QR{quot, rem};
    }
    const std::size_t m = u.digit.size() - n;
    Unsigned q;
    q.digit.resize(m + 1);

    // D1
    const std::size_t ls = v.countLeadingZeroes();
    impl::digit_t vn1;
    impl::digit_t vn2;
    if (ls == 0) {
        vn1 = v.digit[n - 1];
        vn2 = v.digit[n - 2];
    } else {
        const std::size_t rs = impl::bitsPerDigit - ls;
        vn1 = (v.digit[n - 1] << ls) | (v.digit[n - 2] >> rs);
        vn2 = (v.digit[n - 2] << ls);
        if (n > 2) {
            vn2 |= v.digit[n - 3] >> rs;
        }
    }
    Unsigned nu;
    nu.digit.resize(u.digit.size() + 1);
    nu.digit = u.digit;
    nu.digit.resize(u.digit.size() + 1);
    nu.digit[u.digit.size()] = 0;

    // D2
    std::size_t j = m;
    while (true) {
        // D3
        impl::digit_t qd = Unsigned::findDivQuotient(nu, ls, vn1, vn2, j + n);
        // D4
        impl::digit_t carry = 0;
        for (std::size_t i = 0; i < n; ++i) {
            impl::digit_t md = impl::multiplyAdd(qd, v.digit[i], carry);
            bool borrow = false;
            nu.digit[i + j] = impl::subBorrow(nu.digit[j + i], md, borrow);
            carry += borrow;
        }
        bool borrow = false;
        nu.digit[j + n] = impl::subBorrow(nu.digit[j + n], carry, borrow);
        // D5
        q.digit[j] = qd;
        if (borrow) {
            // D6
            --q.digit[j];
            bool acarry = false;
            for (std::size_t i = 0; i < n; ++i) {
                nu.digit[j + i] =
                    impl::addCarry(nu.digit[j + i], v.digit[i], acarry);
            }
            nu.digit[j + n] += acarry;
        }
        // D7
        if (j == 0) {
            break;
        }
        --j;
    }
    // D8
    nu.removeLeadingZeroDigits();
    q.removeLeadingZeroDigits();
    return Unsigned::QR{q, nu};
}
//------------------------------------------------------------------------------
inline Unsigned pow(const Unsigned& u, std::size_t exp)
{
    Unsigned r = 1;
    Unsigned p = u;
    while (true) {
        if (exp & 1) {
            r *= p;
        }
        exp >>= 1;
        if (exp == 0) {
            return r;
        }
        p *= p;
    }
}
//------------------------------------------------------------------------------
inline Unsigned powmod(const Unsigned& u, Unsigned exp, const Unsigned& mod)
{
    Unsigned r = 1;
    if (exp.digits() == 0) {
        return r;
    }
    Unsigned p = u % mod;
    while (true) {
        if (exp.digit[0] & 1) {
            r = (r * p) % mod;
        }
        exp >>= 1;
        if (exp.digits() == 0) {
            return r;
        }
        p = (p * p) % mod;
    }
}
//------------------------------------------------------------------------------
inline Unsigned sqrt(const Unsigned& u)
{
    if (u.empty()) {
        return u;
    }
    std::size_t shift = (u.bits() - 1) & ~static_cast<std::size_t>(1);
    Unsigned bit = 1;
    bit <<= shift;

    Unsigned r;
    Unsigned n = u;
    while (bit != 0) {
        if (n >= r + bit) {
            n -= r;
            n -= bit;
            r >>= 1;
            r += bit;
        } else {
            r >>= 1;
        }
        bit >>= 2;
    }
    return r;
}
//------------------------------------------------------------------------------
inline Unsigned egcd(const Unsigned& u, const Unsigned& v)
{
    const bool vlte = (v <= u);
    Unsigned a = vlte ? u : v;
    Unsigned b = vlte ? v : u;
    Unsigned zero;
    while (!b.empty()) {
        Unsigned c = a % b;
        a = std::move(b);
        b = std::move(c);
    }
    return a;
}
//------------------------------------------------------------------------------
inline Unsigned bgcd(const Unsigned& u, const Unsigned& v)
{
    if (u.empty()) {
        return v;
    }
    if (v.empty()) {
        return u;
    }

    Unsigned wu = u;
    Unsigned wv = v;
    std::size_t utz = wu.ctz();
    std::size_t vtz = wv.ctz();
    std::size_t shift = std::min(utz, vtz);
    wu >>= utz;
    wv >>= shift;
    do {
        vtz = wv.ctz();
        wv >>= vtz;
        if (wu > wv) {
            std::swap(wu, wv);
        }
        wv -= wu;
    } while (wv != 0);
    return wu << shift;
}
//------------------------------------------------------------------------------
inline Unsigned gcd(const Unsigned& u, const Unsigned& v)
{
    return bgcd(u, v);
}
//------------------------------------------------------------------------------
inline std::ostream& operator<<(std::ostream& os, const Unsigned& u)
{
    return os << u.str();
}
//------------------------------------------------------------------------------
inline std::ostream& operator<<(std::ostream& out, const Unsigned::QR& qr)
{
    out << "(q=" << qr.quot << " r=" << qr.rem << ")";
    return out;
}
//------------------------------------------------------------------------------
inline bool operator==(const Unsigned::QR& qr1, const Unsigned::QR& qr2)
{
    return (qr1.quot == qr2.quot) && (qr1.rem == qr2.rem);
}
//------------------------------------------------------------------------------
inline bool operator!=(const Unsigned::QR& qr1, const Unsigned::QR& qr2)
{
    return !(qr1 == qr2);
}
//------------------------------------------------------------------------------
inline Signed::Signed() noexcept : sign(0)
{
}
//------------------------------------------------------------------------------
inline Signed::Signed(const Signed& other) : val(other.val), sign(other.sign)
{
}
//------------------------------------------------------------------------------
inline Signed::Signed(Signed&& other) noexcept
    : val(std::move(other.val)), sign(other.sign)
{
}
//------------------------------------------------------------------------------
inline Signed::Signed(const Unsigned& other)
    : val(other), sign(val.empty() ? 0 : 1)
{
}
//------------------------------------------------------------------------------
inline Signed::Signed(Unsigned&& other) noexcept
    : val(std::move(other)), sign(val.empty() ? 0 : 1)
{
}
//------------------------------------------------------------------------------
inline Signed::Signed(std::int32_t i)
    : val(
        i < 0 ? ~static_cast<std::uint32_t>(i) + 1
              : static_cast<std::uint32_t>(i))
    , sign(
          i > 0   ? 1
          : i < 0 ? -1
                  : 0)
{
}
//------------------------------------------------------------------------------
inline Signed::Signed(std::uint32_t i) : val(i), sign(i > 0 ? 1 : 0)
{
}
//------------------------------------------------------------------------------
inline Signed::Signed(std::int64_t i)
    : val(
        i < 0 ? ~static_cast<std::uint64_t>(i) + 1
              : static_cast<std::uint64_t>(i))
    , sign(
          i > 0   ? 1
          : i < 0 ? -1
                  : 0)
{
}
//------------------------------------------------------------------------------
inline Signed::Signed(std::uint64_t i) : val(i), sign(i > 0 ? 1 : 0)
{
}
//------------------------------------------------------------------------------
inline Signed::Signed(const char* dec)
    : val(dec[0] == '-' ? dec + 1 : dec)
    , sign(
          val.empty()     ? 0
          : dec[0] == '-' ? -1
                          : 1)
{
}
//------------------------------------------------------------------------------
inline Signed& Signed::operator=(const Signed& other)
{
    val = other.val;
    sign = other.sign;
    return *this;
}
//------------------------------------------------------------------------------
inline Signed& Signed::operator=(Signed&& other) noexcept
{
    val = std::move(other.val);
    sign = other.sign;
    return *this;
}
//------------------------------------------------------------------------------
inline std::string Signed::str() const
{
    std::string vs = val.str();
    if (sign == -1) {
        vs = "-" + vs;
    }
    return vs;
}
//------------------------------------------------------------------------------
inline int Signed::sgn() const
{
    return sign;
}
//------------------------------------------------------------------------------
inline const Unsigned& Signed::abs() const
{
    return val;
}
//------------------------------------------------------------------------------
inline Signed& Signed::operator++()
{
    switch (sign) {
    case -1:
        --val;
        if (val.empty()) {
            sign = 0;
        }
        break;
    case 0:
        ++val;
        sign = 1;
        break;
    case 1: ++val; break;
    }
    return *this;
}
//------------------------------------------------------------------------------
inline Signed Signed::operator++(int)
{
    Signed ret = *this;
    ++(*this);
    return ret;
}
//------------------------------------------------------------------------------
inline Signed& Signed::operator--()
{
    switch (sign) {
    case -1: ++val; break;
    case 0:
        ++val;
        sign = -1;
        break;
    case 1:
        --val;
        if (val.empty()) {
            sign = 0;
        }
        break;
    }
    return *this;
}
//------------------------------------------------------------------------------
inline Signed Signed::operator--(int)
{
    Signed ret = *this;
    --(*this);
    return ret;
}
//------------------------------------------------------------------------------
inline Signed& Signed::operator+=(const Signed& v)
{
    if (sign == v.sign) {
        val += v.val;
    } else if (val > v.val) {
        val -= v.val;
    } else {
        val = v.val - val;
        sign = val.empty() ? 0 : v.sign;
    }
    return *this;
}
//------------------------------------------------------------------------------
inline Signed& Signed::operator-=(const Signed& v)
{
    if (-sign == v.sign) {
        val += v.val;
    } else if (val > v.val) {
        val -= v.val;
    } else {
        val = v.val - val;
        sign = val.empty() ? 0 : -v.sign;
    }
    return *this;
}
//------------------------------------------------------------------------------
inline Signed& Signed::operator*=(const Signed& v)
{
    val *= v.val;
    sign *= v.sign;
    return *this;
}
//------------------------------------------------------------------------------
inline Signed& Signed::operator/=(const Signed& v)
{
    val /= v.val;
    sign *= v.sign;
    return *this;
}
//------------------------------------------------------------------------------
inline Signed& Signed::operator%=(const Signed& v)
{
    val %= v.val;
    return *this;
}
//------------------------------------------------------------------------------
inline Signed Signed::div(const Signed& v)
{
    Signed rem(val.div(v.val));
    rem.sign = sign;
    sign *= v.sign;
    return rem;
}
//------------------------------------------------------------------------------
inline bool operator==(const Signed& u, const Signed& v)
{
    return (u.sign == v.sign) && (u.val == v.val);
}
//------------------------------------------------------------------------------
inline bool operator!=(const Signed& u, const Signed& v)
{
    return !(u == v);
}
//------------------------------------------------------------------------------
inline bool operator<(const Signed& u, const Signed& v)
{
    if (u.sign != v.sign) {
        return u.sign < v.sign;
    }
    return u.val < v.val;
}
//------------------------------------------------------------------------------
inline bool operator>=(const Signed& u, const Signed& v)
{
    return !(u < v);
}
//------------------------------------------------------------------------------
inline bool operator>(const Signed& u, const Signed& v)
{
    return (v < u);
}
//------------------------------------------------------------------------------
inline bool operator<=(const Signed& u, const Signed& v)
{
    return !(u > v);
}
//------------------------------------------------------------------------------
inline Signed operator-(const Signed& u)
{
    Signed w;
    w.sign = -u.sign;
    w.val = u.val;
    return w;
}
//------------------------------------------------------------------------------
inline Signed operator+(const Signed& u, const Signed& v)
{
    Signed w;
    if (u.sign == v.sign) {
        w.val = u.val + v.val;
        w.sign = u.sign;
    } else if (u.val > v.val) {
        w.val = u.val - v.val;
        w.sign = u.sign;
    } else {
        w.val = v.val - u.val;
        w.sign = w.val.empty() ? 0 : v.sign;
    }
    return w;
}
//------------------------------------------------------------------------------
inline Signed operator-(const Signed& u, const Signed& v)
{
    Signed w;
    if (-u.sign == v.sign) {
        w.val = u.val + v.val;
        w.sign = u.sign;
    } else if (u.val > v.val) {
        w.val = u.val - v.val;
        w.sign = u.sign;
    } else {
        w.val = v.val - u.val;
        w.sign = w.val.empty() ? 0 : -v.sign;
    }
    return w;
}
//------------------------------------------------------------------------------
inline Signed operator*(const Signed& u, const Signed& v)
{
    Signed w;
    w.val = u.val * v.val;
    w.sign = u.sign * v.sign;
    return w;
}
//------------------------------------------------------------------------------
inline Signed operator/(const Signed& u, const Signed& v)
{
    return div(u, v).quot;
}
//------------------------------------------------------------------------------
inline Signed operator%(const Signed& u, const Signed& v)
{
    return div(u, v).rem;
}
//------------------------------------------------------------------------------
inline Signed::QR div(const Signed& u, const Signed& v)
{
    Unsigned::QR uqr = ::bn::div(u.val, v.val);
    Signed::QR qr{std::move(uqr.quot), std::move(uqr.rem)};
    qr.quot.sign = u.sign * v.sign;
    qr.rem.sign = u.sign;
    return qr;
}
//------------------------------------------------------------------------------
inline std::ostream& operator<<(std::ostream& os, const Signed& s)
{
    if (s.sign == -1) {
        os << '-';
    }
    os << s.val;
    return os;
}
//------------------------------------------------------------------------------
inline Rational::Rational() noexcept : den(1)
{
}
//------------------------------------------------------------------------------
inline Rational::Rational(const Signed& num, const Unsigned& den)
    : num(num), den(den)
{
    if (den.empty()) {
        throw std::invalid_argument("den is 0");
    }
    reduce();
}
//------------------------------------------------------------------------------
inline Rational::Rational(Signed&& num, Unsigned&& den)
    : num(std::move(num)), den(std::move(den))
{
    if (this->den.empty()) {
        throw std::invalid_argument("den is 0");
    }
    reduce();
}
//------------------------------------------------------------------------------
inline Rational::Rational(double d) : den(1)
{
    if (!std::isfinite(d)) {
        throw std::invalid_argument("d is not finite");
    }
    std::uint64_t u;
    memcpy(&u, &d, 8);
    std::uint64_t frac = u & 0xFFFFFFFFFFFFFull;
    std::uint64_t bexp = (u >> 52) & 0x7FFull;
    std::uint64_t sign = u >> 63;
    if (bexp == 0) {
        if (frac != 0) {
            num.val = Unsigned(frac);
            num.sign = (sign == 1) ? -1 : 1;
            den <<= (1022 + 52);
        }
    } else {
        frac |= 0x10000000000000ull;
        num.val = Unsigned(frac);
        num.sign = (sign == 1) ? -1 : 1;
        if (bexp < (1023 + 52)) {
            den <<= (1023 + 52 - bexp);
        } else {
            num.val <<= (bexp - 1023 - 52);
        }
    }
    reduce();
}
//------------------------------------------------------------------------------
inline Rational::Rational(const Unsigned& v) : num(v), den(1)
{
}
//------------------------------------------------------------------------------
inline Rational::Rational(Unsigned&& v) : num(std::move(v)), den(1)
{
}
//------------------------------------------------------------------------------
inline Rational::Rational(const Signed& v) : num(v), den(1)
{
}
//------------------------------------------------------------------------------
inline Rational::Rational(Signed&& v) : num(std::move(v)), den(1)
{
}
//------------------------------------------------------------------------------
inline const Signed& Rational::numerator() const
{
    return num;
}
//------------------------------------------------------------------------------
inline const Unsigned& Rational::denominator() const
{
    return den;
}
//------------------------------------------------------------------------------
inline Rational Rational::reciprocal() const
{
    if (num.abs().empty()) {
        throw std::logic_error("numerator is 0");
    }
    Rational w;
    w.num = den;
    w.den = num.abs();
    w.num.sign = num.sign;
    return w;
}
//------------------------------------------------------------------------------
inline Rational& Rational::operator+=(const Rational& v)
{
    num *= v.den;
    num += v.num * den;
    den *= v.den;
    reduce();
    return *this;
}
//------------------------------------------------------------------------------
inline Rational& Rational::operator-=(const Rational& v)
{
    num *= v.den;
    num -= v.num * den;
    den *= v.den;
    reduce();
    return *this;
}
//------------------------------------------------------------------------------
inline Rational& Rational::operator*=(const Rational& v)
{
    num *= v.num;
    den *= v.den;
    reduce();
    return *this;
}
//------------------------------------------------------------------------------
inline Rational& Rational::operator/=(const Rational& v)
{
    if (v.num.abs().empty()) {
        throw std::invalid_argument("division by 0");
    }
    num *= v.den;
    den *= v.num.abs();
    num.sign *= v.num.sign;
    reduce();
    return *this;
}
//------------------------------------------------------------------------------
inline Rational::operator double() const
{
    if (num.abs().empty()) {
        return 0.0;
    }
    std::ptrdiff_t d = static_cast<std::ptrdiff_t>(num.abs().bits())
                     - static_cast<std::ptrdiff_t>(den.bits());
    // We use the fact that 2^(d-1) < *this < 2^(d+1)
    if (d <= -1075) {
        // Underflow
        return std::copysign(0.0, num.sign);
    }
    if (d >= 1025) {
        // Overflow
        return std::copysign(std::numeric_limits<double>::infinity(), num.sign);
    }
    // Shift the numerator so that the quotient of the shifted numerator and
    // the denominator have at least 54 bits. According to the inequation above,
    // the quotient will have either 54 or 55 bits.
    std::ptrdiff_t nls = 54 - d;
    Unsigned snum = (nls >= 0) ? num.abs() << static_cast<std::size_t>(nls)
                               : num.abs() >> static_cast<std::size_t>(-nls);
    std::uint64_t q = static_cast<std::uint64_t>(snum / den);
    // If the quotient has 55 bits, discard one bit.
    if (q & (1ull << 54)) {
        q >>= 1;
        --nls;
    }
    // Check if we have to round up
    if (q & 3) {
        ++q;
        // If the quotient has 55 bits, discard one bit.
        if (q & (1ull << 54)) {
            q >>= 1;
            --nls;
        }
    }
    // Discard the bit used to check if rounding up is necessary
    q >>= 1;
    --nls;
    // Create the components
    std::uint64_t sign = (num.sign == -1) ? 1 : 0;
    std::uint64_t bexp;
    std::uint64_t frac;
    if (nls > 1074) {
        bexp = 0;
        std::size_t sr = nls - 1074;
        frac = (sr > 63) ? 0 : q >> sr;
    } else if (-nls > 971) {
        // Overflow
        bexp = 2047;
        frac = 0;
    } else {
        bexp = 1075 - nls;
        // Create the fraction by masking the leading bit of q
        frac = q & 0xFFFFFFFFFFFFFull;
    }
    std::uint64_t uret = (sign << 63) | (bexp << 52) | frac;
    double ret;
    memcpy(&ret, &uret, 8);
    return ret;
}
//------------------------------------------------------------------------------
inline std::string Rational::str() const
{
    std::ostringstream out;
    out << *this;
    return out.str();
}
//------------------------------------------------------------------------------
inline void Rational::reduce()
{
    Unsigned d = gcd(num.abs(), den);
    num /= d;
    den /= d;
}
//------------------------------------------------------------------------------
inline bool operator==(const Rational& u, const Rational& v)
{
    return (u.num == v.num) && (u.den == v.den);
}
//------------------------------------------------------------------------------
inline bool operator!=(const Rational& u, const Rational& v)
{
    return !(u == v);
}
//------------------------------------------------------------------------------
inline bool operator<(const Rational& u, const Rational& v)
{
    if (u.num.sgn() != v.num.sgn()) {
        return u.num.sgn() < v.num.sgn();
    }
    if (u.num.sgn() == 0) {
        return false;
    }
    const Rational& a = (u.num.sgn() == 1) ? u : v;
    const Rational& b = (u.num.sgn() == 1) ? v : u;
    const std::ptrdiff_t m = static_cast<std::ptrdiff_t>(a.num.abs().digits())
                           - static_cast<std::ptrdiff_t>(a.den.digits());
    const std::ptrdiff_t n = static_cast<std::ptrdiff_t>(b.num.abs().digits())
                           - static_cast<std::ptrdiff_t>(b.den.digits());
    if ((m + 1) <= (n - 1)) {
        return true;
    }
    if ((n + 1) <= (m - 1)) {
        return false;
    }
    const Unsigned ane = a.num.abs() * b.den;
    const Unsigned bne = b.num.abs() * a.den;
    return ane < bne;
}
//------------------------------------------------------------------------------
inline bool operator>=(const Rational& u, const Rational& v)
{
    return !(u < v);
}
//------------------------------------------------------------------------------
inline bool operator>(const Rational& u, const Rational& v)
{
    return v < u;
}
//------------------------------------------------------------------------------
inline bool operator<=(const Rational& u, const Rational& v)
{
    return !(u > v);
}
//------------------------------------------------------------------------------
inline Rational operator-(const Rational& u)
{
    return Rational(-u.num, u.den);
}
//------------------------------------------------------------------------------
inline Rational operator+(const Rational& u, const Rational& v)
{
    Rational w(u.num * v.den + v.num * u.den, u.den * v.den);
    return w;
}
//------------------------------------------------------------------------------
inline Rational operator-(const Rational& u, const Rational& v)
{
    Rational w(u.num * v.den - v.num * u.den, u.den * v.den);
    return w;
}
//------------------------------------------------------------------------------
inline Rational operator*(const Rational& u, const Rational& v)
{
    Rational w(u.num * v.num, u.den * v.den);
    return w;
}
//------------------------------------------------------------------------------
inline Rational operator/(const Rational& u, const Rational& v)
{
    if (v.num.abs().empty()) {
        throw std::invalid_argument("division by 0");
    }
    Rational w(u.num.abs() * v.den, u.den * v.num.abs());
    w.num.sign = u.num.sgn() * v.num.sgn();
    return w;
}
//------------------------------------------------------------------------------
inline std::ostream& operator<<(std::ostream& out, const Rational& u)
{
    out << u.num << "/" << u.den;
    return out;
}
//------------------------------------------------------------------------------
namespace impl {
//------------------------------------------------------------------------------
template<typename T>
typename std::enable_if<std::is_unsigned<T>::value, std::size_t>::type
    countLeadingZeroes(T val)
{
    if (val == 0) {
        return 8 * sizeof(T);
    }
    size_t ret = 0;
    T mask = ~static_cast<T>(0);
    for (T i = 4 * sizeof(T); i != 0; i >>= 1) {
        mask <<= i;
        if ((val & mask) == 0) {
            ret += i;
            val <<= i;
        }
    }
    return ret;
}
//------------------------------------------------------------------------------
template<typename T>
typename std::enable_if<std::is_unsigned<T>::value, std::size_t>::type
    countTrailingZeroes(T val)
{
    if (val == 0) {
        return 8 * sizeof(T);
    }
    size_t ret = 0;
    T mask = ~static_cast<T>(0);
    for (T i = 4 * sizeof(T); i != 0; i >>= 1) {
        mask >>= i;
        if ((val & mask) == 0) {
            ret += i;
            val >>= i;
        }
    }
    return ret;
}
//------------------------------------------------------------------------------
#if defined DIGIT_T && defined DDIGIT_T
using ddigit_t = DDIGIT_T;
#else
using ddigit_t = std::uint64_t;
#endif
//------------------------------------------------------------------------------
static_assert(
    2 * sizeof(digit_t) == sizeof(ddigit_t),
    "Size of ddigit_t must be twice the sizeof digit_t");
static_assert(std::is_unsigned<ddigit_t>::value, "ddigit_t must be unsigned");
static_assert(!std::is_const<ddigit_t>::value, "ddigit_t must not be const");
//------------------------------------------------------------------------------
inline digit_t addCarry(digit_t a, digit_t b, bool& carry)
{
    ddigit_t dd = static_cast<ddigit_t>(a) + static_cast<ddigit_t>(b)
                + static_cast<ddigit_t>(carry);
    digit_t ret = static_cast<digit_t>(dd);
    carry = static_cast<bool>(dd >> bitsPerDigit);
    return ret;
}
//------------------------------------------------------------------------------
inline digit_t subBorrow(digit_t a, digit_t b, bool& borrow)
{
    ddigit_t dd = static_cast<ddigit_t>(a) - static_cast<ddigit_t>(b)
                - static_cast<ddigit_t>(borrow);
    digit_t ret = static_cast<digit_t>(dd);
    borrow = static_cast<bool>((dd >> bitsPerDigit) != 0);
    return ret;
}
//------------------------------------------------------------------------------
inline digit_t multiplyAdd(digit_t a, digit_t b, digit_t& carry)
{
    ddigit_t dd = static_cast<ddigit_t>(a) * static_cast<ddigit_t>(b)
                + static_cast<ddigit_t>(carry);
    digit_t ret = static_cast<digit_t>(dd);
    carry = static_cast<digit_t>(dd >> bitsPerDigit);
    return ret;
}
//------------------------------------------------------------------------------
inline digit_t multiplyAdd2(digit_t a, digit_t b, digit_t c, digit_t& carry)
{
    ddigit_t dd = static_cast<ddigit_t>(a) * static_cast<ddigit_t>(b)
                + static_cast<ddigit_t>(c) + static_cast<ddigit_t>(carry);
    digit_t ret = static_cast<digit_t>(dd);
    carry = static_cast<digit_t>(dd >> bitsPerDigit);
    return ret;
}
//------------------------------------------------------------------------------
inline digit_t divideRemainder(digit_t a, digit_t b, digit_t& remainder)
{
    ddigit_t dd = (static_cast<ddigit_t>(remainder) << bitsPerDigit)
                | static_cast<ddigit_t>(a);
    digit_t ret = static_cast<digit_t>(dd / b);
    remainder = static_cast<digit_t>(dd % b);
    return ret;
}
//------------------------------------------------------------------------------
inline std::size_t countLeadingZeroes(digit_t val)
{
    return countLeadingZeroes<digit_t>(val);
}
//------------------------------------------------------------------------------
inline std::size_t countTrailingZeroes(digit_t val)
{
    return countTrailingZeroes<digit_t>(val);
}
//------------------------------------------------------------------------------
}  // namespace impl
//------------------------------------------------------------------------------
}  // namespace bn
//------------------------------------------------------------------------------
#endif
