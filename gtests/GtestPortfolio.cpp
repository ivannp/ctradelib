// std headers
#include <limits>
#include <stdio.h>

// libraries headers
#include "gtest/gtest.h"
#include "Poco/DateTime.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Timespan.h"
#include "Poco/Timestamp.h"

// tradelib headers
#include "tradelib/Instrument.h"
#include "tradelib/Portfolio.h"
#include "tradelib/Types.h"

using namespace tradelib;

TEST(Portfolio, General)
{
   Instrument es = Instrument::newFuture("ES", 0.25, 50.0);
   Portfolio pp("default");
   pp.addInstrument(es);

   // Transactions verified using R's blotter package or manually

   pp.appendTransaction(es, Poco::DateTime(2014, 1, 2, 17).timestamp(), 1, 1819.50, 0.0);

   ASSERT_EQ(pp.quantity("ES"), 1);
   ASSERT_DOUBLE_EQ(pp.price("ES"), 1819.50);
   ASSERT_DOUBLE_EQ(pp.value("ES"), 90975.00);
   ASSERT_DOUBLE_EQ(pp.averageCost("ES"), 1819.50);
   ASSERT_EQ(pp.positionQuantity("ES"), 1);
   ASSERT_DOUBLE_EQ(pp.positionAverageCost("ES"), 1819.50);
   ASSERT_DOUBLE_EQ(pp.grossPnl("ES"), 0.0);
   ASSERT_DOUBLE_EQ(pp.netPnl("ES"), 0.0);
   ASSERT_DOUBLE_EQ(pp.fees("ES"), 0.0);

   numeric realizedPnl;
   numeric unrealizedPnl;

   pp.getPositionPnl(es, 1825.5, realizedPnl, unrealizedPnl);

   ASSERT_DOUBLE_EQ(realizedPnl, 0.0);
   ASSERT_DOUBLE_EQ(unrealizedPnl, 300.0);

   pp.appendTransaction(es, Poco::DateTime(2014, 1, 9, 17).timestamp(), 2, 1826, 0.0);

   ASSERT_EQ(pp.quantity("ES"), 2);
   ASSERT_DOUBLE_EQ(pp.price("ES"), 1826);
   ASSERT_DOUBLE_EQ(pp.value("ES"), 182600);
   ASSERT_DOUBLE_EQ(pp.averageCost("ES"), 1826.00);
   ASSERT_EQ(pp.positionQuantity("ES"), 3);
   ASSERT_DOUBLE_EQ(roundAny(pp.positionAverageCost("ES"), 0.0001), 1823.8333);
   ASSERT_DOUBLE_EQ(pp.grossPnl("ES"), 0.0);
   ASSERT_DOUBLE_EQ(pp.netPnl("ES"), 0.0);
   ASSERT_DOUBLE_EQ(pp.fees("ES"), 0.0);

   pp.getPositionPnl(es, 1826, realizedPnl, unrealizedPnl);

   ASSERT_DOUBLE_EQ(realizedPnl, 0.0);
   ASSERT_NEAR(unrealizedPnl, 325.0, 1e-10);

   pp.getPositionPnl(es, 1829.25, realizedPnl, unrealizedPnl);

   // next transaction sells everything for a pnl of 812.5, that's our unrealized pnl at the moment
   ASSERT_DOUBLE_EQ(realizedPnl, 0.0);
   ASSERT_NEAR(unrealizedPnl, 812.5, 0.000000001);

   pp.appendTransaction(es, Poco::DateTime(2014, 1, 16, 17).timestamp(), -3, 1829.25, 0.0);

   ASSERT_EQ(pp.quantity("ES"), -3);
   ASSERT_DOUBLE_EQ(pp.price("ES"), 1829.25);
   ASSERT_DOUBLE_EQ(pp.value("ES"), -274387.5);
   ASSERT_DOUBLE_EQ(pp.averageCost("ES"), 1829.25);
   ASSERT_EQ(pp.positionQuantity("ES"), 0);
   ASSERT_DOUBLE_EQ(pp.positionAverageCost("ES"), 0.0);
   ASSERT_NEAR(pp.grossPnl("ES"), 812.5, 1e-10);
   ASSERT_NEAR(pp.netPnl("ES"), 812.5, 1e-10);
   ASSERT_DOUBLE_EQ(pp.fees("ES"), 0.0);

   pp.appendTransaction(es, Poco::DateTime(2014, 1, 24, 17).timestamp(), -2, 1775.00, 0.0);

   ASSERT_EQ(pp.quantity("ES"), -2);
   ASSERT_DOUBLE_EQ(pp.price("ES"), 1775.00);
   ASSERT_DOUBLE_EQ(pp.value("ES"), -177500);
   ASSERT_DOUBLE_EQ(pp.averageCost("ES"), 1775.00);
   ASSERT_EQ(pp.positionQuantity("ES"), -2);
   ASSERT_DOUBLE_EQ(pp.positionAverageCost("ES"), 1775.0000);
   ASSERT_DOUBLE_EQ(pp.grossPnl("ES"), 0.0);
   ASSERT_DOUBLE_EQ(pp.netPnl("ES"), 0.0);
   ASSERT_DOUBLE_EQ(pp.fees("ES"), 0.0);

   // a transaction which is split
   pp.appendTransaction(es, Poco::DateTime(2014, 1, 31, 17).timestamp(), 3, 1769.50, 0.0);

   ASSERT_EQ(pp.quantity("ES"), 1);
   ASSERT_DOUBLE_EQ(pp.price("ES"), 1769.50);
   ASSERT_DOUBLE_EQ(pp.value("ES"), 88475.0);
   ASSERT_DOUBLE_EQ(pp.averageCost("ES"), 1769.50);
   ASSERT_EQ(pp.positionQuantity("ES"), 1);
   ASSERT_DOUBLE_EQ(pp.positionAverageCost("ES"), 1769.50);
   ASSERT_DOUBLE_EQ(pp.grossPnl("ES"), 0.0);
   ASSERT_DOUBLE_EQ(pp.netPnl("ES"), 0.0);
   ASSERT_DOUBLE_EQ(pp.fees("ES"), 0.0);

   pp.appendTransaction(es, Poco::DateTime(2014, 2, 7, 17).timestamp(), -2, 1786.50, 0.0);

   ASSERT_EQ(pp.quantity("ES"), -1);
   ASSERT_DOUBLE_EQ(pp.price("ES"), 1786.50);
   ASSERT_DOUBLE_EQ(pp.value("ES"), -89325.0);
   ASSERT_DOUBLE_EQ(pp.averageCost("ES"), 1786.50);
   ASSERT_EQ(pp.positionQuantity("ES"), -1);
   ASSERT_DOUBLE_EQ(pp.positionAverageCost("ES"), 1786.50);
   ASSERT_DOUBLE_EQ(pp.grossPnl("ES"), 0.0);
   ASSERT_DOUBLE_EQ(pp.netPnl("ES"), 0.0);
   ASSERT_DOUBLE_EQ(pp.fees("ES"), 0.0);

   pp.appendTransaction(es, Poco::DateTime(2014, 2, 14, 17).timestamp(), 1, 1828.00, 0.0);

   ASSERT_EQ(pp.quantity("ES"), 1);
   ASSERT_DOUBLE_EQ(pp.price("ES"), 1828.00);
   ASSERT_DOUBLE_EQ(pp.value("ES"), 91400.0);
   ASSERT_DOUBLE_EQ(pp.averageCost("ES"), 1828.00);
   ASSERT_EQ(pp.positionQuantity("ES"), 0);
   ASSERT_DOUBLE_EQ(pp.positionAverageCost("ES"), 0.0);
   ASSERT_NEAR(pp.grossPnl("ES"), -2075.0, 1e-10);
   ASSERT_NEAR(pp.netPnl("ES"), -2075.0, 1e-10);
   ASSERT_DOUBLE_EQ(pp.fees("ES"), 0.0);

   /*
   ts es_ts;
   es_ts.from_csv("es.csv");
   es_ts.tail(200);
   time_vector pnl_time;
   data_vector pnl;
   pp.pnl(es, es_ts.time(), es_ts[3], pnl_time, pnl);
   ASSERT_EQ(pnl_time.size(), pnl.size());
   tl::ts pnl_ts(pnl_time, pnl);

   std::vector<numeric> expected(pnl_ts.size());
   expected[96] = -50.0;
   expected[97] = -237.5;
   expected[98] = 500.0;
   expected[99] = 87.5;
   expected[100] = 25.0;
   expected[101] = 712.5;
   expected[102] = -3412.5;
   expected[103] = 2700.0;
   expected[104] = 1275.0;
   expected[105] = -787.5;

   expected[111] = 625;
   expected[112] = -1250.0;
   expected[113] = 1700.0;
   expected[114] = -1000;
   expected[115] = 475.0;
   expected[116] = -2187.5;
   expected[117] = 550.0;
   expected[118] = 12.5;
   expected[119] = 1125;
   expected[120] = 1350;
   expected[121] = -62.5;
   expected[122] = -937.5;
   expected[123] = -175.0;
   expected[124] = -362.5;
   expected[125] = -537.5;

   for (sint ii = 0; ii < pnl.size(); ++ii)
   {
      if (pnl[ii] != 0.0) std::cout << "[" << ii << "]: " << pnl_time[ii] << ": " << pnl[ii] << std::endl;
   }

   for (sint ii = 0; ii < pnl_ts.size(); ++ii)
   {
      ASSERT_DOUBLE_EQ(expected[ii], pnl_ts[0][ii]);
   }

   pnl_ts.aggregate([](tl::time t) { Poco::DateTime dt(t); return Poco::DateTime(dt.year(), dt.month(), dt.day()).timestamp(); });

   for (sint ii = 0; ii < pnl_ts.size(); ++ii)
   {
      if (pnl_ts[0][ii] != 0.0) std::cout << "[" << ii << "]: " << pnl_time[ii] << ": " << pnl_ts[0][ii] << std::endl;
   }

   for (sint ii = 0; ii < pnl_ts.size(); ++ii)
   {
      ASSERT_DOUBLE_EQ(expected[ii], pnl_ts[0][ii]);
   }
   */
}

TEST(Portfolio, TradeStats)
{
   Instrument es = Instrument::newFuture("ES", 0.25, 50.0);
   Portfolio pp("default");
   pp.addInstrument(es);

   // Transactions and trade stats verified using R's blotter package and/or manually

   pp.appendTransaction(es, Poco::DateTime(2014, 1, 2, 17).timestamp(), 1, 1819.50, -2.02);

   ASSERT_EQ(pp.quantity("ES"), 1);
   ASSERT_DOUBLE_EQ(pp.price("ES"), 1819.50);
   ASSERT_DOUBLE_EQ(pp.value("ES"), 90975.00);
   ASSERT_DOUBLE_EQ(pp.averageCost("ES"), 1819.50);
   ASSERT_EQ(pp.positionQuantity("ES"), 1);
   ASSERT_DOUBLE_EQ(pp.positionAverageCost("ES"), 1819.50);
   ASSERT_DOUBLE_EQ(pp.grossPnl("ES"), 0.0);
   ASSERT_DOUBLE_EQ(pp.netPnl("ES"), -2.02);
   ASSERT_DOUBLE_EQ(pp.fees("ES"), -2.02);

   pp.appendTransaction(es, Poco::DateTime(2014, 1, 7, 17).timestamp(), -2, 1816.50, -2.03);

   ASSERT_EQ(pp.quantity("ES"), -1);
   ASSERT_DOUBLE_EQ(pp.price("ES"), 1816.50);
   ASSERT_DOUBLE_EQ(pp.value("ES"), -90825.00);
   ASSERT_DOUBLE_EQ(pp.averageCost("ES"), 1816.50);
   ASSERT_EQ(pp.positionQuantity("ES"), -1);
   ASSERT_DOUBLE_EQ(pp.positionAverageCost("ES"), 1816.50);
   ASSERT_DOUBLE_EQ(pp.grossPnl("ES"), 0.0);
   ASSERT_DOUBLE_EQ(pp.netPnl("ES"), -1.015);
   ASSERT_DOUBLE_EQ(pp.fees("ES"), -1.015);

   pp.appendTransaction(es, Poco::DateTime(2014, 1, 23, 17).timestamp(), 2, 1810.00, -2.04);
   pp.appendTransaction(es, Poco::DateTime(2014, 2, 10, 17).timestamp(), -2, 1780.50, -2.05);
   pp.appendTransaction(es, Poco::DateTime(2014, 2, 27, 17).timestamp(), 3, 1839.75, -2.06);
   pp.appendTransaction(es, Poco::DateTime(2014, 3, 3, 17).timestamp(), -4, 1828.75, -2.07);
   pp.appendTransaction(es, Poco::DateTime(2014, 3, 14, 17).timestamp(), 3, 1825.75, -2.08);
   pp.appendTransaction(es, Poco::DateTime(2014, 3, 20, 17).timestamp(), -2, 1858.75, -2.09);
   pp.appendTransaction(es, Poco::DateTime(2014, 3, 24, 17).timestamp(), 2, 1842.25, -2.08);
   pp.appendTransaction(es, Poco::DateTime(2014, 4, 8, 17).timestamp(), -2, 1837.75, -2.07);
   pp.appendTransaction(es, Poco::DateTime(2014, 4, 25, 17).timestamp(), 2, 1852.75, -2.06);
   pp.appendTransaction(es, Poco::DateTime(2014, 5, 14, 17).timestamp(), -2, 1878, -2.05);
   pp.appendTransaction(es, Poco::DateTime(2014, 5, 28, 17).timestamp(), 2, 1901.75, -2.04);
   pp.appendTransaction(es, Poco::DateTime(2014, 6, 16, 17).timestamp(), -2, 1929.25, -2.03);
   pp.appendTransaction(es, Poco::DateTime(2014, 7, 1, 17).timestamp(), 2, 1965.75, -2.02);
   pp.appendTransaction(es, Poco::DateTime(2014, 7, 16, 17).timestamp(), -2, 1974.75, -2.01);
   pp.appendTransaction(es, Poco::DateTime(2014, 7, 24, 17).timestamp(), 2, 1980.75, -2.02);
   pp.appendTransaction(es, Poco::DateTime(2014, 7, 25, 17).timestamp(), -2, 1971.50, -2.03);
   pp.appendTransaction(es, Poco::DateTime(2014, 8, 1, 17).timestamp(), 1, 1918.50, -2.04);

   TradeStatsVector tradeStats;
   pp.getTradeStats(es, tradeStats);

   /*
   for (const auto & ts : tradeStats)
   {
      std::cout << ts << std::endl;
   }
   */

   // Verify the trades
   TradeStatsVector::const_iterator it = std::begin(tradeStats);
   ASSERT_EQ(it->start, Poco::DateTime(2014, 1, 2, 17).timestamp());
   ASSERT_EQ(it->end, Poco::DateTime(2014, 1, 7, 17).timestamp());
   ASSERT_EQ(it->initialPosition, 1);
   ASSERT_EQ(it->maxPosition, 1);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, 90975.0);
   ASSERT_DOUBLE_EQ(it->pnl, -150.0);
   ASSERT_NEAR(it->pctPnl, -0.001648805, 0.00000001);

   ++it;
   ASSERT_EQ(it->start, Poco::DateTime(2014, 1, 7, 17).timestamp() + 1);
   ASSERT_EQ(it->end, Poco::DateTime(2014, 1, 23, 17).timestamp());
   ASSERT_EQ(it->initialPosition, -1);
   ASSERT_EQ(it->maxPosition, -1);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, -90825.0);
   ASSERT_DOUBLE_EQ(it->pnl, 325);
   ASSERT_NEAR(it->pctPnl, 0.003578310, 0.00000001);

   ++it;
   ASSERT_EQ(it->start, Poco::DateTime(2014, 1, 23, 17).timestamp() + 1);
   ASSERT_EQ(it->end, Poco::DateTime(2014, 2, 10, 17).timestamp());
   ASSERT_EQ(it->initialPosition, 1);
   ASSERT_EQ(it->maxPosition, 1);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, 90500.0);
   ASSERT_DOUBLE_EQ(it->pnl, -1475.0);
   ASSERT_NEAR(it->pctPnl, -0.016298343, 0.00000001);

   ++it;
   ASSERT_EQ(it->start, Poco::DateTime(2014, 2, 10, 17).timestamp() + 1);
   ASSERT_EQ(it->end, Poco::DateTime(2014, 2, 27, 17).timestamp());
   ASSERT_EQ(it->initialPosition, -1);
   ASSERT_EQ(it->maxPosition, -1);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, -89025.0);
   ASSERT_DOUBLE_EQ(it->pnl, -2962.5);
   ASSERT_NEAR(it->pctPnl, -0.033277169, 0.00000001);

   ++it;
   ASSERT_EQ(it->start, Poco::DateTime(2014, 2, 27, 17).timestamp() + 1);
   ASSERT_EQ(it->end, Poco::DateTime(2014, 3, 3, 17).timestamp());
   ASSERT_EQ(it->initialPosition, 2);
   ASSERT_EQ(it->maxPosition, 2);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, 183975.0);
   ASSERT_DOUBLE_EQ(it->pnl, -1100.0);
   ASSERT_NEAR(it->pctPnl, -0.005979073, 0.00000001);

   ++it;
   ASSERT_EQ(it->start, Poco::DateTime(2014, 3, 3, 17).timestamp() + 1);
   ASSERT_EQ(it->end, Poco::DateTime(2014, 3, 14, 17).timestamp());
   ASSERT_EQ(it->initialPosition, -2);
   ASSERT_EQ(it->maxPosition, -2);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, -182875.0);
   ASSERT_DOUBLE_EQ(it->pnl, 300.0);
   ASSERT_NEAR(it->pctPnl, 0.001640465, 0.00000001);

   ++it;
   ASSERT_EQ(it->start, Poco::DateTime(2014, 3, 14, 17).timestamp() + 1);
   ASSERT_EQ(it->end, Poco::DateTime(2014, 3, 20, 17).timestamp());
   ASSERT_EQ(it->initialPosition, 1);
   ASSERT_EQ(it->maxPosition, 1);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, 91287.5);
   ASSERT_DOUBLE_EQ(it->pnl, 1650.0);
   ASSERT_NEAR(it->pctPnl, 0.018074764, 0.00000001);

   ++it;
   ASSERT_EQ(it->start, Poco::DateTime(2014, 3, 20, 17).timestamp() + 1);
   ASSERT_EQ(it->end, Poco::DateTime(2014, 3, 24, 17).timestamp());
   ASSERT_EQ(it->initialPosition, -1);
   ASSERT_EQ(it->maxPosition, -1);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, -92937.5);
   ASSERT_DOUBLE_EQ(it->pnl, 825.0);
   ASSERT_NEAR(it->pctPnl, 0.008876933, 0.00000001);

   ++it;
   ASSERT_EQ(it->start, Poco::DateTime(2014, 3, 24, 17).timestamp() + 1);
   ASSERT_EQ(it->end, Poco::DateTime(2014, 4, 8, 17).timestamp());
   ASSERT_EQ(it->initialPosition, 1);
   ASSERT_EQ(it->maxPosition, 1);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, 92112.5);
   ASSERT_DOUBLE_EQ(it->pnl, -225.0);
   ASSERT_NEAR(it->pctPnl, -0.002442665, 0.00000001);

   ++it;
   ASSERT_EQ(it->start, Poco::DateTime(2014, 4, 8, 17).timestamp() + 1);
   ASSERT_EQ(it->end, Poco::DateTime(2014, 4, 25, 17).timestamp());
   ASSERT_EQ(it->initialPosition, -1);
   ASSERT_EQ(it->maxPosition, -1);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, -91887.5);
   ASSERT_DOUBLE_EQ(it->pnl, -750.0);
   ASSERT_NEAR(it->pctPnl, -0.008162155, 0.00000001);

   ++it;
   ASSERT_EQ(it->start, Poco::DateTime(2014, 4, 25, 17).timestamp() + 1);
   ASSERT_EQ(it->end, Poco::DateTime(2014, 5, 14, 17).timestamp());
   ASSERT_EQ(it->initialPosition, 1);
   ASSERT_EQ(it->maxPosition, 1);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, 92637.5);
   ASSERT_DOUBLE_EQ(it->pnl, 1262.5);
   ASSERT_NEAR(it->pctPnl, 0.013628390, 0.00000001);

   ++it;
   ASSERT_EQ(it->start, Poco::DateTime(2014, 5, 14, 17).timestamp() + 1);
   ASSERT_EQ(it->end, Poco::DateTime(2014, 5, 28, 17).timestamp());
   ASSERT_EQ(it->initialPosition, -1);
   ASSERT_EQ(it->maxPosition, -1);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, -93900.0);
   ASSERT_DOUBLE_EQ(it->pnl, -1187.5);
   ASSERT_NEAR(it->pctPnl, -0.012646432, 0.00000001);

   ++it;
   ASSERT_EQ(it->start, Poco::DateTime(2014, 5, 28, 17).timestamp() + 1);
   ASSERT_EQ(it->end, Poco::DateTime(2014, 6, 16, 17).timestamp());
   ASSERT_EQ(it->initialPosition, 1);
   ASSERT_EQ(it->maxPosition, 1);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, 95087.5);
   ASSERT_DOUBLE_EQ(it->pnl, 1375.0);
   ASSERT_NEAR(it->pctPnl, 0.014460365, 0.00000001);

   ++it;
   ASSERT_EQ(it->start, Poco::DateTime(2014, 6, 16, 17).timestamp() + 1);
   ASSERT_EQ(it->end, Poco::DateTime(2014, 7, 1, 17).timestamp());
   ASSERT_EQ(it->initialPosition, -1);
   ASSERT_EQ(it->maxPosition, -1);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, -96462.5);
   ASSERT_DOUBLE_EQ(it->pnl, -1825.0);
   ASSERT_NEAR(it->pctPnl, -0.018919269, 0.00000001);

   ++it;
   ASSERT_EQ(it->start, Poco::DateTime(2014, 7, 1, 17).timestamp() + 1);
   ASSERT_EQ(it->end, Poco::DateTime(2014, 7, 16, 17).timestamp());
   ASSERT_EQ(it->initialPosition, 1);
   ASSERT_EQ(it->maxPosition, 1);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, 98287.5);
   ASSERT_DOUBLE_EQ(it->pnl, 450);
   ASSERT_NEAR(it->pctPnl, 0.004578405, 0.00000001);

   ++it;
   ASSERT_EQ(it->start, Poco::DateTime(2014, 7, 16, 17).timestamp() + 1);
   ASSERT_EQ(it->end, Poco::DateTime(2014, 7, 24, 17).timestamp());
   ASSERT_EQ(it->initialPosition, -1);
   ASSERT_EQ(it->maxPosition, -1);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, -98737.5);
   ASSERT_DOUBLE_EQ(it->pnl, -300);
   ASSERT_NEAR(it->pctPnl, -0.003038359, 0.00000001);

   ++it;
   ASSERT_EQ(it->start, Poco::DateTime(2014, 7, 24, 17).timestamp() + 1);
   ASSERT_EQ(it->end, Poco::DateTime(2014, 7, 25, 17).timestamp());
   ASSERT_EQ(it->initialPosition, 1);
   ASSERT_EQ(it->maxPosition, 1);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, 99037.5);
   ASSERT_DOUBLE_EQ(it->pnl, -462.5);
   ASSERT_NEAR(it->pctPnl, -0.004669948, 0.00000001);


   ++it;
   ASSERT_EQ(it->start, Poco::DateTime(2014, 7, 25, 17).timestamp() + 1);
   ASSERT_EQ(it->end, Poco::DateTime(2014, 8, 1, 17).timestamp());
   ASSERT_EQ(it->initialPosition, -1);
   ASSERT_EQ(it->maxPosition, -1);
   ASSERT_EQ(it->numTransacations, 2);
   ASSERT_DOUBLE_EQ(it->maxNotionalCost, -98575.0);
   ASSERT_DOUBLE_EQ(it->pnl, 2650.0);
   ASSERT_NEAR(it->pctPnl, 0.026883084, 0.00000001);
}