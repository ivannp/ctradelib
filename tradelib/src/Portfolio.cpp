// std headers
#include <iterator>

// libraries headers
#include "Poco/Bugcheck.h"
#include "Poco/Timespan.h"

// tradelib headers
#include "tradelib/Indicators.h"
#include "tradelib/Instrument.h"
#include "tradelib/Portfolio.h"

namespace tradelib
{
   void Portfolio::TransactionCollection::append(const Instrument & instrument, Timestamp t, long quantity, numeric price, numeric fees)
   {
      if (container_.size() == 0)
      {
         // Add an all-zeroes transaction as origin
         container_.emplace_back(t - Poco::Timespan(1));
      }

      // Transactions must be added in chronological order!
      poco_assert(t > container_.back().timestamp);

      // Get the previous quantity for this position
      long ppq = container_.back().positionQuantity;

      Transaction transaction(t, quantity, price, fees);

      if (ppq != 0 && ppq != -transaction.quantity && sign(ppq + transaction.quantity) != sign(ppq))
      {
         // Split the transaction into two, first add the zero-ing transaction
         numeric perUnitFee = transaction.fees / abs(transaction.quantity);
         append(instrument, transaction.timestamp, -ppq, transaction.price, perUnitFee*abs(ppq));

         // ajdust the inputs to reflect what's left to transact, increase the
         // date time by a bit to keep the uniqueness in the transaction set
         transaction.incrementTimestamp();
         transaction.quantity += ppq;
         ppq = 0;
         transaction.fees = perUnitFee*std::abs(transaction.quantity);
      }

      // Transaction value, gross of fees
      // transaction.value = transaction.quantity*transaction.price*instrument.bpv() - transaction.fees;
      transaction.value = transaction.quantity*transaction.price*instrument.bpv();

      // Transaction average cost
      transaction.averageCost = transaction.value / (transaction.quantity*instrument.bpv());

      // Calculate the new quantity for this position
      transaction.positionQuantity = ppq + transaction.quantity;

      // Previous position average cost
      numeric ppac = container_.back().positionAverageCost;

      // Calculate position average cost
      if (transaction.positionQuantity == 0) transaction.positionAverageCost = 0.0;
      else if (abs(ppq) > abs(transaction.positionQuantity)) transaction.positionAverageCost = ppac;
      else transaction.positionAverageCost = ((ppq*ppac*instrument.bpv() + transaction.value) / (transaction.positionQuantity*instrument.bpv()));

      // Calculate PnL
      if (abs(ppq) < abs(transaction.positionQuantity) || ppq == 0) transaction.grossPnl = 0.0;
      else transaction.grossPnl = transaction.quantity*instrument.bpv()*(ppac - transaction.averageCost);

      transaction.netPnl = transaction.grossPnl + transaction.fees;

      container_.push_back(transaction);
   }

   /**
    * @brief Computes the PnL for the current position
    *
    * Computes the PnL for the current position and the specified price. Undefined
    * behaviour if a position doesn't exist.
    *
    * A position starts with the first transaction which sets a quantity different than 0.
    *
    * Uses the gross PnL for all transactions part of this trade.
    *
    * @param[in] instrument the instrument
    * @param[in] price the price to compute the PnL
    * @param[out] realized the realized PnL
    * @param[out] unrealized the unrealized PnL
    */
   void Portfolio::TransactionCollection::getPositionPnl(const Instrument & instrument, numeric price, numeric & realized, numeric & unrealized) const
   {
      auto it = container_.rbegin();
      // Must not be called without a position
      poco_assert(it->positionQuantity != 0);
      unrealized = instrument.bpv()*it->positionQuantity*(price - it->positionAverageCost);
      // Add all realized pnl
      realized = 0.0;
      while (it->positionQuantity != 0)
      {
         realized += it->grossPnl;
         ++it;
      }
   }

   void Portfolio::TransactionCollection::getPnl(const Instrument & instrument, const NumericIndexer & prices, NumericIndexer & pnl) const
   {
      pnl.resize(0);

      // Handle the trivial case of no transactions
      if (container_.size() <= 1)
      {
         pnl.append(std::begin(prices.index), std::end(prices.index));
         return;
      }

      // Find the start
      sint currentTransaction = 1;
      sint ii = 0;
      while (ii < prices.size() && prices.index[ii] < container_[currentTransaction].timestamp) ++ii;

      // Set the pnl to 0 from beginning of time to the first transaction
      pnl.append(std::begin(prices.index), std::begin(prices.index) + ii);
      if (ii == prices.size()) return;

      numeric previousPositionValue = 0.0;
      while (ii < prices.size() && currentTransaction < container_.size())
      {
         if (prices.index[ii] == container_[currentTransaction].timestamp)
         {
            // The current time is both in the price list and in the transaction list
            numeric transactionValue = container_[currentTransaction].value;
            numeric positionValue = container_[currentTransaction].positionQuantity*instrument.bpv()*prices.container[ii];
            pnl.push_back(prices.index[ii], positionValue - previousPositionValue - transactionValue);

            ++ii;
            ++currentTransaction;

            previousPositionValue = positionValue;
         }
         else if (prices.index[ii] < container_[currentTransaction].timestamp)
         {
            // Only in the price list - use the previous position
            numeric positionValue = container_[currentTransaction - 1].positionQuantity*instrument.bpv()*prices.container[ii];
            pnl.push_back(prices.index[ii], positionValue - previousPositionValue);

            ++ii;

            previousPositionValue = positionValue;
         }
         else
         {
            if (ii > 0)
            {
               // Only in the transaction list - use the previous price
               numeric positionValue = container_[currentTransaction].positionQuantity*instrument.bpv()*prices.container[ii - 1];
               pnl.push_back(container_[currentTransaction].timestamp, positionValue - previousPositionValue - container_[currentTransaction].value);

               previousPositionValue = positionValue;
            }
            else
            {
               // No "previous" price - no pnl
               pnl.push_back(container_[currentTransaction].timestamp, 0.0);
            }

            ++currentTransaction;
         }
      }

      while (ii < prices.size())
      {
         numeric positionValue = container_[currentTransaction - 1].positionQuantity*instrument.bpv()*prices.container[ii];
         pnl.push_back(prices.index[ii], positionValue - previousPositionValue);

         ++ii;

         previousPositionValue = positionValue;
      }

      /*
      while (ii < time.size())
      {
      numeric transactionValue = 0.0;
      if (next_transaction < container_.size())
      {
      std::cout << time[ii] << " " << container_[next_transaction].time() << std::endl;
      BOOST_ASSERT_MSG(time[ii] <= container_[next_transaction].time(), "all transactions must be present in the price list");
      if (time[ii] == container_[next_transaction].time())
      {
      ++currentTransaction;
      ++next_transaction;
      transactionValue = container_[currentTransaction].value();
      }
      }
      numeric positionValue = container_[currentTransaction].position_quantity()*instrument.bpv()*prices[ii];
      pnl[ii] = positionValue - previousPositionValue - transactionValue;
      previousPositionValue = positionValue;

      // advance to the next date
      ++ii;
      }
      */
   }

   /**
   * @brief Computes statistics for each trade and a summary
   *
   * @param[in] instrument the instrument
   * @param[out] tradeStats the per-trade statistics
   * @param[out] summary the totals
   */
   void Portfolio::TransactionCollection::getTradeStats(const Instrument & instrument, TradeStatsVector & tradeStats) const
   {
      tradeStats.resize(0);
      ContainerType::const_iterator beginIt = container_.begin();
      // Positiont at the first non-zero quantity
      while (true)
      {
         if (beginIt == container_.end()) return; // No meaningful transactions
         if (beginIt->positionQuantity != 0) break; // Found a real transaction
         ++beginIt;
      }
      ContainerType::const_iterator endIt = beginIt;
      for (++endIt; endIt != container_.end() && endIt->positionQuantity != 0; ++endIt)
      {
      }
      if (endIt != container_.end()) ++endIt;

      // [beginIt, endIt) contains all transactions participating in the current trade
      while (true)
      {
         numeric quantity = 0.0;
         numeric positionCostBasis = 0.0;
         
         numeric pnl = 0.0;
         TradeStats ts;

         ContainerType::const_iterator lastIt = std::prev(endIt);
         
         ts.start = beginIt->timestamp;
         ts.end = lastIt->timestamp;
         ts.initialPosition = beginIt->quantity;
         ts.maxPosition = 0;
         ts.numTransacations = 0;
         ts.maxNotionalCost = 0.0;
         ts.fees = 0;

         for (ContainerType::const_iterator it = beginIt; it != endIt; ++it)
         {
            if (it->value != 0) ++ts.numTransacations;

            positionCostBasis += it->value;
            ts.fees += it->fees;

            if (std::abs(it->positionQuantity) > std::abs(ts.maxPosition))
            {
               ts.maxPosition = it->positionQuantity;
               ts.maxNotionalCost = positionCostBasis;
            }
         }

         numeric positionValue = lastIt->positionQuantity*instrument.bpv()*lastIt->price;
         ts.pnl = positionValue - positionCostBasis;
         ts.pctPnl = ts.pnl / std::abs(ts.maxNotionalCost);

         tradeStats.push_back(ts);

         // Advance to the next trade
         if (endIt == container_.end()) break;
         beginIt = endIt;
         for (++endIt; endIt != container_.end() && endIt->positionQuantity != 0; ++endIt)
         {
         }
         if (endIt != container_.end()) ++endIt;
      }
   }

   void Portfolio::addInstrument(const Instrument & instrument)
   {
      poco_assert(data_.find(instrument.symbol()) == data_.end());
      data_.emplace(instrument.symbol(), TransactionCollection());
   }

   void Portfolio::appendTransaction(const Instrument & instrument, Timestamp t, long quantity, numeric price, numeric fees)
   {
      auto it = data_.find(instrument.symbol());
      if (it == data_.end())
      {
         data_.emplace(instrument.symbol(), TransactionCollection()).first->second.append(instrument, t, quantity, price, fees);
      }
      else
      {
         it->second.append(instrument, t, quantity, price, fees);
      }
   }

   void Portfolio::getPositionPnl(const Instrument & instrument, numeric price, numeric & realized, numeric & unrealized) const
   {
      auto it = data_.find(instrument.symbol());
      poco_assert(it != data_.end());
      it->second.getPositionPnl(instrument, price, realized, unrealized);
   }

   void Portfolio::getPnl(const Instrument & instrument, const NumericIndexer & prices, NumericIndexer & pnl) const
   {
      auto it = data_.find(instrument.symbol());
      if (it == data_.end()) return;
      it->second.getPnl(instrument, prices, pnl);
   }

   // The work area (WA) to compute a TradeSummary
   class TradeSummaryWA
   {
   public:
      TradeSummaryWA(const NumericIndexer & pnl)
         : numTrades_(0), grossProfits_(0.0), grossLosses_(0.0), nonZero_(0), positive_(0), negative_(0),
           maxWin_(NUMERIC_MIN), maxLoss_(NUMERIC_MAX), pnl_(pnl), pnlId_(0),
           minEquity_(NUMERIC_MAX), maxEquity_(NUMERIC_MIN), previousEquity_(0.0), maxDrawdown_(NUMERIC_MAX)
      {}

      void update(const TradeStats & ts)
      {
         ++numTrades_;
         if (ts.pnl < 0.0)
         {
            ++nonZero_;
            ++negative_;
            averageLossTrade_.add(ts.pnl);
            grossLosses_ += ts.pnl;
         }
         else if (ts.pnl > 0.0)
         {
            ++nonZero_;
            ++positive_;
            averageWinTrade_.add(ts.pnl);
            grossProfits_ += ts.pnl;
         }

         pnlStats_.add(ts.pnl);

         maxWin_ = std::max(maxWin_, ts.pnl);
         maxLoss_ = std::min(maxLoss_, ts.pnl);

         // Set the PnL to zero until the current trade begins
         while (pnlId_ < pnl_.size() && pnl_.index[pnlId_] < ts.start)
         {
            pnl_.container[pnlId_++] = 0.0;
         }

         // Keep the PnL as it is inside the current trade
         while (pnlId_ < pnl_.size() && pnl_.index[pnlId_] <= ts.end)
         {
            numeric equity = previousEquity_ + pnl_.container[pnlId_];
            maxEquity_ = std::max(maxEquity_, equity);
            minEquity_ = std::min(minEquity_, equity);
            maxDrawdown_ = std::min(maxDrawdown_, equity - maxEquity_);

            if (pnl_.container[pnlId_] != 0.0) dailyPnlStats_.add(pnl_.container[pnlId_]);

            ++pnlId_;
         }
      }

      void summarize(TradeSummary & summary)
      {
         summary.numTrades = numTrades_;

         if (numTrades_ > 0)
         {
            summary.grossLosses = grossLosses_;
            summary.grossProfits = grossProfits_;
            summary.profitFactor = grossLosses_ != 0 ? std::abs(grossProfits_ / grossLosses_) : std::abs(grossProfits_);

            summary.averageTradePnl = pnlStats_.getAverage();
            summary.tradePnlStdDev = pnlStats_.getStdDev();
            summary.pctNegative = numTrades_ > 0 ? (numeric)negative_ / numTrades_*100.0 : 0.0;
            summary.pctPositive = numTrades_ > 0 ? (numeric)positive_ / numTrades_*100.0 : 0.0;

            summary.maxLoss = maxLoss_;
            summary.maxWin = maxWin_;
            summary.averageLoss = averageLossTrade_.get();
            summary.averageWin = averageWinTrade_.get();
            summary.averageWinLoss = summary.averageLoss != 0.0 ? summary.averageWin / -summary.averageLoss : summary.averageWin;

            summary.equityMin = minEquity_;
            summary.equityMax = maxEquity_;
            summary.maxDrawdown = maxDrawdown_;

            summary.averageDailyPnl = dailyPnlStats_.getAverage();
            summary.dailyPnlStdDev = dailyPnlStats_.getStdDev();
            summary.sharpeRatio = summary.averageDailyPnl / summary.dailyPnlStdDev * std::sqrt(252);
         }
      }

   protected:
      ulong numTrades_;

      numeric grossProfits_;
      numeric grossLosses_;

      numeric mean_;
      numeric variance_;

      AverageAndVariance dailyPnlStats_;
      AverageAndVariance pnlStats_;

      ulong nonZero_;
      ulong positive_;
      ulong negative_;

      numeric maxWin_;
      numeric maxLoss_;

      Average averageWinTrade_;
      Average averageLossTrade_;

      NumericIndexer pnl_;
      sint pnlId_;

      numeric previousEquity_;
      numeric minEquity_;
      numeric maxEquity_;
      numeric maxDrawdown_;
   };

   void Portfolio::getTradeStats(const Instrument & instrument, const NumericIndexer & pnl, TradeStatsVector & tradeStats, TradeSummary & all, TradeSummary & longs, TradeSummary & shorts) const
   {
      getTradeStats(instrument, tradeStats);

      TradeSummaryWA shortsWA(pnl);
      TradeSummaryWA longsWA(pnl);
      TradeSummaryWA allWA(pnl);

      sint tradeId = 0;

      for (const auto & ts : tradeStats)
      {
         if (ts.initialPosition > 0)
         {
            allWA.update(ts);
            longsWA.update(ts);
         }
         else if (ts.initialPosition < 0)
         {
            allWA.update(ts);
            shortsWA.update(ts);
         }
      }

      allWA.summarize(all);
      shortsWA.summarize(shorts);
      longsWA.summarize(longs);
   }

   void Portfolio::getTradeStats(const Instrument & instrument, TradeStatsVector & tradeStats) const
   {
      auto it = data_.find(instrument.symbol());
      if (it == data_.end()) return;
      it->second.getTradeStats(instrument, tradeStats);
   }

   std::ostream & operator<<(std::ostream & os, const Portfolio::Transaction & t)
   {
      os << Poco::DateTimeFormatter::format(t.timestamp, "%Y%m%d") << " : " << t.price << " : " << t.value << " : " <<
         t.averageCost << " : " << t.positionQuantity << " : " <<
         t.positionAverageCost << " : " << t.grossPnl << " : " <<
         t.netPnl << " : " << t.fees;
      return os;
   }

   std::ostream & operator<<(std::ostream & os, const TradeStats & ts)
   {
      os << Poco::DateTimeFormatter::format(ts.start, "%Y%m%d") << " - " << Poco::DateTimeFormatter::format(ts.end, "%Y%m%d")
         << " : " << ts.initialPosition << " : " << ts.maxPosition << " : " << ts.numTransacations << " : "
         << ts.maxNotionalCost << " : " << ts.pnl << " : " << ts.pctPnl << " : " << ts.fees;
      return os;
   }
}