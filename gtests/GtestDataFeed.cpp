#include <string>

#include "Poco/Delegate.h"
#include "Poco/Data/SQLite/Connector.h"
#include "gtest/gtest.h"

#include "tradelib/PinnacleDataFeed.h"

using namespace tradelib;

TEST(PinnacleDataFeed, Instruments)
{
   DataFeed * df = new PinnacleDataFeed;
   df->configure("pinnacle.sqlite");

   const Instrument * future = df->getInstrument("ES");
   ASSERT_EQ(future->bpv(), 50.0);
   ASSERT_EQ(future->tick(), 0.25);
   ASSERT_EQ(future->symbol(), "ES");
   ASSERT_TRUE(future->isFuture());
   ASSERT_FALSE(future->isStock());
   ASSERT_EQ(future->name(), "E-mini S&P 500");

   future = df->getInstrument("ZW");
   ASSERT_EQ(future->bpv(), 50.0);
   ASSERT_EQ(future->tick(), 0.25);
   ASSERT_EQ(future->symbol(), "ZW");
   ASSERT_TRUE(future->isFuture());
   ASSERT_FALSE(future->isStock());
   ASSERT_EQ(future->name(), "Wheat");
}

class BarLoader
{
public:
   std::vector<Bar> bars;
   void onBar(const void * sender, const Bar & bar)
   {
      bars.push_back(bar);
   }
};

TEST(PinnacleDataFeed, BarProcessing)
{
   DataFeed * df = new PinnacleDataFeed;
   df->configure("pinnacle.sqlite");

   // subscribe
   df->subscribe("YM");
   df->subscribe("JN");
   df->subscribe("ZO");

   // setup the observer
   BarLoader bl;
   df->barEvent += Poco::delegate(&bl, &BarLoader::onBar);

   // process the feed
   df->start();

   // Verify the bar counts for each future
   sint ymCounter = 0;
   sint jnCounter = 0;
   sint zoCounter = 0;
   for (auto & bar : bl.bars)
   {
      if (bar.symbol == "YM") ++ymCounter;
      if (bar.symbol == "JN") ++jnCounter;
      if (bar.symbol == "ZO") ++zoCounter;
   }

   ASSERT_EQ(ymCounter, 3111);
   ASSERT_EQ(jnCounter, 9220);
   ASSERT_EQ(zoCounter, 9975);

   // Verify the bar order
   for (sint ii = 1; ii < bl.bars.size(); ++ii)
   {
      ASSERT_LE(bl.bars[ii - 1].timestamp, bl.bars[ii].timestamp);
   }
}