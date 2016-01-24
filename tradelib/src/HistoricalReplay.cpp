#include "Poco/Logger.h"

#include "tradelib/HistoricalReplay.h"
#include "tradelib/Order.h"

namespace tradelib
{
   HistoricalReplay::HistoricalReplay()
      : dataFeed_(nullptr)
   {}

   HistoricalReplay::HistoricalReplay(DataFeed & dataFeed)
      : dataFeed_(&dataFeed)
   {
      dataFeed_->barEvent += Poco::delegate(this, &HistoricalReplay::barEventHandler);
   }

   HistoricalReplay::~HistoricalReplay()
   {
      dataFeed_->barEvent.clear();
   }

   void HistoricalReplay::start()
   {
      poco_check_ptr(dataFeed_);
      dataFeed_->start();
   }

   void HistoricalReplay::subscribe(const std::string & symbol)
   {
      poco_check_ptr(dataFeed_);
      dataFeed_->subscribe(symbol);
   }

   void HistoricalReplay::unsubscribe(const std::string & symbol)
   {
      poco_check_ptr(dataFeed_);
      dataFeed_->unsubscribe(symbol);
   }

   HistoricalReplay::InstrumentCB & HistoricalReplay::lookupInstrumentCB(const std::string & symbol)
   {
      InstrumentCBMap::iterator it = instrumentCBMap_.find(symbol);
      if (it != instrumentCBMap_.end()) return it->second;
      // Add a control block if one doesn't exist. Adding an order without an existing subscription
      // sounds like a misuse, but throwing an exception because of it seems like an overkill too.
      return instrumentCBMap_.emplace(symbol, InstrumentCB(dataFeed_->getInstrument(symbol))).first->second;
   }

   void HistoricalReplay::submitOrder(const Order & order)
   {
      InstrumentCB & icb = lookupInstrumentCB(order.symbol);
      icb.newOrders.push_back(order);
   }

   const Portfolio * HistoricalReplay::getPortfolio(const std::string & portfolio)
   {
      return &portfolio_;
   }

   void HistoricalReplay::addNewOrders(InstrumentCB & icb)
   {
      std::move(std::begin(icb.newOrders), std::end(icb.newOrders), std::back_inserter(icb.orders));
      icb.newOrders.resize(0);
   }

   void HistoricalReplay::processOrders(InstrumentCB & icb, const Tick & tick, bool executeOnLimitOrStop)
   {
      // Scan all orders and check for a fill against the current tick
      for (auto it = std::begin(icb.orders); it != std::end(icb.orders); ++it)
      {
         numeric fillPrice;
         long filledQuantity;
         long transactionQuantity;
         long newPosition;
         long previousPosition = icb.instrumentPosition.position;
         bool filled = it->tryFill(tick, previousPosition, executeOnLimitOrStop, fillPrice, filledQuantity, transactionQuantity, newPosition);
         if (filled)
         {
            if (filled)
            {
               // Update the position
               icb.instrumentPosition = { newPosition, tick.timestamp };

               // Some previous exit orders may need to be cancelled. For instance, if we
               // just exited a long position, any previous exit orders are cancelled.
               bool removeExits;
               if ((previousPosition > 0 && newPosition <= 0) || (previousPosition < 0 && newPosition >= 0))
               {
                  removeExits = true;
               }
               else
               {
                  removeExits = false;
               }

               // cancel the orders
               if (removeExits)
               {
                  for (auto oo = std::begin(icb.orders); oo != it; ++oo)
                  {
                     // Cancel active, exit orders
                     if (oo->isExit() && oo->isActive()) oo->cancel();
                  }
               }

               // Mark the current order as filled
               it->fill();
               // Add a transaction to the portfolio
               Poco::Logger::root().debug("appending transaction: " + icb.instrument->symbol() + 
                                          ": " + Poco::DateTimeFormatter::format(tick.timestamp, "%Y-%m-%d") + 
                                          ": " + Poco::NumberFormatter::format(transactionQuantity) + 
                                          ", " + Poco::NumberFormatter::format(filledQuantity));
               portfolio_.appendTransaction(*icb.instrument, tick.timestamp, transactionQuantity, fillPrice, 0.0);
               // Add an execution
               icb.executions.emplace_back(tick.timestamp, fillPrice, filledQuantity);
               // Add a notification (posted after the order processing loop finishes)
               icb.orderNotifications.emplace_back(&*it, &icb.executions.back());
            }
         }
      }
   }

   void HistoricalReplay::postOrderNotifications(InstrumentCB & icb)
   {
      for (auto & on : icb.orderNotifications)
      {
         orderNotificationEvent(this, on);
      }
      icb.orderNotifications.resize(0);
   }

   void HistoricalReplay::cleanupOrders(InstrumentCB & icb, const Bar & bar)
   {
      // While improving performance by removing inactive orders from the list,
      // we lose the order history.
      auto it = std::begin(icb.orders);
      while (it != std::end(icb.orders))
      {
         // First expire the order if necessary
         it->updateState(bar);

         if (it->isActive()) ++it;
         else it = icb.orders.erase(it);
      }
   }

   void HistoricalReplay::barEventHandler(const Bar & bar)
   {
      InstrumentCB & icb = lookupInstrumentCB(bar.symbol);

      // 1. All orders are eligible for execution at this point.
      addNewOrders(icb);

      // 2. Process orders at open. At the open the limit and stop orders
      // are executed on the tick (using false for executeOnLimitOrStop).
      Poco::DateTime dt(bar.timestamp);
      dt.assign(dt.year(), dt.month(), dt.day(), 9, 0, 1);
      Poco::Timestamp ts = dt.timestamp();
      processOrders(icb, Tick(bar.symbol, ts, bar.open), false);

      // 3. Send notifications for the executed trades
      postOrderNotifications(icb);

      // 4. Notify for the opening of the bar. We use a bar, not a Tick object,
      // so that the callee can use (symbol, timespan) to identify the bar set
      // this bar belongs to. The callee may use only the open price from the bar.
      Bar openBar = bar;
      openBar.high = openBar.low = openBar.close = NAN;
      openBar.volume = openBar.interest = LONG_MIN;
      barOpenEvent(this, openBar);

      // 5. Pick up any new orders submitted during steps 2. and 4.
      addNewOrders(icb);

      // 6. Process orders at high (assume at 11:00:01)
      dt.assign(dt.year(), dt.month(), dt.day(), 11, 0, 1);
      ts = dt.timestamp();
      processOrders(icb, Tick(bar.symbol, ts, bar.high), true);

      // No new orders are added here. Orders submitted during the *high*
      // processing are not eligible for execution during the *low* processing.

      // 7. Send notifications for the executed trades
      postOrderNotifications(icb);

      // 8. Process orders at low (assume at 13:00:01)
      dt.assign(dt.year(), dt.month(), dt.day(), 13, 0, 1);
      ts = dt.timestamp();
      processOrders(icb, Tick(bar.symbol, ts, bar.low), true);

      // 9. Send notifications for the executed trades
      postOrderNotifications(icb);

      // 10. Publish the bar, but it's not closed yet - this is to accomodate trading
      // where the signal is computed at the close and the trading takes place at the close.
      barCloseEvent(this, bar);

      // 11. Pick up any new orders submitted during the previous two steps. Everything
      // is eligible to be processed at the close.
      addNewOrders(icb);

      // 12. Process orders at close
      dt.assign(dt.year(), dt.month(), dt.day(), 16, 0, 1);
      ts = dt.timestamp();
      processOrders(icb, Tick(bar.symbol, ts, bar.close), false);

      // 13. Send notifications for the executed trades
      postOrderNotifications(icb);

      // 14. The bar is closed
      barClosedEvent(this, bar);

      // 15. Make all orders eligible
      addNewOrders(icb);

      // 16. It's not safe to cleanup the order vectors earlier, since notifications
      // point straight into the order vector. So all order updates (expiration and/or
      // removal from the list) had to be postponed til now.
      cleanupOrders(icb, bar);
   }

   const Broker::InstrumentPosition * HistoricalReplay::getInstrumentPosition(const std::string & symbol)
   {
      InstrumentCBMap::const_iterator it = instrumentCBMap_.find(symbol);
      if (it == instrumentCBMap_.end()) return nullptr;
      return &it->second.instrumentPosition;
   }

   const InstrumentVariation * HistoricalReplay::getInstrumentVariation(const std::string & provider, const std::string & symbol)
   {
      return dataFeed_->getInstrumentVariation(provider, symbol);
   }

   const Instrument * HistoricalReplay::getInstrument(const std::string & symbol)
   {
      return dataFeed_->getInstrument(symbol);
   }

   void HistoricalReplay::reset()
   {
      // Clear all event subscriptions
      barOpenEvent.clear();
      barCloseEvent.clear();
      barClosedEvent.clear();
      orderNotificationEvent.clear();

      // Remove all per instrument runtime data
      instrumentCBMap_.erase(std::begin(instrumentCBMap_), std::end(instrumentCBMap_));

      // Reset the data feed
      dataFeed_->reset();
   }

   void HistoricalReplay::getPositionPnl(const std::string & symbol, numeric price, numeric & realized, numeric & unrealized)
   {
      const Instrument * instrument = getInstrument(symbol);
      // The caller must ensure that there is a position
      poco_check_ptr(instrument);
      portfolio_.getPositionPnl(*instrument, price, realized, unrealized);
   }
}