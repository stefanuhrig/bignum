FIND_PACKAGE(GTest REQUIRED)
INCLUDE(GoogleTest)

SET(bignumtest_sources
    StoreTest.cpp
    UnsignedTest.cpp
    SignedTest.cpp
    RationalTest.cpp
)
ADD_EXECUTABLE(bignumtest ${bignumtest_sources})
TARGET_INCLUDE_DIRECTORIES(bignumtest PRIVATE ${PROJECT_SOURCE_DIR}/include)
TARGET_COMPILE_DEFINITIONS(bignumtest
    PRIVATE DIGIT_T=std::uint8_t
    PRIVATE DDIGIT_T=std::uint16_t
)
TARGET_LINK_LIBRARIES(bignumtest GTest::Main)
GTEST_DISCOVER_TESTS(bignumtest)
ADD_TEST(NAME bignumtest COMMAND bignumtest)
