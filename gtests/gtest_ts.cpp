#include "ts.hpp"
#include "gtest/gtest.h"

TEST(ts, general)
{
   tl::ts es;
   es.from_csv("es.csv");
   ASSERT_GT(es.size(), 0);
   ASSERT_DOUBLE_EQ(es[0].back(), 1921.50);
   ASSERT_DOUBLE_EQ(es[1].back(), 1923.25);
   ASSERT_DOUBLE_EQ(es[2].back(), 1916.00);
   ASSERT_DOUBLE_EQ(es[3].back(), 1922);

   es.tail(100);
   ASSERT_DOUBLE_EQ(es[3][0], 1826);

   es.tail(-1);
   ASSERT_DOUBLE_EQ(es[3][0], 1830.75);

   es.head(90);
   ASSERT_DOUBLE_EQ(es[3].back(), 1868.0);

   es.head(-2);
   ASSERT_DOUBLE_EQ(es[3].back(), 1874.75);
}