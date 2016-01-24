#ifndef DATA_FEED_H
#define DATA_FEED_H

// std headers
#include <map>
#include <string>
#include <vector>
#include <unordered_map>

// libraries headers
#include "Poco/BasicEvent.h"
#include "Poco/JSON/Object.h"

// tradelib headers
#include "tradelib/BarFileReader.h"
#include "tradelib/Instrument.h"
#include "tradelib/Types.h"

namespace tradelib
{
   /**
    * @class DataFeed
    *
    * @brief The abstract base class for a data feed
    *
    * The use-case scenario is:
    *
    *    1. construct - empty, so that we can easy migrate to DynamicFactory if necessary
    *    2. configure - configures the DataFeed using a single string (config file?) as input
    *    3. subscribe a few times
    *    4. attach the observer (via barEvent) - the bars are fed via an event interface
    *    5. start - kicks off the processing. For historical replays returns when the feed is exhausted.
    *
    * Afterwards "restart" returns the data feed at step 2 - configured without any subscriptions.
    */
   class DataFeed
   {
   public:
      Poco::BasicEvent<const Bar> barEvent;

      virtual void configure(const std::string & config) {}
      virtual void reset() {}

      virtual void subscribe(const std::string & symbol) = 0;
      virtual void unsubscribe(const std::string & symbol) = 0;
      virtual void start() = 0;

      const Instrument * getInstrument(const std::string & symbol)
      {
         InstrumentMap::iterator it = instruments_.find(symbol);
         if (it == instruments_.end()) return nullptr;
         return &it->second;
      }

      const InstrumentVariation * getInstrumentVariation(const std::string & variation, const std::string & symbol)
      {
         InstrumentVariationProviders::iterator variationIt = instrumentVariationProviders_.find(variation);
         if (variationIt == instrumentVariationProviders_.end()) return nullptr; // No mapping to this provider

         InstrumentVariationMap::iterator it = variationIt->second.find(symbol);
         if (it == variationIt->second.end()) return nullptr; // No mapping for this symbol

         return &it->second;
      }

   protected:
      typedef std::unordered_map<std::string, Instrument> InstrumentMap;
      InstrumentMap instruments_;

      // To understand the next structures, consider historical data feed from Pinnacle Data Corp (futures).
      // instruments_ contains the contract information for this provider and the incoming data will be priced
      // as per these specification. This information may differ from another provider, for isntance IB.
      // The following structures are used to provide the necessary information to convert prices from one
      //
      // provider to another. Consider the Euro:
      //    Pinnacle Data: symbol = "FN", bpv = 1,250, tick = 0.01
      //    Interactive Brokers: symbol = "EUR", bpv = 125,000, tick = 0.0001
      //
      // For the Euro, we will have an InstrumentVarition for "FN" containing:
      //    symbol = "EUR", factor = 100, tick = 0.0001
      //
      // This varition will be stored in the map for "ib". 
      typedef std::map<std::string, InstrumentVariation> InstrumentVariationMap;
      typedef std::map<std::string, InstrumentVariationMap> InstrumentVariationProviders;
      InstrumentVariationProviders instrumentVariationProviders_;
   };
}

#endif // DATA_FEED_H