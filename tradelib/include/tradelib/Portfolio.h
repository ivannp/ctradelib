#ifndef PORTFOLIO_H
#define PORTFOLIO_H

// std headers
#include <map>
#include <string>
#include <vector>

// libraries headers
#include "Poco/DateTimeFormatter.h"
#include "Poco/Timespan.h"

// tradelib headers
#include "tradelib/Instrument.h"
#include "tradelib/Math.h"
#include "tradelib/Types.h"

namespace tradelib
{
   class TradeStats
   {
   public:
      std::string symbol;

      Timestamp start;
      Timestamp end;

      long initialPosition;
      long maxPosition;
      long numTransacations;

      numeric maxNotionalCost;

      // currency stats: PnL, MAE, MFE, etc
      numeric pnl;

      // percentage stats: PnL, MAE, MFE, etc
      numeric pctPnl;

      // tick stats: PnL, MAE, MFE, etc
      numeric tickPnl;

      numeric fees;
   };

   typedef std::vector<TradeStats> TradeStatsVector;

   class TradeSummary
   {
   public:
      ulong numTrades;
      numeric grossProfits;
      numeric grossLosses;
      numeric profitFactor;

      numeric averageDailyPnl;
      numeric dailyPnlStdDev;
      numeric sharpeRatio;

      numeric averageTradePnl;
      numeric tradePnlStdDev;

      numeric pctPositive;
      numeric pctNegative;

      numeric maxWin;
      numeric maxLoss;
      numeric averageWin;
      numeric averageLoss;
      numeric averageWinLoss;

      numeric equityMin;
      numeric equityMax;
      numeric maxDrawdown;
   };

   class Portfolio
   {
   public:
      Portfolio(const std::string & name)
         : name_(name)
      {
      }

      Portfolio()
         : Portfolio("default")
      {}

      // Append a transaction
      void appendTransaction(const Instrument & instrument, Timestamp t, long quantity, numeric price, numeric fees);
      // Compute the realized and unrealized PnL for a position
      void getPositionPnl(const Instrument & instrument, numeric price, numeric & realized, numeric & unrealized) const;
      // Compute the PnL for set of prices. For instance, given daily prices, computes the daily PnL 
      void getPnl(const Instrument & instrument, const NumericIndexer & prices, NumericIndexer & pnl) const;
      // Add a new instrument to the portfolio
      void addInstrument(const Instrument & instrument);
      // Get the per-trade statistics for an instrument
      void getTradeStats(const Instrument & instrument, TradeStatsVector & tradeStats) const;
      void getTradeStats(const Instrument & instrument, const NumericIndexer & pnl, TradeStatsVector & tradeStats, TradeSummary & all, TradeSummary & longs, TradeSummary & shorts) const;

      const std::string & name() const { return name_; }

      // The next set of functions get the attributes of the last transaction
      long quantity(const std::string & symbol) const { return data_.find(symbol)->second.back().quantity; }
      numeric price(const std::string & symbol) const { return data_.find(symbol)->second.back().price; }
      numeric averageCost(const std::string & symbol) const { return data_.find(symbol)->second.back().averageCost; }
      long positionQuantity(const std::string & symbol) const { return data_.find(symbol)->second.back().positionQuantity; }
      numeric positionAverageCost(const std::string & symbol) const { return data_.find(symbol)->second.back().positionAverageCost; }
      numeric grossPnl(const std::string & symbol) const { return data_.find(symbol)->second.back().grossPnl; }
      numeric netPnl(const std::string & symbol) const { return data_.find(symbol)->second.back().netPnl; }
      numeric fees(const std::string & symbol) const { return data_.find(symbol)->second.back().fees; }
      numeric value(const std::string & symbol) const { return data_.find(symbol)->second.back().value; }

   protected:
      class Transaction
      {
      public:
         Timestamp timestamp;
         long quantity;
         numeric price;
         numeric value;
         numeric averageCost;
         long positionQuantity;
         numeric positionAverageCost;
         numeric grossPnl;
         numeric netPnl;
         numeric fees;

         Transaction(Timestamp t, long q, numeric p, numeric f)
            : timestamp(t), quantity(q), price(p), fees(f),
            value(0.0), averageCost(0.0), positionQuantity(0),
            positionAverageCost(0.0), grossPnl(0.0), netPnl(0.0)
         {}

         Transaction(Timestamp t)
            : timestamp(t), quantity(0), price(0.0), fees(0.0),
            value(0.0), averageCost(0.0), positionQuantity(0),
            positionAverageCost(0.0), grossPnl(0.0), netPnl(0.0)
         {}

         void incrementTimestamp() { timestamp += Poco::Timespan(1); }
      };

      class TransactionCollection
      {
      public:
         typedef std::vector<Transaction> ContainerType;
         typedef ContainerType::iterator IteratorType;

         void append(const Instrument & instrument, Timestamp time, long quantity, numeric price, numeric fees);
         void getPositionPnl(const Instrument & instrument, numeric price, numeric & realized, numeric & unrealized) const;
         void getPnl(const Instrument & instrument, const NumericIndexer & prices, NumericIndexer & pnl) const;
         void getTradeStats(const Instrument & instrument, TradeStatsVector & tradeStats) const;

         const Transaction & back() const { return container_.back(); }
         const Transaction & front() const { return container_.front(); }

      private:
         ContainerType container_;
      };

      std::string name_;
      std::map<std::string, TransactionCollection> data_;

      friend std::ostream & operator<<(std::ostream &, const Transaction &);
   };

   std::ostream & operator<<(std::ostream & os, const Portfolio::Transaction & t);
   std::ostream & operator<<(std::ostream & os, const TradeStats & ts);
}

#endif // PORTFOLIO_H
