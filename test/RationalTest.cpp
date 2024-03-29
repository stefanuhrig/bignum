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
#include "bignum.h"

#include <gmock/gmock.h>
//------------------------------------------------------------------------------
using namespace bn;
using namespace std;
//------------------------------------------------------------------------------
using digit_t = impl::digit_t;
constexpr size_t bitsPerDigit = impl::bitsPerDigit;
//------------------------------------------------------------------------------
TEST(RationalTest, constructDefault)
{
    Rational r;
    EXPECT_EQ(Signed(), r.numerator());
    EXPECT_EQ(Unsigned(1), r.denominator());
}
//------------------------------------------------------------------------------
TEST(RationalTest, constructCopy)
{
    Signed num = 18;
    Unsigned den = 48;
    Unsigned zero;
    Rational r(num, den);
    EXPECT_EQ(Signed(3), r.numerator());
    EXPECT_EQ(Unsigned(8), r.denominator());
    EXPECT_THROW(Rational temp(num, zero), invalid_argument);
}
//------------------------------------------------------------------------------
TEST(RationalTest, constructMove)
{
    Signed num = 18;
    Unsigned den = 48;
    Rational r(std::move(num), std::move(den));
    EXPECT_EQ(Signed(3), r.numerator());
    EXPECT_EQ(Unsigned(8), r.denominator());

    Signed one = 1;
    Unsigned zero;
    EXPECT_THROW(
        Rational temp(std::move(one), std::move(zero)), invalid_argument);
}
//------------------------------------------------------------------------------
TEST(RationalTest, constructDouble)
{
    EXPECT_THROW(
        Rational temp(numeric_limits<double>::infinity()), invalid_argument);

    Rational r01;
    Rational rd01 = 0.0;
    EXPECT_EQ(r01, rd01);

    Rational r12(1, 2);
    Rational rd12 = 0.5;
    EXPECT_EQ(r12, rd12);

    Rational r264(Unsigned(1) << 64, 1);
    Rational rd264 = pow(2.0, 64.0);
    EXPECT_EQ(r264, rd264);

    Rational rm12(-1, 2);
    Rational rdm12 = -0.5;
    EXPECT_EQ(rm12, rdm12);

    Rational rmin(1, Unsigned(1) << (1022 + 52));
    Rational rdmin(numeric_limits<double>::denorm_min());
    EXPECT_EQ(rmin, rdmin);

    Rational rmmin(-1, Unsigned(1) << (1022 + 52));
    Rational rdmmin(-numeric_limits<double>::denorm_min());
    EXPECT_EQ(rmmin, rdmmin);
}
//------------------------------------------------------------------------------
TEST(RationalTest, constructUnsigned)
{
    Rational r5(5, 1);
    Unsigned u5(5);
    Rational ru5(u5);
    EXPECT_EQ(r5, ru5);
    EXPECT_EQ(r5, Rational(Unsigned(5)));
}
//------------------------------------------------------------------------------
TEST(RationalTest, constructSigned)
{
    Rational r5(5, 1);
    Signed s5(5);
    Rational rs5(s5);
    EXPECT_EQ(r5, rs5);
    EXPECT_EQ(r5, Rational(Signed(5)));
}
//------------------------------------------------------------------------------
TEST(RationalTest, reciprocal)
{
    Rational r01;
    Rational rm12(-1, 2);
    Rational rm21(-2, 1);
    EXPECT_THROW(r01.reciprocal(), logic_error);
    EXPECT_EQ(rm21, rm12.reciprocal());
    EXPECT_EQ(rm12, rm21.reciprocal());
}
//------------------------------------------------------------------------------
TEST(RationalTest, operatorAssignPlus)
{
    Rational r12(1, 2);
    Rational rm12(-1, 2);
    Rational r11(1, 1);
    Rational r01;
    Rational r56(5, 6);
    Rational r38(3, 8);
    Rational r2924(29, 24);

    Rational actual;

    actual = r12;
    actual += rm12;
    EXPECT_EQ(r01, actual);

    actual = r12;
    actual += r12;
    EXPECT_EQ(r11, actual);

    actual = r56;
    actual += r38;
    EXPECT_EQ(r2924, actual);
}
//------------------------------------------------------------------------------
TEST(RationalTest, operatorAssignMinus)
{
    Rational r12(1, 2);
    Rational rm12(-1, 2);
    Rational r11(1, 1);
    Rational r01;
    Rational r56(5, 6);
    Rational r38(3, 8);
    Rational rm1124(-11, 24);

    Rational actual;

    actual = r12;
    actual -= rm12;
    EXPECT_EQ(r11, actual);

    actual = r12;
    actual -= r12;
    EXPECT_EQ(r01, actual);

    actual = r38;
    actual -= r56;
    EXPECT_EQ(rm1124, actual);
}
//------------------------------------------------------------------------------
TEST(RationalTest, operatorAssignTimes)
{
    Rational r12(1, 2);
    Rational rm12(-1, 2);
    Rational r14(1, 4);
    Rational rm14(-1, 4);
    Rational r01;

    Rational actual;

    actual = r12;
    actual *= r12;
    EXPECT_EQ(r14, actual);

    actual = r12;
    actual *= rm12;
    EXPECT_EQ(rm14, actual);

    actual = rm12;
    actual *= rm12;
    EXPECT_EQ(r14, actual);

    actual = rm12;
    actual *= r01;
    EXPECT_EQ(r01, actual);
}
//------------------------------------------------------------------------------
TEST(RationalTest, operatorAssignDiv)
{
    Rational r12(1, 2);
    Rational rm12(-1, 2);
    Rational r14(1, 4);
    Rational rm14(-1, 4);
    Rational r01;

    Rational actual;

    actual = r14;
    actual /= r12;
    EXPECT_EQ(r12, actual);

    actual = rm14;
    actual /= r12;
    EXPECT_EQ(rm12, actual);

    actual = rm14;
    actual /= rm12;
    EXPECT_EQ(r12, actual);

    actual = rm12;
    EXPECT_THROW(actual /= r01, invalid_argument);
}
//------------------------------------------------------------------------------
TEST(RationalTest, operatorDouble)
{
    EXPECT_EQ(0.0, static_cast<double>(Rational(0.0)));
    EXPECT_EQ(1.0, static_cast<double>(Rational(1.0)));
    EXPECT_EQ(-1.0, static_cast<double>(Rational(-1.0)));

    constexpr double dmin = numeric_limits<double>::denorm_min();
    Rational rdmin = dmin;
    EXPECT_EQ(dmin, static_cast<double>(rdmin));

    Rational hrdmin = rdmin * Rational(0.5);
    EXPECT_EQ(0.0, static_cast<double>(hrdmin));

    constexpr double nmax = numeric_limits<double>::max();
    Rational rnmax = nmax;
    EXPECT_EQ(nmax, static_cast<double>(rnmax));

    Rational drnmax = rnmax * 2.0;
    EXPECT_EQ(numeric_limits<double>::infinity(), static_cast<double>(drnmax));

    Rational trnmax = rnmax * 10.0;
    EXPECT_EQ(numeric_limits<double>::infinity(), static_cast<double>(trnmax));

    Rational runeeded(
        Signed(static_cast<uint64_t>(0x3FFFFFFFFFFFFFull)), Unsigned(1));
    double ruexpected = 0x40000000000000ull;
    EXPECT_EQ(ruexpected, static_cast<double>(runeeded));
}
//------------------------------------------------------------------------------
TEST(RationalTest, str)
{
    Rational rm12(-1, 2);
    EXPECT_EQ(string("-1/2"), rm12.str());
}
//------------------------------------------------------------------------------
TEST(RationalTest, comparisonEq)
{
    Rational u(18, 48);
    Rational v(3, 8);
    Rational w(1, 2);
    EXPECT_TRUE(u == v);
    EXPECT_FALSE(v == w);
}
//------------------------------------------------------------------------------
TEST(RationalTest, comparisonNEq)
{
    Rational u(18, 48);
    Rational v(3, 8);
    Rational w(1, 2);
    EXPECT_FALSE(u != v);
    EXPECT_TRUE(v != w);
}
//------------------------------------------------------------------------------
TEST(RationalTest, comparisonLt)
{
    Rational zero;
    Rational half(1, 2);
    Rational mhalf(-1, 2);

    EXPECT_TRUE(mhalf < half);
    EXPECT_FALSE(zero < mhalf);
    EXPECT_FALSE(zero < zero);
    EXPECT_FALSE(half < half);

    Unsigned max1d(numeric_limits<digit_t>::max());
    Unsigned min2d = Unsigned(1) << bitsPerDigit;

    Rational r12(max1d, min2d);
    Rational r21(min2d, max1d);
    EXPECT_TRUE(r12 < r21);
    EXPECT_FALSE(r21 < r12);

    Rational mr12(-Signed(max1d), min2d);
    Rational mr21(-Signed(min2d), max1d);
    EXPECT_FALSE(mr12 < mr21);
    EXPECT_TRUE(mr21 < mr12);

    Rational r37(3, 7);
    EXPECT_TRUE(r37 < half);
    EXPECT_FALSE(half < r37);
}
//------------------------------------------------------------------------------
TEST(RationalTest, comparisonGtEq)
{
    Rational zero;
    Rational half(1, 2);

    EXPECT_TRUE(half >= half);
    EXPECT_TRUE(half >= zero);
    EXPECT_FALSE(zero >= half);
}
//------------------------------------------------------------------------------
TEST(RationalTest, comparisonGt)
{
    Rational zero;
    Rational half(1, 2);

    EXPECT_FALSE(half > half);
    EXPECT_TRUE(half > zero);
    EXPECT_FALSE(zero > half);
}
//------------------------------------------------------------------------------
TEST(RationalTest, comparisonLtEq)
{
    Rational zero;
    Rational half(1, 2);

    EXPECT_TRUE(half <= half);
    EXPECT_FALSE(half <= zero);
    EXPECT_TRUE(zero <= half);
}
//------------------------------------------------------------------------------
TEST(RationalTest, operatorUnaryMinus)
{
    Rational half(1, 2);
    Rational mhalf(-1, 2);

    EXPECT_EQ(mhalf, -half);
    EXPECT_EQ(half, -mhalf);
}
//------------------------------------------------------------------------------
TEST(RationalTest, operatorPlus)
{
    Rational r12(1, 2);
    Rational rm12(-1, 2);
    Rational r11(1, 1);
    Rational r01;
    Rational r56(5, 6);
    Rational r38(3, 8);
    Rational r2924(29, 24);

    EXPECT_EQ(r01, r12 + rm12);
    EXPECT_EQ(r11, r12 + r12);
    EXPECT_EQ(r2924, r56 + r38);
}
//------------------------------------------------------------------------------
TEST(RationalTest, operatorMinus)
{
    Rational r12(1, 2);
    Rational rm12(-1, 2);
    Rational r11(1, 1);
    Rational r01;
    Rational r56(5, 6);
    Rational r38(3, 8);
    Rational rm1124(-11, 24);

    EXPECT_EQ(r11, r12 - rm12);
    EXPECT_EQ(r01, r12 - r12);
    EXPECT_EQ(rm1124, r38 - r56);
}
//------------------------------------------------------------------------------
TEST(RationalTest, operatorTimes)
{
    Rational r12(1, 2);
    Rational rm12(-1, 2);
    Rational r14(1, 4);
    Rational rm14(-1, 4);
    Rational r01;

    EXPECT_EQ(r14, r12 * r12);
    EXPECT_EQ(rm14, r12 * rm12);
    EXPECT_EQ(r14, rm12 * rm12);
    EXPECT_EQ(r01, rm12 * r01);
}
//------------------------------------------------------------------------------
TEST(RationalTest, operatorDiv)
{
    Rational r12(1, 2);
    Rational rm12(-1, 2);
    Rational r14(1, 4);
    Rational rm14(-1, 4);
    Rational r01;

    EXPECT_EQ(r12, r14 / r12);
    EXPECT_EQ(rm12, rm14 / r12);
    EXPECT_EQ(r12, rm14 / rm12);
    EXPECT_THROW(rm12 / r01, invalid_argument);
}
//------------------------------------------------------------------------------
TEST(RationalTest, operatorStream)
{
    Rational rm12(-1, 2);
    ostringstream os;
    os << rm12;
    EXPECT_EQ(string("-1/2"), os.str());
}
