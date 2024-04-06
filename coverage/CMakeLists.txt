FIND_PACKAGE(GTest REQUIRED)
INCLUDE(GoogleTest)

SET(bignumcoverage_sources
    ${PROJECT_SOURCE_DIR}/test/StoreTest.cpp
    ${PROJECT_SOURCE_DIR}/test/UnsignedTest.cpp
    ${PROJECT_SOURCE_DIR}/test/SignedTest.cpp
    ${PROJECT_SOURCE_DIR}/test/RationalTest.cpp
)
ADD_EXECUTABLE(bignumcoverage ${bignumcoverage_sources})
TARGET_INCLUDE_DIRECTORIES(bignumcoverage PRIVATE ${PROJECT_SOURCE_DIR}/include)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g --coverage -fprofile-abs-path")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g --coverage -fprofile-abs-path")
TARGET_COMPILE_DEFINITIONS(bignumcoverage
    PRIVATE DIGIT_T=std::uint8_t
    PRIVATE DDIGIT_T=std::uint16_t
)
TARGET_LINK_LIBRARIES(bignumcoverage GTest::Main)
GTEST_DISCOVER_TESTS(bignumcoverage)
ADD_TEST(NAME bignumcoverage COMMAND bignumcoverage)

