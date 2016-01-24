#include <string>

#include "gtest/gtest.h"

#include "tradelib/Math.h"

TEST(Math, Sign)
{
   ASSERT_EQ(tradelib::sign(10), 1);
   ASSERT_EQ(tradelib::sign(-10), -1);
   ASSERT_EQ(tradelib::sign(0), 0);
   ASSERT_EQ(tradelib::sign(std::numeric_limits<int>::max()), 1);
   ASSERT_EQ(tradelib::sign(std::numeric_limits<int>::lowest()), -1);
   ASSERT_EQ(tradelib::sign(std::numeric_limits<uint64_t>::max()), 1);
   ASSERT_EQ(tradelib::sign(std::numeric_limits<uint64_t>::lowest()), 0);
   ASSERT_EQ(tradelib::sign(std::numeric_limits<int64_t>::max()), 1);
   ASSERT_EQ(tradelib::sign(std::numeric_limits<int64_t>::lowest()), -1);
   ASSERT_EQ(tradelib::sign(std::numeric_limits<double>::max()), 1);
   ASSERT_EQ(tradelib::sign(std::numeric_limits<double>::lowest()), -1);
   ASSERT_EQ(tradelib::sign(0.0L), 0);
}

TEST(Math, RoundAny)
{
   ASSERT_DOUBLE_EQ(tradelib::roundAny(10.1234, 1.0), 10.0);
   ASSERT_DOUBLE_EQ(tradelib::roundAny(10.1234, 0.1), 10.1);
   ASSERT_DOUBLE_EQ(tradelib::roundAny(10.1234, 0.01), 10.12);
   ASSERT_DOUBLE_EQ(tradelib::roundAny(10.1234, 0.001), 10.123);
}