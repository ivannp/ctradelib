#ifndef HISTORICAL_REPLAY_H
#define HISTORICAL_REPLAY_H

// std headers
#include <map>
#include <unordered_map>

// tradelib headers
#include "tradelib/Broker.h"
#include "tradelib/DataFeed.h"
#include "tradelib/Execution.h"
#include "tradelib/Instrument.h"
#include "tradelib/Order.h"
#include "tradelib/Portfolio.h"

namespace tradelib
{
   class HistoricalReplay : public Broker
   {
   public:
      HistoricalReplay();
      HistoricalReplay(DataFeed & dataFeed);

      ~HistoricalReplay();

      // The Broker interface implementation
      virtual void start();
      virtual void subscribe(const std::string & symbol);
      virtual void unsubscribe(const std::string & symbol);
      virtual void submitOrder(const Order & order);
      virtual const InstrumentPosition * getInstrumentPosition(const std::string & symbol);
      virtual const InstrumentVariation * getInstrumentVariation(const std::string & provider, const std::string & symbol);
      virtual const Instrument * getInstrument(const std::string & symbol);
      virtual void reset();

      // Portfolio interface
      virtual const Portfolio * getPortfolio(const std::string & portfolio);
      virtual void getPositionPnl(const std::string & symbol, numeric price, numeric & realized, numeric & unrealized);

   protected:
      typedef std::vector<Order> OrderVector;
      typedef std::vector<OrderNotification> OrderNotificationVector;
      typedef std::vector<Execution> ExecutionVector;

      class InstrumentCB
      {
      public:
         // Pointer to the instrument
         const Instrument * instrument;
         // Position information
         Broker::InstrumentPosition instrumentPosition;
         // The orders
         OrderVector orders;
         // The new orders merged at specific points into the orders list
         OrderVector newOrders;
         // The executions
         ExecutionVector executions;
         // The order notifications for this instrument
         OrderNotificationVector orderNotifications;

         InstrumentCB()
            : instrument(nullptr), instrumentPosition(0, TIMESTAMP_MIN)
         {}

         InstrumentCB(const Instrument * i)
            : instrument(i), instrumentPosition(0, TIMESTAMP_MIN)
         {}
      };

      typedef std::unordered_map<std::string, InstrumentCB> InstrumentCBMap;
      InstrumentCBMap instrumentCBMap_;

      // The data feed object
      DataFeed * dataFeed_;

      // The portfolio
      Portfolio portfolio_;

      InstrumentCB & lookupInstrumentCB(const std::string & symbol);

      void barEventHandler(const Bar & bar);

      void addNewOrders(InstrumentCB & icb);
      void processOrders(InstrumentCB & icb, const Tick & tick, bool executeOnLimitOrStop);
      void postOrderNotifications(InstrumentCB & icb);
      void cleanupOrders(InstrumentCB & icb, const Bar & bar);
   };
}

#endif // HISTORICAL_REPLAY_H