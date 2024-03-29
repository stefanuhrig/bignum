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
#include "uint128.h"

#include <gmock/gmock.h>
#include <sstream>
#include <string>
#include <utility>
//------------------------------------------------------------------------------
using namespace bn;
using namespace std;
//------------------------------------------------------------------------------
namespace bn {
//------------------------------------------------------------------------------
template<>
struct EnableUserDefinedIntegral<uint128> : true_type
{
};
//------------------------------------------------------------------------------
}  // namespace bn
//------------------------------------------------------------------------------
template<typename T>
struct DDigitHelper
{
};
template<>
struct DDigitHelper<uint8_t>
{
    using type = uint16_t;
};
template<>
struct DDigitHelper<uint16_t>
{
    using type = uint32_t;
};
template<>
struct DDigitHelper<uint32_t>
{
    using type = uint64_t;
};
template<>
struct DDigitHelper<uint64_t>
{
    using type = uint128;
};
//------------------------------------------------------------------------------
template<typename T>
struct SignedHelper
{
};
template<>
struct SignedHelper<uint8_t>
{
    using type = int8_t;
};
template<>
struct SignedHelper<uint16_t>
{
    using type = int16_t;
};
template<>
struct SignedHelper<uint32_t>
{
    using type = int32_t;
};
template<>
struct SignedHelper<uint64_t>
{
    using type = int64_t;
};
template<>
struct SignedHelper<uint128>
{
    using type = uint128;
};
//------------------------------------------------------------------------------
using digit_t = impl::digit_t;
using ddigit_t = DDigitHelper<digit_t>::type;
constexpr size_t bitsPerDigit = impl::bitsPerDigit;
//------------------------------------------------------------------------------
TEST(UnsignedTest, constructDefault)
{
    Unsigned u;
    EXPECT_EQ("0", u.str());
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, constructFromIntegral)
{
    int32_t neg32 = -1;
    EXPECT_THROW(Unsigned temp1(neg32), invalid_argument);
    int64_t neg64 = -1;
    EXPECT_THROW(Unsigned temp2(neg64), invalid_argument);

    digit_t digit = (static_cast<digit_t>(1) << (8 * sizeof(digit_t) - 1))
                  | static_cast<digit_t>(1);
    ddigit_t ddigit = (static_cast<ddigit_t>(1) << (8 * sizeof(ddigit_t) - 1))
                    | static_cast<ddigit_t>(1);
    make_signed<digit_t>::type ndigit =
        (static_cast<digit_t>(1) << (8 * sizeof(digit_t) - 2))
        | static_cast<digit_t>(1);
    SignedHelper<ddigit_t>::type nddigit =
        (static_cast<ddigit_t>(1) << (8 * sizeof(ddigit_t) - 2))
        | static_cast<ddigit_t>(1);

    ostringstream osd, osdd, osnd, osndd;
    if (sizeof(digit_t) < 4) {
        osd << static_cast<unsigned>(digit);
        osdd << static_cast<unsigned>(ddigit);
        osnd << static_cast<unsigned>(ndigit);
        osndd << static_cast<unsigned>(nddigit);
    } else {
        osd << digit;
        osdd << ddigit;
        osnd << ndigit;
        osndd << nddigit;
    }

    Unsigned ad(digit);
    Unsigned add(ddigit);
    Unsigned ansd(ndigit);
    Unsigned andd(nddigit);

    EXPECT_EQ(osd.str(), ad.str());
    EXPECT_EQ(osdd.str(), add.str());
    EXPECT_EQ(osnd.str(), ansd.str());
    EXPECT_EQ(osndd.str(), andd.str());
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, constructFromString)
{
    const char* us = "123456789012345678901234567890";
    Unsigned u = us;
    EXPECT_EQ(us, u.str());
    EXPECT_THROW(Unsigned temp1(""), invalid_argument);
    EXPECT_THROW(Unsigned temp2("a"), invalid_argument);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, randomCreate)
{
    std::mt19937 gen(0);
    Unsigned zero;

    Unsigned z = Unsigned::random(0, gen);
    EXPECT_EQ(zero, z);
    Unsigned u = Unsigned::random(256, gen);
    Unsigned v = Unsigned::random(258, gen);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorPlusPlus)
{
    Unsigned one = 1;
    Unsigned two = 2;
    Unsigned three = 3;

    Unsigned u = one;
    EXPECT_EQ(two, ++u);
    Unsigned v = u++;
    EXPECT_EQ(two, v);
    EXPECT_EQ(three, u);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorMinusMinus)
{
    Unsigned zero = 0;
    Unsigned one = 1;
    Unsigned two = 2;
    Unsigned sone = one << bitsPerDigit;
    Unsigned all = Unsigned(numeric_limits<digit_t>::max());

    Unsigned u = two;
    EXPECT_EQ(one, --u);
    Unsigned v = u--;
    EXPECT_EQ(one, v);
    EXPECT_EQ(zero, u);
    EXPECT_THROW(--zero, logic_error);
    Unsigned w = sone;
    --w;
    EXPECT_EQ(all, w);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorAssignOr)
{
    Unsigned three = 3;
    Unsigned six = 6;
    Unsigned seven = 7;
    Unsigned ods = Unsigned(static_cast<ddigit_t>(1) << bitsPerDigit);
    Unsigned ods7 = Unsigned((static_cast<ddigit_t>(1) << bitsPerDigit) | 7u);

    Unsigned actual;
    actual = three;
    actual |= six;
    EXPECT_EQ(seven, actual);

    actual = ods;
    actual |= seven;
    EXPECT_EQ(ods7, actual);

    actual = seven;
    actual |= ods;
    EXPECT_EQ(ods7, actual);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorAssignAnd)
{
    Unsigned zero = 0;
    Unsigned one = 1;
    Unsigned two = 2;
    Unsigned three = 3;
    Unsigned six = 6;

    Unsigned actual;

    actual = one;
    actual &= zero;
    EXPECT_EQ(zero, actual);

    actual = two;
    actual &= one;
    EXPECT_EQ(zero, actual);

    actual = three;
    actual &= six;
    EXPECT_EQ(two, actual);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorAssignXor)
{
    Unsigned one = 1;
    Unsigned three = 3;
    Unsigned five = 5;
    Unsigned six = 6;
    Unsigned ods = Unsigned(static_cast<ddigit_t>(1) << bitsPerDigit);
    Unsigned ods1 = Unsigned((static_cast<ddigit_t>(1) << bitsPerDigit) | 1u);
    Unsigned ods7 = Unsigned((static_cast<ddigit_t>(1) << bitsPerDigit) | 7u);

    Unsigned actual;

    actual = three;
    actual ^= six;
    EXPECT_EQ(five, actual);

    actual = ods1;
    actual ^= ods7;
    EXPECT_EQ(six, actual);

    actual = ods;
    actual ^= one;
    EXPECT_EQ(ods1, actual);

    actual = one;
    actual ^= ods;
    EXPECT_EQ(ods1, actual);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorAssignLeftShift)
{
    Unsigned zero = 0;
    Unsigned one = 1;
    Unsigned four = 4;
    Unsigned ods = Unsigned(static_cast<ddigit_t>(1) << bitsPerDigit);
    Unsigned lsou = Unsigned(
        (static_cast<ddigit_t>(1) << bitsPerDigit)
        | numeric_limits<digit_t>::max());
    Unsigned lsous2 = Unsigned(
        (static_cast<ddigit_t>(7) << bitsPerDigit)
        | (numeric_limits<digit_t>::max() - 3));
    Unsigned u4 = Unsigned(static_cast<ddigit_t>(4) << bitsPerDigit);

    Unsigned actual;

    actual = zero;
    actual <<= 20;
    EXPECT_EQ(zero, actual);

    actual = one;
    actual <<= 0;
    EXPECT_EQ(one, actual);

    actual = one;
    actual <<= bitsPerDigit;
    EXPECT_EQ(ods, actual);

    actual = one;
    actual <<= 2;
    EXPECT_EQ(four, actual);

    actual = four;
    actual <<= (bitsPerDigit - 2);
    EXPECT_EQ(ods, actual);

    actual = lsou;
    actual <<= 2;
    EXPECT_EQ(lsous2, actual);

    actual = one;
    actual <<= (bitsPerDigit + 2);
    EXPECT_EQ(u4, actual);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorAssignRightShift)
{
    Unsigned zero = 0;
    Unsigned one = 1;
    Unsigned four = 4;
    Unsigned ods = Unsigned(static_cast<ddigit_t>(1) << bitsPerDigit);
    Unsigned lsou = Unsigned(
        (static_cast<ddigit_t>(1) << bitsPerDigit)
        | numeric_limits<digit_t>::max());
    Unsigned lsous2 = Unsigned(
        (static_cast<ddigit_t>(7) << bitsPerDigit)
        | (numeric_limits<digit_t>::max() - 3));
    Unsigned u4 = Unsigned(static_cast<ddigit_t>(4) << bitsPerDigit);

    Unsigned actual;

    actual = one;
    actual >>= 0;
    EXPECT_EQ(one, actual);

    actual = zero;
    actual >>= 20;
    EXPECT_EQ(zero, actual);

    actual = one;
    actual >>= 1;
    EXPECT_EQ(zero, actual);

    actual = u4;
    actual >>= bitsPerDigit;
    EXPECT_EQ(four, actual);

    actual = four;
    actual >>= 2;
    EXPECT_EQ(one, actual);

    actual = ods;
    actual >>= (bitsPerDigit - 2);
    EXPECT_EQ(four, actual);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorAssignPlus)
{
    Unsigned zero = 0;
    Unsigned one = 1;
    Unsigned two = 2;
    Unsigned hb = Unsigned(static_cast<digit_t>(1) << (bitsPerDigit - 1));
    Unsigned hbsum = Unsigned(static_cast<ddigit_t>(1) << bitsPerDigit);
    Unsigned lset = Unsigned(
        numeric_limits<digit_t>::max()
        | (static_cast<ddigit_t>(1) << bitsPerDigit));
    Unsigned lsetp1 = Unsigned(static_cast<ddigit_t>(1) << (bitsPerDigit + 1));
    Unsigned set2 = (Unsigned(numeric_limits<digit_t>::max()) << bitsPerDigit)
                  | Unsigned(numeric_limits<digit_t>::max());
    Unsigned h3 = one << (2 * bitsPerDigit);

    Unsigned actual;

    actual = zero;
    actual += one;
    EXPECT_EQ(one, actual);

    actual = one;
    actual += zero;
    EXPECT_EQ(one, actual);

    actual = one;
    actual += one;
    EXPECT_EQ(two, actual);

    actual = hb;
    actual += hb;
    EXPECT_EQ(hbsum, actual);

    actual = lset;
    actual += one;
    EXPECT_EQ(lsetp1, actual);

    actual = one;
    actual += lset;
    EXPECT_EQ(lsetp1, actual);

    actual = one;
    actual += set2;
    EXPECT_EQ(h3, actual);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorAssignMinus)
{
    Unsigned zero = 0;
    Unsigned one = 1;
    Unsigned two = 2;
    Unsigned sone = Unsigned(static_cast<ddigit_t>(1) << bitsPerDigit);
    Unsigned allset = Unsigned(numeric_limits<digit_t>::max());
    Unsigned s2one = sone << bitsPerDigit;
    Unsigned allset2 = (allset << bitsPerDigit) | allset;

    Unsigned actual;

    actual = zero;
    EXPECT_THROW(actual -= one, std::invalid_argument);

    actual = one;
    actual -= zero;
    EXPECT_EQ(one, actual);

    actual = two;
    actual -= one;
    EXPECT_EQ(one, actual);

    actual = one;
    EXPECT_THROW(actual -= two, std::invalid_argument);

    actual = sone;
    actual -= allset;
    EXPECT_EQ(one, actual);

    actual = s2one;
    actual -= one;
    EXPECT_EQ(allset2, actual);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorAssignTimes)
{
    Unsigned three = 3;
    Unsigned seven = 7;
    Unsigned twentyone = 21;

    Unsigned actual = three;
    actual *= seven;
    EXPECT_EQ(twentyone, actual);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorAssignDiv)
{
    Unsigned three = 3;
    Unsigned seven = 7;
    Unsigned twentysix = 26;

    Unsigned actual = twentysix;
    actual /= seven;
    EXPECT_EQ(three, actual);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorAssignMod)
{
    Unsigned five = 5;
    Unsigned seven = 7;
    Unsigned twentysix = 26;

    Unsigned actual = twentysix;
    actual %= seven;
    EXPECT_EQ(five, actual);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, memberDiv)
{
    Unsigned three = 3;
    Unsigned five = 5;
    Unsigned seven = 7;
    Unsigned twentysix = 26;

    Unsigned actual = twentysix;
    Unsigned rem = actual.div(seven);
    EXPECT_EQ(three, actual);
    EXPECT_EQ(five, rem);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, empty)
{
    Unsigned zero = 0;
    Unsigned one = 1;
    EXPECT_TRUE(zero.empty());
    EXPECT_FALSE(one.empty());
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, digits)
{
    Unsigned zero = 0;
    Unsigned one = 1;
    Unsigned high = one << (bitsPerDigit - 1);
    Unsigned oneh = one << bitsPerDigit;

    EXPECT_EQ(0, zero.digits());
    EXPECT_EQ(1, one.digits());
    EXPECT_EQ(1, high.digits());
    EXPECT_EQ(2, oneh.digits());
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, bits)
{
    Unsigned zero = 0;
    Unsigned one = 1;
    Unsigned high = one << (bitsPerDigit - 1);
    Unsigned oneh = one << bitsPerDigit;

    EXPECT_EQ(0, zero.bits());
    EXPECT_EQ(1, one.bits());
    EXPECT_EQ(bitsPerDigit, high.bits());
    EXPECT_EQ(bitsPerDigit + 1, oneh.bits());
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorUint64)
{
    Unsigned of = Unsigned(1) << 64;
    EXPECT_THROW(static_cast<uint64_t>(of), overflow_error);
    Unsigned m = numeric_limits<uint64_t>::max();
    EXPECT_EQ(numeric_limits<uint64_t>::max(), static_cast<uint64_t>(m));
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, comparisonEqAndNEq)
{
    Unsigned zero = 0;
    Unsigned one1 = 1;
    Unsigned one2 = 1;
    Unsigned two = 2;

    EXPECT_FALSE(zero == one1);
    EXPECT_TRUE(one1 == one2);
    EXPECT_FALSE(one1 == two);

    EXPECT_TRUE(zero != one1);
    EXPECT_FALSE(one1 != one2);
    EXPECT_TRUE(one1 != two);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, comparisonLTAndNGET)
{
    Unsigned zero = 0;
    Unsigned one1 = 1;
    Unsigned one2 = 1;
    Unsigned two = 2;

    EXPECT_TRUE(zero < one1);
    EXPECT_FALSE(one1 < zero);
    EXPECT_FALSE(one1 < one2);
    EXPECT_TRUE(one1 < two);

    EXPECT_FALSE(zero >= one1);
    EXPECT_TRUE(one1 >= zero);
    EXPECT_TRUE(one1 >= one2);
    EXPECT_FALSE(one1 >= two);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, comparisonGTAndNLET)
{
    Unsigned zero = 0;
    Unsigned one1 = 1;
    Unsigned one2 = 1;
    Unsigned two = 2;

    EXPECT_FALSE(zero > one1);
    EXPECT_TRUE(one1 > zero);
    EXPECT_FALSE(one1 > one2);
    EXPECT_FALSE(one1 > two);

    EXPECT_TRUE(zero <= one1);
    EXPECT_FALSE(one1 <= zero);
    EXPECT_TRUE(one1 <= one2);
    EXPECT_TRUE(one1 <= two);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorOr)
{
    Unsigned three = 3;
    Unsigned six = 6;
    Unsigned seven = 7;
    Unsigned ods = Unsigned(static_cast<ddigit_t>(1) << bitsPerDigit);
    Unsigned ods7 = Unsigned((static_cast<ddigit_t>(1) << bitsPerDigit) | 7u);

    EXPECT_EQ(seven, three | six);
    EXPECT_EQ(ods7, ods | seven);
    EXPECT_EQ(ods7, seven | ods);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorAnd)
{
    Unsigned zero = 0;
    Unsigned one = 1;
    Unsigned two = 2;
    Unsigned three = 3;
    Unsigned six = 6;

    EXPECT_EQ(zero, one & zero);
    EXPECT_EQ(zero, two & one);
    EXPECT_EQ(two, three & six);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorXor)
{
    Unsigned one = 1;
    Unsigned three = 3;
    Unsigned five = 5;
    Unsigned six = 6;
    Unsigned ods = Unsigned(static_cast<ddigit_t>(1) << bitsPerDigit);
    Unsigned ods1 = Unsigned(static_cast<ddigit_t>(1) << bitsPerDigit | 1u);
    Unsigned ods7 = Unsigned((static_cast<ddigit_t>(1) << bitsPerDigit) | 7u);

    EXPECT_EQ(five, three ^ six);
    EXPECT_EQ(six, ods1 ^ ods7);
    EXPECT_EQ(ods1, ods ^ one);
    EXPECT_EQ(ods1, one ^ ods);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorLeftShift)
{
    Unsigned zero = 0;
    Unsigned one = 1;
    Unsigned four = 4;
    Unsigned ods = Unsigned(static_cast<ddigit_t>(1) << bitsPerDigit);
    Unsigned lsou = Unsigned(
        (static_cast<ddigit_t>(1) << bitsPerDigit)
        | numeric_limits<digit_t>::max());
    Unsigned lsous2 = Unsigned(
        (static_cast<ddigit_t>(7) << bitsPerDigit)
        | (numeric_limits<digit_t>::max() - 3));
    Unsigned u4 = Unsigned(static_cast<ddigit_t>(4) << bitsPerDigit);

    EXPECT_EQ(zero, zero << 20);
    EXPECT_EQ(one, one << 0);
    EXPECT_EQ(ods, one << bitsPerDigit);
    EXPECT_EQ(four, one << 2);
    EXPECT_EQ(ods, four << (bitsPerDigit - 2));
    EXPECT_EQ(lsous2, lsou << 2);
    EXPECT_EQ(u4, one << (bitsPerDigit + 2));
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorRightShift)
{
    Unsigned zero = 0;
    Unsigned one = 1;
    Unsigned four = 4;
    Unsigned ods = Unsigned(static_cast<ddigit_t>(1) << bitsPerDigit);
    Unsigned lsou = Unsigned(
        (static_cast<ddigit_t>(1) << bitsPerDigit)
        | numeric_limits<digit_t>::max());
    Unsigned lsous2 = Unsigned(
        (static_cast<ddigit_t>(7) << bitsPerDigit)
        | (numeric_limits<digit_t>::max() - 3));
    Unsigned u4 = Unsigned(static_cast<ddigit_t>(4) << bitsPerDigit);

    EXPECT_EQ(one, one >> 0);
    EXPECT_EQ(zero, zero >> 20);
    EXPECT_EQ(zero, one >> 1);
    EXPECT_EQ(four, u4 >> bitsPerDigit);
    EXPECT_EQ(one, four >> 2);
    EXPECT_EQ(four, ods >> (bitsPerDigit - 2));
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorPlus)
{
    Unsigned zero = 0;
    Unsigned one = 1;
    Unsigned two = 2;
    Unsigned hb = Unsigned(static_cast<digit_t>(1) << (bitsPerDigit - 1));
    Unsigned hbsum = Unsigned(static_cast<ddigit_t>(1) << bitsPerDigit);
    Unsigned lset = Unsigned(
        numeric_limits<digit_t>::max()
        | (static_cast<ddigit_t>(1) << bitsPerDigit));
    Unsigned lsetp1 = Unsigned(static_cast<ddigit_t>(1) << (bitsPerDigit + 1));

    EXPECT_EQ(one, zero + one);
    EXPECT_EQ(one, one + zero);
    EXPECT_EQ(two, one + one);
    EXPECT_EQ(hbsum, hb + hb);
    EXPECT_EQ(lsetp1, lset + one);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorMinus)
{
    Unsigned zero = 0;
    Unsigned one = 1;
    Unsigned two = 2;
    Unsigned sone = Unsigned(static_cast<ddigit_t>(1) << bitsPerDigit);
    Unsigned allset = Unsigned(numeric_limits<digit_t>::max());
    Unsigned s2one = sone << bitsPerDigit;
    Unsigned allset2 = (allset << bitsPerDigit) | allset;

    EXPECT_THROW(zero - one, std::invalid_argument);
    EXPECT_EQ(one, one - zero);
    EXPECT_EQ(one, two - one);
    EXPECT_THROW(one - two, std::invalid_argument);
    EXPECT_EQ(one, sone - allset);
    EXPECT_EQ(allset2, s2one - one);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorTimes)
{
    Unsigned zero = 0;
    Unsigned one = 1;
    Unsigned two = 2;
    Unsigned three = 3;
    Unsigned six = 6;
    Unsigned allset = Unsigned(numeric_limits<digit_t>::max());
    Unsigned product =
        (one << (2 * bitsPerDigit)) - (one << (bitsPerDigit + 1)) + one;

    EXPECT_EQ(zero, two * zero);
    EXPECT_EQ(zero, zero * two);
    EXPECT_EQ(six, two * three);
    EXPECT_EQ(product, allset * allset);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorDiv)
{
    Unsigned seven = 7;
    Unsigned three = 3;
    Unsigned two = 2;
    EXPECT_EQ(two, seven / three);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorMod)
{
    Unsigned seven = 7;
    Unsigned three = 3;
    Unsigned one = 1;
    EXPECT_EQ(one, seven % three);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, div)
{
    Unsigned zero = 0;
    Unsigned one = 1;
    Unsigned two = 2;
    Unsigned three = 3;
    Unsigned hset = one << (bitsPerDigit - 1);
    Unsigned hhset = (hset << bitsPerDigit) | hset;
    Unsigned sone = one << bitsPerDigit;
    Unsigned ssone = sone << bitsPerDigit;
    Unsigned uab = (one << (3 * bitsPerDigit)) | (two << bitsPerDigit);
    Unsigned vab = (hset << (2 * bitsPerDigit)) | (one << bitsPerDigit) | hset;
    Unsigned b2p2 = (one << 2 * bitsPerDigit) | two;
    Unsigned b1p1 = (one << bitsPerDigit) | one;
    Unsigned allset = numeric_limits<digit_t>::max();

    EXPECT_THROW(div(one, zero), invalid_argument);
    EXPECT_EQ((Unsigned::QR{zero, two}), div(two, sone));
    EXPECT_EQ((Unsigned::QR{zero, one}), div(one, two));
    EXPECT_EQ((Unsigned::QR{one, zero}), div(sone, sone));
    EXPECT_EQ((Unsigned::QR{one, uab - vab}), div(uab, vab));
    EXPECT_EQ((Unsigned::QR{one, ssone - hhset}), div(ssone, hhset));
    EXPECT_EQ((Unsigned::QR{sone, zero}), div(ssone, sone));
    EXPECT_EQ((Unsigned::QR{allset, three}), div(b2p2, b1p1));
}
//------------------------------------------------------------------------------
TEST(Unsigned, divQuotientFind)
{
    Unsigned::QR qr;

    qr = div(Unsigned("66302"), Unsigned("259"));
    EXPECT_EQ(Unsigned("255"), qr.quot);
    EXPECT_EQ(Unsigned("257"), qr.rem);

    qr = div(Unsigned("131072"), Unsigned("515"));
    EXPECT_EQ(Unsigned("254"), qr.quot);
    EXPECT_EQ(Unsigned("262"), qr.rem);

    qr = div(Unsigned("131584"), Unsigned("515"));
    EXPECT_EQ(Unsigned("255"), qr.quot);
    EXPECT_EQ(Unsigned("259"), qr.rem);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, pow)
{
    Unsigned base = 23;
    std::size_t exp = 1000;
    Unsigned expected = 1;
    for (std::size_t i = 0; i < exp; ++i) {
        expected *= base;
    }
    Unsigned actual = pow(base, exp);
    EXPECT_EQ(expected, actual);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, powmod)
{
    Unsigned base = 23;
    std::size_t exp = 1000;
    Unsigned mod = 1000;
    Unsigned expected = 1;
    for (std::size_t i = 0; i < exp; ++i) {
        expected *= base;
    }
    expected %= mod;
    Unsigned actual = powmod(base, exp, mod);
    EXPECT_EQ(expected, actual);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, sqrt)
{
    EXPECT_EQ(Unsigned(0), sqrt(Unsigned(0)));
    for (Unsigned n = 1; n <= 1000; ++n) {
        Unsigned s = sqrt(n);
        EXPECT_GE(n, s * s);
        EXPECT_LT(n, (s + 1) * (s + 1));
    }
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, zeroGcd)
{
    Unsigned zero = 0;
    Unsigned two = 2;

    EXPECT_EQ(zero, egcd(zero, zero));
    EXPECT_EQ(two, egcd(zero, two));
    EXPECT_EQ(two, egcd(two, zero));

    EXPECT_EQ(zero, bgcd(zero, zero));
    EXPECT_EQ(two, bgcd(zero, two));
    EXPECT_EQ(two, bgcd(two, zero));

    EXPECT_EQ(zero, gcd(zero, zero));
    EXPECT_EQ(two, gcd(zero, two));
    EXPECT_EQ(two, gcd(two, zero));
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, simpleGcd)
{
    Unsigned a = 18;
    Unsigned b = 48;
    Unsigned exp = 6;

    Unsigned actual;
    actual = gcd(a, b);
    EXPECT_EQ(exp, actual);
    actual = gcd(b, a);
    EXPECT_EQ(exp, actual);
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, largeGcd)
{
    Unsigned primes[12] = {
        Unsigned("2"),
        Unsigned("8683317618811886495518194401279999999"),
        Unsigned("1066340417491710595814572169"),
        Unsigned("19134702400093278081449423917"),
        Unsigned("3331113965338635107"),
        Unsigned("6161791591356884791277"),
        Unsigned("18014398777917439"),
        Unsigned("18446744082299486207"),
        Unsigned("523347633027360537213687137"),
        Unsigned("43143988327398957279342419750374600193"),
        Unsigned("162259276829213363391578010288127"),
        Unsigned("1469367938527859384960920671527807097273331945965109401885939"
                 "6328480215743184089660644531")};
    Unsigned u = 1;
    Unsigned v = 1;
    for (size_t i = 0; i < 8; ++i) {
        u *= primes[i];
    }
    for (size_t i = 6; i < 12; ++i) {
        v *= primes[i];
    }
    Unsigned exp = primes[6] * primes[7];

    EXPECT_EQ(exp, egcd(u, v));
    EXPECT_EQ(exp, egcd(v, u));
    EXPECT_EQ(exp, bgcd(u, v));
    EXPECT_EQ(exp, bgcd(v, u));
    EXPECT_EQ(exp, gcd(u, v));
}
//------------------------------------------------------------------------------
TEST(UnsignedTest, operatorOut)
{
    Unsigned u("123456789012345678901234567890");
    ostringstream os;
    os << u;
    EXPECT_EQ(u.str(), os.str());
}
