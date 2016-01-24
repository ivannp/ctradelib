#ifndef STRATEGY_H
#define STRATEGY_H

#include <string>

#include "Poco/Delegate.h"

#include "tradelib/Broker.h"
#include "tradelib/Types.h"

namespace tradelib
{
   class Strategy
   {
   public:
      Strategy()
         : broker_(nullptr)
      {}

      Strategy(Broker * broker)
         : broker_(broker)
      {
         broker_->barClosedEvent += Poco::delegate(this, &Strategy::barClosedHandler);
         broker_->barCloseEvent += Poco::delegate(this, &Strategy::barCloseHandler);
         broker_->barOpenEvent += Poco::delegate(this, &Strategy::barOpenHandler);
         broker_->orderNotificationEvent += Poco::delegate(this, &Strategy::orderNotificationHandler);
      }

      ~Strategy()
      {
         broker_->barClosedEvent.clear();
         broker_->barCloseEvent.clear();
         broker_->barOpenEvent.clear();
         broker_->orderNotificationEvent.clear();
      }

      // Db interface
      static void setupDb(const std::string & dbPath, bool cleanup = true);
      void setDb(const std::string & dbPath, bool setup = false);
      const std::string & getDb() const { return dbPath_; }

   protected:
      // The handlers for the Broker events
      void barOpenHandler(const void * sender, const Bar & bar);
      void barCloseHandler(const void * sender, const Bar & bar);
      void barClosedHandler(const void * sender, const Bar & bar);
      void orderNotificationHandler(const void * sender, const OrderNotification & on);

      // Virtual methods, to be overwritten by strategy implementations:
      virtual void onBarOpen(const BarHistory & history, const Bar & bar) {}
      virtual void onBarClose(const BarHistory & history, const Bar & bar) {}
      virtual void onBarClosed(const BarHistory & history, const Bar & bar) {}
      virtual void onOrderNotification(const OrderNotification & on) {}

      // Order management
      void enterLong(const std::string & symbol, long quantity = 1);
      void enterLongLimit(const std::string & symbol, numeric limitPrice, long quantity = 1);
      void enterLongStop(const std::string & symbol, numeric stopPrice, long quantity = 1);
      void enterLongStopLimit(const std::string & symbol, numeric stopPrice, numeric limitPrice, long quantity = 1);

      void exitLong(const std::string & symbol, long quantity = -1);
      void exitLongLimit(const std::string & symbol, numeric limitPrice, long quantity = -1);
      void exitLongStop(const std::string & symbol, numeric stopPrice, long quantity = -1);
      void exitLongStopLimit(const std::string & symbol, numeric stopPrice, numeric limitPrice, long quantity = -1);

      void enterShort(const std::string & symbol, long quantity = 1);
      void enterShortLimit(const std::string & symbol, numeric limitPrice, long quantity = 1);
      void enterShortStop(const std::string & symbol, numeric stopPrice, long quantity = 1);
      void enterShortStopLimit(const std::string & symbol, numeric stopPrice, numeric limitPrice, long quantity = 1);

      void exitShort(const std::string & symbol, long quantity = -1);
      void exitShortLimit(const std::string & symbol, numeric limitPrice, long quantity = -1);
      void exitShortStop(const std::string & symbol, numeric stopPrice, long quantity = -1);
      void exitShortStopLimit(const std::string & symbol, numeric stopPrice, numeric limitPrice, long quantity = -1);

      void enterLongStopLimit(const std::string & symbol, numeric stopPrice, numeric limitPrice, long quantity, uint barsValidFor);
      void enterShortStopLimit(const std::string & symbol, numeric stopPrice, numeric limitPrice, long quantity, uint barsValidFor);

      // Db interface
      void logExecution(const OrderNotification & on);
      void logTrades(const std::string & symbol);

      Broker * broker_;
      BarHistories barHistories_;
      std::string dbPath_;
   };
}

#endif // STRATEGY_H