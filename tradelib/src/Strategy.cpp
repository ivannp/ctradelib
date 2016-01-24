// std headers
#include <string>

// libraries headers
#include "Poco/Data/RecordSet.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/Statement.h"
#include "Poco/Data/Transaction.h"

// tradelib headers
#include "tradelib/Portfolio.h"
#include "tradelib/Strategy.h"

namespace tradelib
{
   void Strategy::enterLong(const std::string & symbol, long quantity)
   {
      poco_assert(quantity > 0);
      poco_check_ptr(broker_);
      broker_->submitOrder(Order::enterLong(symbol, quantity));
   }

   void Strategy::enterLongLimit(const std::string & symbol, numeric limitPrice, long quantity)
   {
      poco_assert(quantity > 0);
      poco_check_ptr(broker_);
      broker_->submitOrder(Order::enterLongLimit(symbol, quantity, limitPrice));
   }
   void Strategy::enterLongStop(const std::string & symbol, numeric stopPrice, long quantity)
   {
      poco_assert(quantity > 0);
      poco_check_ptr(broker_);
      broker_->submitOrder(Order::enterLongStop(symbol, quantity, stopPrice));
   }
   void Strategy::enterLongStopLimit(const std::string & symbol, numeric stopPrice, numeric limitPrice, long quantity)
   {
      poco_assert(quantity > 0);
      poco_check_ptr(broker_);
      broker_->submitOrder(Order::enterLongStopLimit(symbol, quantity, stopPrice, limitPrice));
   }

   void Strategy::enterLongStopLimit(const std::string & symbol, numeric stopPrice, numeric limitPrice, long quantity, uint barsValidFor)
   {
      poco_assert(quantity > 0);
      poco_check_ptr(broker_);
      Order order = Order::enterLongStopLimit(symbol, quantity, stopPrice, limitPrice);
      order.setExpiration(barsValidFor);
      broker_->submitOrder(order);
   }

   void Strategy::exitLong(const std::string & symbol, long quantity)
   {
      poco_assert(quantity > 0 || quantity == -1);
      poco_check_ptr(broker_);
      broker_->submitOrder(Order::exitLong(symbol, quantity));
   }

   void Strategy::exitLongLimit(const std::string & symbol, numeric limitPrice, long quantity)
   {
      poco_assert(quantity > 0 || quantity == -1);
      poco_check_ptr(broker_);
      broker_->submitOrder(Order::exitLongLimit(symbol, quantity, limitPrice));
   }

   void Strategy::exitLongStop(const std::string & symbol, numeric stopPrice, long quantity)
   {
      poco_assert(quantity > 0 || quantity == -1);
      poco_check_ptr(broker_);
      broker_->submitOrder(Order::exitLongStop(symbol, quantity, stopPrice));

   }
   void Strategy::exitLongStopLimit(const std::string & symbol, numeric stopPrice, numeric limitPrice, long quantity)
   {
      poco_assert(quantity > 0 || quantity == -1);
      poco_check_ptr(broker_);
      broker_->submitOrder(Order::exitLongStopLimit(symbol, quantity, stopPrice, limitPrice));
   }

   void Strategy::enterShort(const std::string & symbol, long quantity)
   {
      poco_assert(quantity > 0);
      poco_check_ptr(broker_);
      broker_->submitOrder(Order::enterShort(symbol, quantity));
   }

   void Strategy::enterShortLimit(const std::string & symbol, numeric limitPrice, long quantity)
   {
      poco_assert(quantity > 0);
      poco_check_ptr(broker_);
      broker_->submitOrder(Order::enterShortLimit(symbol, quantity, limitPrice));
   }

   void Strategy::enterShortStop(const std::string & symbol, numeric stopPrice, long quantity)
   {
      poco_assert(quantity > 0);
      poco_check_ptr(broker_);
      broker_->submitOrder(Order::enterShortStop(symbol, quantity, stopPrice));
   }

   void Strategy::enterShortStopLimit(const std::string & symbol, numeric stopPrice, numeric limitPrice, long quantity)
   {
      poco_assert(quantity > 0);
      poco_check_ptr(broker_);
      broker_->submitOrder(Order::enterShortStopLimit(symbol, quantity, stopPrice, limitPrice));
   }

   void Strategy::enterShortStopLimit(const std::string & symbol, numeric stopPrice, numeric limitPrice, long quantity, uint barsValidFor)
   {
      poco_assert(quantity > 0);
      poco_check_ptr(broker_);
      Order order = Order::enterShortStopLimit(symbol, quantity, stopPrice, limitPrice);
      order.setExpiration(barsValidFor);
      broker_->submitOrder(order);
   }

   void Strategy::exitShort(const std::string & symbol, long quantity)
   {
      poco_assert(quantity > 0 || quantity == -1);
      poco_check_ptr(broker_);
      broker_->submitOrder(Order::exitShort(symbol, quantity));
   }

   void Strategy::exitShortLimit(const std::string & symbol, numeric limitPrice, long quantity)
   {
      poco_assert(quantity > 0 || quantity == -1);
      poco_check_ptr(broker_);
      broker_->submitOrder(Order::exitShortLimit(symbol, quantity, limitPrice));
   }

   void Strategy::exitShortStop(const std::string & symbol, numeric stopPrice, long quantity)
   {
      poco_assert(quantity > 0 || quantity == -1);
      poco_check_ptr(broker_);
      broker_->submitOrder(Order::exitShortStop(symbol, quantity, stopPrice));
   }

   void Strategy::exitShortStopLimit(const std::string & symbol, numeric stopPrice, numeric limitPrice, long quantity)
   {
      poco_assert(quantity > 0 || quantity == -1);
      poco_check_ptr(broker_);
      broker_->submitOrder(Order::exitShortStopLimit(symbol, quantity, stopPrice, limitPrice));
   }

   void Strategy::barOpenHandler(const void * sender, const Bar & bar)
   {
      BarHistory * history = barHistories_.lookup(bar.symbol, bar.timespan);
      poco_check_ptr(history);
      onBarOpen(*history, bar);
   }

   void Strategy::barCloseHandler(const void * sender, const Bar & bar)
   {
      BarHistory * history = barHistories_.lookupOrAdd(bar.symbol, bar.timespan);
      history->append(bar);
      onBarClose(*history, bar);
   }

   void Strategy::barClosedHandler(const void * sender, const Bar & bar)
   {
      BarHistory * history = barHistories_.lookup(bar.symbol, bar.timespan);
      poco_check_ptr(history);
      onBarClosed(*history, bar);
   }

   void Strategy::orderNotificationHandler(const void * sender, const OrderNotification & on)
   {
      logExecution(on);
      onOrderNotification(on);
   }

   void Strategy::setupDb(const std::string & dbPath, bool cleanup)
   {
      Poco::Data::Session session("SQLite", dbPath);
      Poco::Data::Statement stmt(session);

      // Create the executions table (for storing Executions objects)
      stmt << "create table if not exists executions (" <<
         "id integer primary key not null, " <<
         "symbol varchar(32) not null, " <<
         "timestamp bigint not null, " <<
         "price real not null, " <<
         "quantity bigint not null)";
      stmt.execute();

      // Create the trade_stats table (for storing TradeStats objects)
      stmt.reset(session);
      stmt << "create table if not exists trade_stats (" <<
         "id integer primary key not null, " <<
         "symbol varchar(32) not null, " <<
         "start bigint not null, " <<
         "end bigint not null, " <<
         "initial_position bigint not null, " <<
         "max_position bigint not null, " <<
         "num_transactions bigint not null, " <<
         "pnl real not null, " <<
         "pct_pnl real not null, " <<
         "tick_pnl real not null, " <<
         "fees real not null)";
      stmt.execute();

      // Create the pnls table (for storing PnL)
      stmt.reset(session);
      stmt << "create table if not exists pnls (" <<
         "id integer primary key not null, " <<
         "symbol varchar(32) not null, " <<
         "timestamp bigint not null, " <<
         "pnl real not null)";
      stmt.execute();

      stmt.reset(session);
      stmt << "create unique index if not exists pnls_unique on pnls (symbol, timestamp)";
      stmt.execute();

      // Crate the trade_summaries table
      stmt.reset(session);
      stmt << "create table if not exists trade_summaries (" <<
         "id integer primary key not null, " <<
         "symbol varchar(32) not null, " <<
         "type varchar(8) not null, "
         "num_trades bigint not null, " <<
         "gross_profits real not null default 0.0, " <<
         "gross_losses real not null default 0.0, " <<
         "profit_factor real not null default 0.0, " <<
         "average_daily_pnl real not null default 0.0, " <<
         "daily_pnl_stddev real not null default 0.0, " <<
         "sharpe_ratio real not null default 0.0, " <<
         "average_trade_pnl real not null default 0.0, " <<
         "trade_pnl_stddev real not null default 0.0, " <<
         "pct_positive real not null default 0.0, " <<
         "pct_negative real not null default 0.0, " <<
         "max_win real not null default 0.0, " <<
         "max_loss real not null default 0.0, " <<
         "average_win real not null default 0.0, " <<
         "average_loss real not null default 0.0, " <<
         "average_win_loss real not null default 0.0, " <<
         "equity_min real not null default 0.0, " <<
         "equity_max real not null default 0.0, " <<
         "max_drawdown real not null default 0.0)";
      stmt.execute();

      stmt.reset(session);
      stmt << "create unique index if not exists trade_summaries_unique on trade_summaries (symbol, type)";
      stmt.execute();

      if (cleanup)
      {
         stmt.reset(session);
         stmt << "delete from executions";
         stmt.execute();

         stmt.reset(session);
         stmt << "delete from trade_stats";
         stmt.execute();

         stmt.reset(session);
         stmt << "delete from pnls";
         stmt.execute();

         stmt.reset(session);
         stmt << "delete from trade_summaries";
         stmt.execute();
      }
   }

   void Strategy::setDb(const std::string & dbPath, bool setup)
   {
      if (setup) Strategy::setupDb(dbPath);
      dbPath_ = dbPath;
   }

   void Strategy::logExecution(const OrderNotification & orderNotification)
   {
      if (dbPath_.empty()) return;

      Poco::Data::Session session("SQLite", dbPath_);
      Poco::Data::Statement stmt(session);
      stmt << "insert into executions (symbol, timestamp, price, quantity) values (\"" <<
                  orderNotification.order->symbol << "\", " <<
                  orderNotification.execution->timestamp.epochMicroseconds() << ", " <<
                  orderNotification.execution->price << ", " <<
                  orderNotification.execution->quantity << ")";
      stmt.execute();
   }

   void Strategy::logTrades(const std::string & symbol)
   {
      if (dbPath_.empty()) return;

      const BarHistory * history = barHistories_.lookup(symbol, Timespan::DAYS);
      NumericIndexer prices(history->timestamp, history->close);

      const Instrument * instrument = broker_->getInstrument(symbol);
      if (instrument == nullptr) return;

      const Portfolio * portfolio = broker_->getPortfolio("default");
      if (portfolio == nullptr) return;

      NumericIndexer pnl;
      portfolio->getPnl(*instrument, prices, pnl);
      if (pnl.size() == 0) return;

      Poco::Data::Session session("SQLite", dbPath_);
      Poco::Data::Statement stmt(session);

      // Log the PnL
      sint64 timestamp;
      numeric value;
      stmt << "insert or replace into pnls(symbol, timestamp, pnl) values(?, ?, ?)",
         Poco::Data::Keywords::useRef(symbol),
         Poco::Data::Keywords::useRef(timestamp),
         Poco::Data::Keywords::useRef(value);

      session.begin();
      for (sint ii = 0; ii < pnl.size(); ++ii)
      {
         timestamp = pnl.index[ii].epochMicroseconds();
         value = pnl.container[ii];
         stmt.execute();
      }
      session.commit();

      // Log the trade stats
      TradeStatsVector tradeStats;
      TradeSummary all, longs, shorts;
      portfolio->getTradeStats(*instrument, pnl, tradeStats, all, longs, shorts);

      if (tradeStats.size() > 0)
      {
         sint64 start, end;
         long initialPosition, maxPosition, numTransactions;
         numeric pnl, pctPnl, tickPnl, fees;

         stmt.reset(session);
         stmt << "insert into trade_stats (symbol, start, end, initial_position, max_position, num_transactions, pnl, pct_pnl, tick_pnl, fees)"
            << "values (\"" << symbol << "\", ?, ?, ?, ?, ?, ?, ?, ?, ?)",
            Poco::Data::Keywords::useRef(start), Poco::Data::Keywords::useRef(end),
            Poco::Data::Keywords::useRef(initialPosition), Poco::Data::Keywords::useRef(maxPosition), Poco::Data::Keywords::useRef(numTransactions),
            Poco::Data::Keywords::useRef(pnl), Poco::Data::Keywords::useRef(pctPnl), Poco::Data::Keywords::useRef(tickPnl), Poco::Data::Keywords::useRef(fees);

         session.begin();
         for (auto & ts : tradeStats)
         {
            start = ts.start.epochMicroseconds();
            end = ts.end.epochMicroseconds();
            initialPosition = ts.initialPosition;
            maxPosition = ts.maxPosition;
            numTransactions = ts.numTransacations;
            pnl = ts.pnl;
            pctPnl = ts.pctPnl;
            tickPnl = ts.tickPnl;
            fees = ts.fees;
            stmt.execute();
         }
         session.commit();

         session.begin();
         stmt.reset(session);
         if (all.numTrades > 0)
         {
            stmt << "insert into trade_summaries (symbol, type, num_trades, gross_profits, gross_losses, "
               << "profit_factor, average_daily_pnl, daily_pnl_stddev, sharpe_ratio, average_trade_pnl, "
               << "trade_pnl_stddev, pct_positive, pct_negative, max_win, max_loss, average_win, average_loss, "
               << "average_win_loss, equity_min, equity_max, max_drawdown) values (\"" << symbol << "\", \"All\", "
               << all.numTrades << ", " << all.grossProfits << ", " << all.grossLosses << ", " << all.profitFactor
               << ", " << all.averageDailyPnl << ", " << all.dailyPnlStdDev << ", " << all.sharpeRatio << ", "
               << all.averageTradePnl << ", " << all.tradePnlStdDev << ", " << all.pctPositive << ", "
               << all.pctNegative << ", " << all.maxWin << ", " << all.maxLoss << ", " << all.averageWin << ", "
               << all.averageLoss << ", " << all.averageWinLoss << ", " << all.equityMin << ", " << all.equityMax
               << ", " << all.maxDrawdown << ")";
         }
         else
         {
            stmt << "insert into trade_summaries (symbol, type, num_trades) values (\"" << symbol << "\", \"All\", 0)";
         }
         stmt.execute();

         stmt.reset(session);
         if (longs.numTrades > 0)
         {
            stmt << "insert into trade_summaries (symbol, type, num_trades, gross_profits, gross_losses, "
               << "profit_factor, average_daily_pnl, daily_pnl_stddev, sharpe_ratio, average_trade_pnl, "
               << "trade_pnl_stddev, pct_positive, pct_negative, max_win, max_loss, average_win, average_loss, "
               << "average_win_loss, equity_min, equity_max, max_drawdown) values (\"" << symbol << "\", \"Long\", "
               << longs.numTrades << ", " << longs.grossProfits << ", " << longs.grossLosses << ", " << longs.profitFactor
               << ", " << longs.averageDailyPnl << ", " << longs.dailyPnlStdDev << ", " << longs.sharpeRatio << ", "
               << longs.averageTradePnl << ", " << longs.tradePnlStdDev << ", " << longs.pctPositive << ", "
               << longs.pctNegative << ", " << longs.maxWin << ", " << longs.maxLoss << ", " << longs.averageWin << ", "
               << longs.averageLoss << ", " << longs.averageWinLoss << ", " << longs.equityMin << ", " << longs.equityMax
               << ", " << longs.maxDrawdown << ")";
         }
         else
         {
            stmt << "insert into trade_summaries (symbol, type, num_trades) values (\"" << symbol << "\", \"Long\", 0)";
         }
         stmt.execute();

         stmt.reset(session);
         if (shorts.numTrades > 0)
         {
            stmt << "insert into trade_summaries (symbol, type, num_trades, gross_profits, gross_losses, "
               << "profit_factor, average_daily_pnl, daily_pnl_stddev, sharpe_ratio, average_trade_pnl, "
               << "trade_pnl_stddev, pct_positive, pct_negative, max_win, max_loss, average_win, average_loss, "
               << "average_win_loss, equity_min, equity_max, max_drawdown) values (\"" << symbol << "\", \"Short\", "
               << shorts.numTrades << ", " << shorts.grossProfits << ", " << shorts.grossLosses << ", " << shorts.profitFactor
               << ", " << shorts.averageDailyPnl << ", " << shorts.dailyPnlStdDev << ", " << shorts.sharpeRatio << ", "
               << shorts.averageTradePnl << ", " << shorts.tradePnlStdDev << ", " << shorts.pctPositive << ", "
               << shorts.pctNegative << ", " << shorts.maxWin << ", " << shorts.maxLoss << ", " << shorts.averageWin << ", "
               << shorts.averageLoss << ", " << shorts.averageWinLoss << ", " << shorts.equityMin << ", " << shorts.equityMax
               << ", " << shorts.maxDrawdown << ")";
         }
         else
         {
            stmt << "insert into trade_summaries (symbol, type, num_trades) values (\"" << symbol << "\", \"Short\", 0)";
         }
         stmt.execute();
         session.commit();
      }
   }
}