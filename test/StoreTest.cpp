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
#include <utility>
//------------------------------------------------------------------------------
TEST(StoreTest, defaultConstructor)
{
    bn::impl::Store store;
    EXPECT_EQ(0U, store.size());
}
//------------------------------------------------------------------------------
TEST(StoreTest, copyConstructor)
{
    bn::impl::Store store;
    store.resize(2);
    store[0] = 1;
    store[1] = 2;
    bn::impl::Store storeCopy(store);
    ASSERT_EQ(store.size(), storeCopy.size());
    for (std::size_t i = 0; i < store.size(); ++i) {
        EXPECT_EQ(store[i], storeCopy[i]);
    }
}
//------------------------------------------------------------------------------
TEST(StoreTest, copyConstructorLarge)
{
    bn::impl::Store store;
    store.resize(bn::impl::Store::smemsize + 1);
    for (std::size_t i = 0; i < store.size(); ++i) {
        store[i] = static_cast<bn::impl::digit_t>(i);
    }
    bn::impl::Store storeCopy(store);
    ASSERT_EQ(store.size(), storeCopy.size());
    for (std::size_t i = 0; i < store.size(); ++i) {
        EXPECT_EQ(store[i], storeCopy[i]);
    }
}
//------------------------------------------------------------------------------
TEST(StoreTest, moveConstructor)
{
    bn::impl::Store store;
    store.resize(2);
    store[0] = 1;
    store[1] = 2;
    bn::impl::Store movedStore(std::move(store));
    ASSERT_EQ(2u, movedStore.size());
    EXPECT_EQ(1u, movedStore[0]);
    EXPECT_EQ(2u, movedStore[1]);
}
//------------------------------------------------------------------------------
TEST(StoreTest, moveConstructorLarge)
{
    bn::impl::Store store;
    store.resize(bn::impl::Store::smemsize + 1);
    for (std::size_t i = 0; i < store.size(); ++i) {
        store[i] = static_cast<bn::impl::digit_t>(i);
    }
    bn::impl::Store movedStore(std::move(store));
    ASSERT_EQ(bn::impl::Store::smemsize + 1, movedStore.size());
    for (std::size_t i = 0; i < movedStore.size(); ++i) {
        EXPECT_EQ(static_cast<bn::impl::digit_t>(i), movedStore[i]);
    }
}
//------------------------------------------------------------------------------
TEST(StoreTest, copyAssignment)
{
    bn::impl::Store store;
    store.resize(2);
    store[0] = 1;
    store[1] = 2;
    bn::impl::Store storeCopy;
    storeCopy = store;
    ASSERT_EQ(store.size(), storeCopy.size());
    for (std::size_t i = 0; i < store.size(); ++i) {
        EXPECT_EQ(store[i], storeCopy[i]);
    }
}
//------------------------------------------------------------------------------
TEST(StoreTest, copyAssignmentSMemReuseHeap)
{
    bn::impl::Store source;
    source.resize(2);
    source[0] = 0;
    source[1] = 1;

    bn::impl::Store target;
    target.resize(bn::impl::Store::smemsize + 1);
    target = source;
    ASSERT_EQ(2u, target.size());
    EXPECT_EQ(0u, target[0]);
    EXPECT_EQ(1u, target[1]);
}
//------------------------------------------------------------------------------
TEST(StoreTest, copyAssignmentSMemFreeHeap)
{
    bn::impl::Store source;
    source.resize(2);
    source[0] = 0;
    source[1] = 1;

    bn::impl::Store target;
    target.resize(3 * bn::impl::Store::smemsize);
    target = source;
    ASSERT_EQ(2u, target.size());
    EXPECT_EQ(0u, target[0]);
    EXPECT_EQ(1u, target[1]);
}
//------------------------------------------------------------------------------
TEST(StoreTest, copyAssignmentHeapReuseHeap)
{
    bn::impl::Store source;
    source.resize(bn::impl::Store::smemsize + 1);
    for (std::size_t i = 0; i < source.size(); ++i) {
        source[i] = static_cast<bn::impl::digit_t>(i);
    }

    bn::impl::Store target;
    target.resize(bn::impl::Store::smemsize + 3);
    target = source;
    ASSERT_EQ(source.size(), target.size());
    for (std::size_t i = 0; i < source.size(); ++i) {
        EXPECT_EQ(source[i], target[i]);
    }
}
//------------------------------------------------------------------------------
TEST(StoreTest, copyAssignmentHeapReallocateHeap)
{
    bn::impl::Store source;
    source.resize(bn::impl::Store::smemsize + 1);
    for (std::size_t i = 0; i < source.size(); ++i) {
        source[i] = static_cast<bn::impl::digit_t>(i);
    }

    bn::impl::Store target;
    target.resize(3 * source.size());
    target = source;
    ASSERT_EQ(source.size(), target.size());
    for (std::size_t i = 0; i < source.size(); ++i) {
        EXPECT_EQ(source[i], target[i]);
    }
}
//------------------------------------------------------------------------------
TEST(StoreTest, copyAssignmentHeapFromSMem)
{
    bn::impl::Store source;
    source.resize(bn::impl::Store::smemsize + 1);
    for (std::size_t i = 0; i < source.size(); ++i) {
        source[i] = static_cast<bn::impl::digit_t>(i);
    }

    bn::impl::Store target;
    target = source;
    ASSERT_EQ(source.size(), target.size());
    for (std::size_t i = 0; i < source.size(); ++i) {
        EXPECT_EQ(source[i], target[i]);
    }
}
//------------------------------------------------------------------------------
TEST(StoreTest, copyAssignmentHeapGrow)
{
    bn::impl::Store source;
    source.resize(2 * bn::impl::Store::smemsize);
    for (std::size_t i = 0; i < source.size(); ++i) {
        source[i] = static_cast<bn::impl::digit_t>(i);
    }

    bn::impl::Store target;
    target.resize(bn::impl::Store::smemsize + 1);
    target = source;
    ASSERT_EQ(source.size(), target.size());
    for (std::size_t i = 0; i < source.size(); ++i) {
        EXPECT_EQ(source[i], target[i]);
    }
}
//------------------------------------------------------------------------------
TEST(StoreTest, moveAssignmentSMemSMem)
{
    bn::impl::Store store;
    store.resize(2);
    store[0] = 1;
    store[1] = 2;
    bn::impl::Store movedStore;
    movedStore = std::move(store);
    ASSERT_EQ(2u, movedStore.size());
    EXPECT_EQ(1u, movedStore[0]);
    EXPECT_EQ(2u, movedStore[1]);
}
//------------------------------------------------------------------------------
TEST(StoreTest, moveAssignmentSMemMoveHeap)
{
    bn::impl::Store store;
    store.resize(2);
    store[0] = 1;
    store[1] = 2;
    bn::impl::Store movedStore;
    movedStore.resize(bn::impl::Store::smemsize + 1);
    movedStore = std::move(store);
    ASSERT_EQ(2u, movedStore.size());
    EXPECT_EQ(1u, movedStore[0]);
    EXPECT_EQ(2u, movedStore[1]);
}
//------------------------------------------------------------------------------
TEST(StoreTest, moveAssignmentHeapMoveSmem)
{
    bn::impl::Store store;
    store.resize(bn::impl::Store::smemsize + 1);
    for (std::size_t i = 0; i < store.size(); ++i) {
        store[i] = static_cast<bn::impl::digit_t>(i);
    }
    bn::impl::Store movedStore;
    movedStore.resize(2);
    movedStore = std::move(store);

    ASSERT_EQ(bn::impl::Store::smemsize + 1, movedStore.size());
    for (std::size_t i = 0; i < movedStore.size(); ++i) {
        EXPECT_EQ(static_cast<bn::impl::digit_t>(i), movedStore[i]);
    }
}
//------------------------------------------------------------------------------
TEST(StoreTest, moveAssignmentHeapMoveHeap)
{
    bn::impl::Store store;
    store.resize(bn::impl::Store::smemsize + 1);
    for (std::size_t i = 0; i < store.size(); ++i) {
        store[i] = static_cast<bn::impl::digit_t>(i);
    }
    bn::impl::Store movedStore;
    movedStore.resize(2 * bn::impl::Store::smemsize);
    movedStore = std::move(store);

    ASSERT_EQ(bn::impl::Store::smemsize + 1, movedStore.size());
    for (std::size_t i = 0; i < movedStore.size(); ++i) {
        EXPECT_EQ(static_cast<bn::impl::digit_t>(i), movedStore[i]);
    }
}
//------------------------------------------------------------------------------
TEST(StoreTest, indexOperator)
{
    bn::impl::Store store;
    store.resize(2);
    store[0] = 1;
    store[1] = 2;
    const bn::impl::Store& roStore = store;
    for (std::size_t i = 0; i < store.size(); ++i) {
        EXPECT_EQ(store[i], roStore[i]);
    }
}
//------------------------------------------------------------------------------
TEST(StoreTest, resize)
{
    bn::impl::Store store;
    store.resize(2);
    store[0] = 1;
    store[1] = 2;
    store.resize(1);
    ASSERT_EQ(1u, store.size());
    EXPECT_EQ(1u, store[0]);

    EXPECT_THROW(
        store.resize(std::numeric_limits<size_t>::max()), std::bad_alloc);
}
//------------------------------------------------------------------------------
TEST(StoreTest, resizeShrinkToSMem)
{
    bn::impl::Store store;
    store.resize(3 * bn::impl::Store::smemsize);
    for (std::size_t i = 0; i < bn::impl::Store::smemsize; ++i) {
        store[i] = static_cast<bn::impl::digit_t>(i);
    }
    store.resize(bn::impl::Store::smemsize);
    const std::size_t smemsize = bn::impl::Store::smemsize;
    ASSERT_EQ(smemsize, store.size());
    for (std::size_t i = 0; i < store.size(); ++i) {
        EXPECT_EQ(static_cast<bn::impl::digit_t>(i), store[i]);
    }
}
//------------------------------------------------------------------------------
TEST(StoreTest, resizeShrinkNoRealloc)
{
    bn::impl::Store store;
    store.resize(3 * bn::impl::Store::smemsize);
    for (std::size_t i = 0; i < store.size(); ++i) {
        store[i] = static_cast<bn::impl::digit_t>(i);
    }
    store.resize(2 * bn::impl::Store::smemsize);
    ASSERT_EQ(2 * bn::impl::Store::smemsize, store.size());
    for (std::size_t i = 0; i < store.size(); ++i) {
        EXPECT_EQ(static_cast<bn::impl::digit_t>(i), store[i]);
    }
}
//------------------------------------------------------------------------------
TEST(StoreTest, resizeShrinkAlloc)
{
    bn::impl::Store store;
    store.resize(6 * bn::impl::Store::smemsize);
    for (std::size_t i = 0; i < store.size(); ++i) {
        store[i] = static_cast<bn::impl::digit_t>(i);
    }
    store.resize(bn::impl::Store::smemsize + 1);
    ASSERT_EQ(bn::impl::Store::smemsize + 1, store.size());
    for (std::size_t i = 0; i < store.size(); ++i) {
        EXPECT_EQ(static_cast<bn::impl::digit_t>(i), store[i]);
    }
}
//------------------------------------------------------------------------------
TEST(StoreTest, resizeShrinkRealloc)
{
    bn::impl::Store store;
    store.resize(bn::impl::Store::smemsize + 1);
    for (std::size_t i = 0; i < store.size(); ++i) {
        store[i] = static_cast<bn::impl::digit_t>(i);
    }
    store.resize(bn::impl::Store::smemsize + 2);
    ASSERT_EQ(bn::impl::Store::smemsize + 2, store.size());
    for (std::size_t i = 0; i < store.size() - 1; ++i) {
        EXPECT_EQ(static_cast<bn::impl::digit_t>(i), store[i]);
    }
}
