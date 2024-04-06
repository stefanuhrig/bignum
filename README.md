# A C++-11 single-header generic arbitrary-precision number library

[![codecov](https://codecov.io/github/stefanuhrig/bignum/graph/badge.svg?token=LKHJZ14LTD)](https://codecov.io/github/stefanuhrig/bignum)

## About

This is a C++-11 compliant generic single-header arbitrary-precision 
number library offering natural ($\mathbb{N}$), integer ($\mathbb{Z}$) and 
rational ($\mathbb{R}$) number types and corresponding mathematical operations.
 
Design goals of this library are
 - intuitive and easy usage
 - providing all functionality in a single header so that it can easily be
   consumed by C++ projects
 - maximum compatibility with C++-11 compliant compilers and all platforms

This library does not contain optimized algorithms for any particular platform 
and uses mainly naive "schoolbook" methods. With regards to performance, it 
cannot compete with mature arbitrary-precision libraries like GMP.

The given time complexity is the worst case complexity. $n$ is the number of 
digits in a number. If multiple numbers are involved in an operation, $n$ is 
chosen to be the number of digits in the longest number.

If you'd like to improve the performance on a particular architecture, you can 
customize the digit type and the seven primitive operations on which all 
algorithms are based.

## Usage

To use the library, you can just copy [bignum.h](include/bignum.h) to your
project.

## Example

Particular useful is the capability to convert double-precision floating-point 
numbers to rational numbers and to round rational numbers to the nearest 
double-precision floating-point number. This might be used to find the correctly
rounded double-precision floating-point value of transcedental functions. The 
following code (optimized for readability) demonstrates how the sine of a double
can be computed using the Taylor series:

$$\sin(x) = x - \frac{1}{3!}x^3 + \frac{1}{5!}x^5 - \frac{1}{7!}x^7 ...$$

```c++
#include "bignum.h"

using namespace bn;
using namespace std;

double crsin(double arg)
{
    Rational x = arg;
    Rational prev_acc;
    Rational curr_acc = x;
    Rational term = x;
    Unsigned factorial = 1;
    Signed sign = 1;
    do {
        prev_acc = std::move(curr_acc);
        sign *= -1;
        term *= x * x;
        term /= (factorial + 1) * (factorial + 2);
        factorial += 2;
        curr_acc = prev_acc + sign * term;
    } while (static_cast<double>(curr_acc) != static_cast<double>(prev_acc));
    return static_cast<double>(curr_acc);
}

```
