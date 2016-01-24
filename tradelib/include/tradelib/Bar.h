#ifndef BAR_H
#define BAR_H

// std headers
#include <limits>
#include <map>
#include <string>
#include <unordered_map>

// tradelib headers
#include "tradelib/Types.h"

namespace tradelib {
   class Bar {
   public:
      std::string symbol;
      Timestamp   timestamp;
      numeric     open;
      numeric     high;
      numeric     low;
      numeric     close;
      ulong       volume;
      ulong       interest;

      Timespan    timespan;

      Bar()
         : timestamp(TIMESTAMP_MIN), close(NUMERIC_NAN), timespan(1, 0, 0, 0, 0)
      {}

      Bar(const std::string & s, Timestamp t, numeric op, numeric hi, numeric lo, numeric cl, ulong vol)
         : symbol(s), timestamp(t), open(op), high(hi), low(lo), close(cl), volume(vol), interest(ULONG_MAX), last_(false), timespan(1, 0, 0, 0, 0)
      {}

      Bar(const std::string & s, Timestamp t, numeric op, numeric hi, numeric lo, numeric cl, ulong vol, ulong i)
         : symbol(s), timestamp(t), open(op), high(hi), low(lo), close(cl), volume(vol), interest(i), last_(false), timespan(1, 0, 0, 0, 0)
      {}

      Bar(const std::string & s, Timestamp t, numeric op, numeric hi, numeric lo, numeric cl)
         : symbol(s), timestamp(t), open(op), high(hi), low(lo), close(cl), volume(ULONG_MAX), interest(ULONG_MAX), last_(false)
      {}

      // "true" if this is the last bar in the data feed (historical data feeds for instance)
      bool isLast() const { return last_; }
      void setLast(bool val) { last_ = val; }

   private:
      bool last_;
   };

   class BarHistory
   {
   public:
      TimestampRVector timestamp;
      NumericRVector open;
      NumericRVector high;
      NumericRVector low;
      NumericRVector close;
      LongRVector volume;
      LongRVector interest;

      void append(const Bar & bar)
      {
         timestamp.push_back(bar.timestamp);
         open.push_back(bar.open);
         high.push_back(bar.high);
         low.push_back(bar.low);
         close.push_back(bar.close);
         volume.push_back(bar.volume);
         interest.push_back(bar.interest);
      }
   };

   template<typename T>
   class BarHierarchy
   {
   public:
      T * lookup(const std::string & symbol, Timespan timespan)
      {
         SymbolToTimespanMap::iterator it = symbolToTimespanMap_.find(symbol);
         if (it == symbolToTimespanMap_.end()) return nullptr;

         TimespanMap::iterator timespanIt = it->second.find(timespan);
         if (timespanIt == it->second.end()) return nullptr;
         return &timespanIt->second;
      }

      T * lookupOrAdd(const std::string & symbol, Timespan timespan)
      {
         return &symbolToTimespanMap_[symbol][timespan];
      }

   protected:
      struct TimespanIdentity
      {
         size_t operator()(const Timespan & timespan) { return (size_t)timespan.milliseconds(); }
      };
      typedef std::unordered_map<Timespan, T, TimespanIdentity> TimespanMap;
      typedef std::unordered_map<std::string, TimespanMap> SymbolToTimespanMap;

      SymbolToTimespanMap symbolToTimespanMap_;
   };

   typedef BarHierarchy<BarHistory> BarHistories;
}

#endif // BAR_H