#include <string>

#include "gtest/gtest.h"
#include "Poco/Delegate.h"

#include "tradelib/Types.h"

using namespace tradelib;

class Pair
{
public:
   Pair(sint ii, sint jj) : i(ii), j(jj) {}

   sint i;
   sint j;
};

// Rolling Sum of N elements
template<sint N>
class RollingSum : public tradelib::RVector<numeric>
{
public:
   void onValue(const void * sender, const sint & value)
   {
      const tradelib::RVector<sint> * rv = reinterpret_cast<const tradelib::RVector<sint> *>(sender);

      sum_ += value;
      if (barId_ >= (N - 1))
      {
         if (barId_ >= N) sum_ -= rv->at(N);
         sums.push_back(sum_);
      }
      else
      {
         sums.push_back(NAN);
      }

      ++barId_;
   }

   tradelib::RVector<numeric> sums;

protected:
   sint barId_ = 0;
   numeric sum_ = 0.0;
};

TEST(Types, RVector)
{
   tradelib::RVector<sint> v;
   tradelib::RVector<Pair> w;

   RollingSum<3> rs;
   v.valueEvent += Poco::delegate(&rs, &RollingSum<3>::onValue);

   for (sint ii = 0; ii < 10; ++ii)
   {
      v.push_back(ii);
      w.emplace_back(ii, ii);
   }

   v.push_back(10);
   w.emplace_back(10, 10);

   for (sint ii = 0; ii < v.size(); ++ii)
   {
      ASSERT_EQ(v[ii], v.size() - 1 - ii);
      ASSERT_EQ(v.at(ii), v.size() - 1 - ii);

      ASSERT_EQ(v[ii], w[ii].i);
      ASSERT_EQ(v[ii], w[ii].j);
      ASSERT_EQ(v[ii], w.at(ii).j);
   }

   ASSERT_TRUE(isnan<numeric>(rs.sums[rs.sums.size() - 1]) && isnan<numeric>(rs.sums[rs.sums.size() - 2]));
   for (sint ii = 0; ii < rs.sums.size() - 2; ++ii)
   {
      ASSERT_EQ(rs.sums[ii], v[ii] + v[ii + 1] + v[ii + 2]);
   }
}