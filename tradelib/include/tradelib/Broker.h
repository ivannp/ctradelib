#ifndef BROKER_H
#define BROKER_H

// std headers
#include <functional>
#include <iostream>
#include <list>
#include <vector>

// libraries headers
#include "Poco/BasicEvent.h"

// tradelib headers
#include "tradelib/Bar.h"
#include "tradelib/Instrument.h"
#include "tradelib/Order.h"
#include "tradelib/Portfolio.h"
#include "tradelib/Tick.h"

namespace tradelib
{
   class Broker
   {
   public:
      Poco::BasicEvent<const Bar> barOpenEvent;
      Poco::BasicEvent<const Bar> barCloseEvent;
      Poco::BasicEvent<const Bar> barClosedEvent;
      Poco::BasicEvent<const OrderNotification> orderNotificationEvent;

      class InstrumentPosition
      {
      public:
         long position;
         Timestamp since;

         InstrumentPosition(long p, Timestamp s)
            : position(p), since(s)
         {}
      };

      virtual void start() = 0;
      virtual void subscribe(const std::string & symbol) = 0;
      virtual void unsubscribe(const std::string & symbol) {}
      virtual void submitOrder(const Order & order) = 0;
      virtual const Instrument * getInstrument(const std::string & symbol) = 0;
      virtual const InstrumentPosition * getInstrumentPosition(const std::string & symbol) = 0;
      virtual const InstrumentVariation * getInstrumentVariation(const std::string & provider, const std::string & symbol) { return nullptr; }
      // Resets all runtime data, but leaves the configuration
      virtual void reset() {}

      virtual const Portfolio * getPortfolio(const std::string & portfolio) { return nullptr; }
      virtual void getPositionPnl(const std::string & symbol, numeric price, numeric & realized, numeric & unrealized) = 0;
   };
}

#endif // BROKER_H