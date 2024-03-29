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
#include "bignum.h"

#include <gmock/gmock.h>
//------------------------------------------------------------------------------
using namespace bn;
using namespace std;
//------------------------------------------------------------------------------
TEST(SignedTest, constructDefault)
{
    Signed u;
    EXPECT_EQ("0", u.str());
}
//------------------------------------------------------------------------------
TEST(SignedTest, constructFromUnsigned)
{
    Unsigned zero = 0;
    Unsigned one = 1;
    Signed szero(zero);
    Signed sone(one);
    EXPECT_EQ(zero.str(), szero.str());
    EXPECT_EQ(one.str(), sone.str());
}
//------------------------------------------------------------------------------
TEST(SignedTest, constructFromInt32)
{
    Signed s1 = static_cast<int32_t>(-1);
    Signed s2 = static_cast<int32_t>(0);
    Signed s3 = static_cast<int32_t>(1);
    EXPECT_EQ(string("-1"), s1.str());
    EXPECT_EQ(string("0"), s2.str());
    EXPECT_EQ(string("1"), s3.str());
}
//------------------------------------------------------------------------------
TEST(SignedTest, constructFromUInt32)
{
    Signed s1 = static_cast<uint32_t>(0);
    Signed s2 = static_cast<uint32_t>(1);
    EXPECT_EQ(string("0"), s1.str());
    EXPECT_EQ(string("1"), s2.str());
}
//------------------------------------------------------------------------------
TEST(SignedTest, constructFromInt64)
{
    Signed s1 = static_cast<int64_t>(-1);
    Signed s2 = static_cast<int64_t>(0);
    Signed s3 = static_cast<int64_t>(1);
    EXPECT_EQ(string("-1"), s1.str());
    EXPECT_EQ(string("0"), s2.str());
    EXPECT_EQ(string("1"), s3.str());
}
//------------------------------------------------------------------------------
TEST(SignedTest, constructFromUInt64)
{
    Signed s1 = static_cast<uint64_t>(0);
    Signed s2 = static_cast<uint64_t>(1);
    EXPECT_EQ(string("0"), s1.str());
    EXPECT_EQ(string("1"), s2.str());
}
//------------------------------------------------------------------------------
TEST(SignedTest, constructFromString)
{
    const char* mones = "-1";
    const char* mzeros = "-0";
    const char* zeros = "0";
    const char* ones = "1";

    Signed mone(mones);
    Signed mzero(mzeros);
    Signed zero(zeros);
    Signed one(ones);

    EXPECT_EQ(mones, mone.str());
    EXPECT_EQ(zeros, mzero.str());
    EXPECT_EQ(zeros, zero.str());
    EXPECT_EQ(ones, one.str());
}
//------------------------------------------------------------------------------
TEST(SignedTest, abs)
{
    Unsigned one = 1;
    Signed sone(one);
    EXPECT_EQ(one, sone.abs());
}
//------------------------------------------------------------------------------
TEST(SignedTest, operatorPlusPlus)
{
    Signed mone = -1;
    Signed zero = 0;
    Signed one = 1;
    Signed two = 2;

    Signed actual;

    actual = mone;
    ++actual;
    EXPECT_EQ(zero, actual);
    ++actual;
    EXPECT_EQ(one, actual);
    ++actual;
    EXPECT_EQ(two, actual);

    actual = zero;
    Signed copy = actual++;
    EXPECT_EQ(zero, copy);
    EXPECT_EQ(one, actual);
}
//------------------------------------------------------------------------------
TEST(SignedTest, operatorMinusMiunus)
{
    Signed mtwo = -2;
    Signed mone = -1;
    Signed zero = 0;
    Signed one = 1;

    Signed actual;

    actual = one;
    --actual;
    EXPECT_EQ(zero, actual);
    --actual;
    EXPECT_EQ(mone, actual);
    --actual;
    EXPECT_EQ(mtwo, actual);

    actual = zero;
    Signed copy = actual--;
    EXPECT_EQ(zero, copy);
    EXPECT_EQ(mone, actual);
}
//------------------------------------------------------------------------------
TEST(SignedTest, operatorAssignPlus)
{
    Signed zero = 0;
    Signed mtwo = -2;
    Signed mone = -1;
    Signed one = 1;
    Signed two = 2;

    Signed actual;

    actual = zero;
    actual += one;
    EXPECT_EQ(one, actual);

    actual = one;
    actual += zero;
    EXPECT_EQ(one, actual);

    actual = mone;
    actual += mone;
    EXPECT_EQ(mtwo, actual);

    actual = one;
    actual += mtwo;
    EXPECT_EQ(mone, actual);

    actual = two;
    actual += mone;
    EXPECT_EQ(one, actual);

    actual = one;
    actual += mone;
    EXPECT_EQ(zero, actual);

    actual = mone;
    actual += one;
    EXPECT_EQ(zero, actual);
}
//------------------------------------------------------------------------------
TEST(SignedTest, operatorAssignMinus)
{
    Signed zero = 0;
    Signed mtwo = -2;
    Signed mone = -1;
    Signed one = 1;
    Signed two = 2;

    Signed actual;

    actual = zero;
    actual -= one;
    EXPECT_EQ(mone, actual);

    actual = one;
    actual -= zero;
    EXPECT_EQ(one, actual);

    actual = one;
    actual -= mone;
    EXPECT_EQ(two, actual);

    actual = two;
    actual -= one;
    EXPECT_EQ(one, actual);

    actual = one;
    actual -= two;
    EXPECT_EQ(mone, actual);

    actual = mtwo;
    actual -= mone;
    EXPECT_EQ(mone, actual);

    actual = mone;
    actual -= mtwo;
    EXPECT_EQ(one, actual);

    actual = one;
    actual -= one;
    EXPECT_EQ(zero, actual);
}
//------------------------------------------------------------------------------
TEST(SignedTest, operatorAssignMul)
{
    Signed zero = 0;
    Signed mtwo = -2;
    Signed mone = -1;
    Signed one = 1;
    Signed two = 2;

    Signed actual;

    actual = zero;
    actual *= zero;
    EXPECT_EQ(zero, actual);

    actual = one;
    actual *= zero;
    EXPECT_EQ(zero, actual);

    actual = mone;
    actual *= mone;
    EXPECT_EQ(one, actual);

    actual = two;
    actual *= mone;
    EXPECT_EQ(mtwo, actual);
}
//------------------------------------------------------------------------------
TEST(SignedTest, operatorAssignDiv)
{
    Signed one = 1;
    Signed three = 3;
    Signed five = 5;

    Signed actual = five;
    actual /= three;
    EXPECT_EQ(one, actual);
}
//------------------------------------------------------------------------------
TEST(SignedTest, operatorAssignMod)
{
    Signed two = 2;
    Signed three = 3;
    Signed five = 5;

    Signed actual = five;
    actual %= three;
    EXPECT_EQ(two, actual);
}
//------------------------------------------------------------------------------
TEST(SignedTest, memberDiv)
{
    Signed mfive = -5;
    Signed mthree = -3;
    Signed mtwo = -2;
    Signed mone = -1;
    Signed zero = 0;
    Signed one = 1;
    Signed two = 2;
    Signed three = 3;
    Signed five = 5;

    Signed actual;
    Signed rem;

    actual = five;
    rem = actual.div(three);
    EXPECT_EQ(one, actual);
    EXPECT_EQ(two, rem);

    actual = mfive;
    rem = actual.div(three);
    EXPECT_EQ(mone, actual);
    EXPECT_EQ(mtwo, rem);

    actual = five;
    rem = actual.div(mthree);
    EXPECT_EQ(mone, actual);
    EXPECT_EQ(two, rem);

    actual = mfive;
    rem = actual.div(mthree);
    EXPECT_EQ(one, actual);
    EXPECT_EQ(mtwo, rem);

    Signed::QR qr5 = div(zero, three);
    EXPECT_EQ(zero, qr5.quot);
    EXPECT_EQ(zero, qr5.rem);
}
//------------------------------------------------------------------------------
TEST(SignedTest, comparisonEq)
{
    Signed zero = 0;
    Signed one = 1;
    EXPECT_TRUE(zero == zero);
    EXPECT_FALSE(zero == one);
}
//------------------------------------------------------------------------------
TEST(SignedTest, comparisonNEq)
{
    Signed zero = 0;
    Signed one = 1;
    EXPECT_FALSE(zero != zero);
    EXPECT_TRUE(zero != one);
}
//------------------------------------------------------------------------------
TEST(SignedTest, comparisonLT)
{
    Signed zero = 0;
    Signed one = 1;
    Signed two = 2;
    EXPECT_TRUE(zero < one);
    EXPECT_FALSE(one < zero);
    EXPECT_TRUE(one < two);
}
//------------------------------------------------------------------------------
TEST(SignedTest, comparisonGET)
{
    Signed zero = 0;
    Signed one = 1;
    Signed two = 2;
    EXPECT_FALSE(zero >= one);
    EXPECT_TRUE(one >= zero);
    EXPECT_FALSE(one >= two);
}
//------------------------------------------------------------------------------
TEST(SignedTest, comparisonGT)
{
    Signed zero = 0;
    Signed one = 1;
    Signed two = 2;
    EXPECT_TRUE(one > zero);
    EXPECT_FALSE(zero > one);
    EXPECT_TRUE(two > one);
}
//------------------------------------------------------------------------------
TEST(SignedTest, comparisonLET)
{
    Signed zero = 0;
    Signed one = 1;
    Signed two = 2;
    EXPECT_FALSE(one <= zero);
    EXPECT_TRUE(zero <= one);
    EXPECT_FALSE(two <= one);
}
//------------------------------------------------------------------------------
TEST(SignedTest, operatorUnaryMinus)
{
    Signed mone = -1;
    Signed zero = 0;
    Signed one = 1;

    EXPECT_EQ(one, -mone);
    EXPECT_EQ(zero, -zero);
    EXPECT_EQ(mone, -one);
}
//------------------------------------------------------------------------------
TEST(SignedTest, operatorPlus)
{
    Signed zero = 0;
    Signed mtwo = -2;
    Signed mone = -1;
    Signed one = 1;
    Signed two = 2;

    EXPECT_EQ(one, zero + one);
    EXPECT_EQ(one, one + zero);
    EXPECT_EQ(mtwo, mone + mone);
    EXPECT_EQ(mone, one + mtwo);
    EXPECT_EQ(one, two + mone);
    EXPECT_EQ(zero, one + mone);
    EXPECT_EQ(zero, mone + one);
}
//------------------------------------------------------------------------------
TEST(SignedTest, operatorMinus)
{
    Signed zero = 0;
    Signed mtwo = -2;
    Signed mone = -1;
    Signed one = 1;
    Signed two = 2;

    EXPECT_EQ(mone, zero - one);
    EXPECT_EQ(one, one - zero);
    EXPECT_EQ(two, one - mone);
    EXPECT_EQ(one, two - one);
    EXPECT_EQ(mone, one - two);
    EXPECT_EQ(mone, mtwo - mone);
    EXPECT_EQ(one, mone - mtwo);
    EXPECT_EQ(zero, one - one);
}
//------------------------------------------------------------------------------
TEST(SignedTest, operatorMul)
{
    Signed zero = 0;
    Signed mtwo = -2;
    Signed mone = -1;
    Signed one = 1;
    Signed two = 2;

    EXPECT_EQ(zero, zero * zero);
    EXPECT_EQ(zero, one * zero);
    EXPECT_EQ(one, mone * mone);
    EXPECT_EQ(mtwo, two * mone);
}
//------------------------------------------------------------------------------
TEST(SignedTest, operatorDiv)
{
    Signed one = 1;
    Signed three = 3;
    Signed five = 5;
    EXPECT_EQ(one, five / three);
}
//------------------------------------------------------------------------------
TEST(SignedTest, operatorMod)
{
    Signed two = 2;
    Signed three = 3;
    Signed five = 5;
    EXPECT_EQ(two, five % three);
}
//------------------------------------------------------------------------------
TEST(SignedTest, div)
{
    Signed mfive = -5;
    Signed mthree = -3;
    Signed mtwo = -2;
    Signed mone = -1;
    Signed zero = 0;
    Signed one = 1;
    Signed two = 2;
    Signed three = 3;
    Signed five = 5;

    Signed::QR qr1 = div(five, three);
    EXPECT_EQ(one, qr1.quot);
    EXPECT_EQ(two, qr1.rem);

    Signed::QR qr2 = div(mfive, three);
    EXPECT_EQ(mone, qr2.quot);
    EXPECT_EQ(mtwo, qr2.rem);

    Signed::QR qr3 = div(five, mthree);
    EXPECT_EQ(mone, qr3.quot);
    EXPECT_EQ(two, qr3.rem);

    Signed::QR qr4 = div(mfive, mthree);
    EXPECT_EQ(one, qr4.quot);
    EXPECT_EQ(mtwo, qr4.rem);

    Signed::QR qr5 = div(zero, three);
    EXPECT_EQ(zero, qr5.quot);
    EXPECT_EQ(zero, qr5.rem);
}
//------------------------------------------------------------------------------
TEST(SignedTest, operatorStream)
{
    Signed mone = -1;
    Signed zero = 0;
    Signed one = 1;
    ostringstream os1, os2, os3;
    os1 << mone;
    os2 << zero;
    os3 << one;
    EXPECT_EQ(mone.str(), os1.str());
    EXPECT_EQ(zero.str(), os2.str());
    EXPECT_EQ(one.str(), os3.str());
}
