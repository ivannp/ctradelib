#ifndef ORDER_H
#define ORDER_H

// std headers
#include <string>

// libraries headers
#include "Poco/Exception.h"

// tradelib headers
#include "tradelib/Bar.h"
#include "tradelib/Execution.h"
#include "tradelib/Tick.h"
#include "tradelib/Types.h"

namespace tradelib
{
   class Order
   {
   public:
      // Use the position quantity when processing this order
      static const long POSITION_QUANTITY = -1;

      std::string symbol;
      long quantity;
      numeric limitPrice;
      numeric stopPrice;
      numeric fillPrice;
      std::string signal;

      static Order enterLong(const std::string s, long q) { return Order(s, q, NAN, NAN, Type::ENTER_LONG); }
      static Order enterLongLimit(const std::string s, long q, numeric lp) { return Order(s, q, lp, NAN, Type::ENTER_LONG_LIMIT); }
      static Order enterLongStop(const std::string s, long q, numeric sp) { return Order(s, q, NAN, sp, Type::ENTER_LONG_STOP); }
      static Order enterLongStopLimit(const std::string s, long q, numeric sp, numeric lp) { return Order(s, q, sp, lp, Type::ENTER_LONG_STOP_LIMIT); }

      static Order enterShort(const std::string s, long q) { return Order(s, q, NAN, NAN, Type::ENTER_SHORT); }
      static Order enterShortLimit(const std::string s, long q, numeric lp) { return Order(s, q, lp, NAN, Type::ENTER_SHORT_LIMIT); }
      static Order enterShortStop(const std::string s, long q, numeric sp) { return Order(s, q, NAN, sp, Type::ENTER_SHORT_STOP); }
      static Order enterShortStopLimit(const std::string s, long q, numeric sp, numeric lp) { return Order(s, q, sp, lp, Type::ENTER_SHORT_STOP_LIMIT); }

      static Order exitLong(const std::string s, long q) { return Order(s, q, NAN, NAN, Type::EXIT_LONG); }
      static Order exitLongLimit(const std::string s, long q, numeric lp) { return Order(s, q, lp, NAN, Type::EXIT_LONG_LIMIT); }
      static Order exitLongStop(const std::string s, long q, numeric sp) { return Order(s, q, NAN, sp, Type::EXIT_LONG_STOP); }
      static Order exitLongStopLimit(const std::string s, long q, numeric sp, numeric lp) { return Order(s, q, sp, lp, Type::EXIT_LONG_STOP_LIMIT); }

      static Order exitShort(const std::string s, long q) { return Order(s, q, NAN, NAN, Type::EXIT_SHORT); }
      static Order exitShortLimit(const std::string s, long q, numeric lp) { return Order(s, q, lp, NAN, Type::EXIT_SHORT_LIMIT); }
      static Order exitShortStop(const std::string s, long q, numeric sp) { return Order(s, q, NAN, sp, Type::EXIT_SHORT_STOP); }
      static Order exitShortStopLimit(const std::string s, long q, numeric sp, numeric lp) { return Order(s, q, sp, lp, Type::EXIT_SHORT_STOP_LIMIT); }

      Order()
         : quantity(LONG_MIN), barsValidFor_(-1)
      {}

      void activate() { state_ = State::ACTIVE; }
      void fill() { state_ = State::FILLED; }
      void cancel() { state_ = State::CANCELLED; }

      bool isActive() const { return state_ == State::ACTIVE; }
      bool isFilled() const { return state_ == State::FILLED; }
      bool isCancelled() const { return state_ == State::CANCELLED; }

      bool isStopped() const { return flags_ & STOP_WAS_TRIGGERED; }
      void makeStopped() { flags_ |= STOP_WAS_TRIGGERED; }

      bool isBuy() const;

      bool isLongEntry() const;
      bool isLongExit() const;
      bool isShortEntry() const;
      bool isShortExit() const;

      bool isSell() const { return !isBuy(); }

      bool isEntry() const { return (type_ & Type::ENTER) != 0; }
      bool isExit() const { return (type_ & Type::EXIT) != 0; }

      bool tryFill(
               const Tick & tick,
               sint position,
               bool executeOnLimitOrStop,
               numeric & fillPrice,
               long & filledQuantity,
               long & transactionQuantity,
               long & newPosition);

      /**
      * @brief Make the order valid for numBars including the bar on
      *        which it is submitted
      *
      * @param numBars The number of bars. The maximum value to use is:
      *        std::numeric_limits<sint>::max()
      */
      void setExpiration(uint numBars);

      /**
       * @brief Update the state for orders which are set to expire after 
       *        a certain number of bars. Must be called at the bar end.
       *
       * @param bar The bar that ended
       */
      void updateState(const Bar & bar);

   protected:
      enum Type : uint
      {
         NONE = 0,

         ENTER = 0x0001,
         EXIT  = 0x0002,
         LONG  = 0x0004,
         SHORT = 0x0008,
         STOP  = 0x0010,
         LIMIT = 0x0020,

         // Only combinations from here onwards

         // ENTER_LONG = ENTER | LONG,
         ENTER_LONG = 0x0005,

         // ENTER_LONG_STOP = ENTER | LONG | STOP,
         ENTER_LONG_STOP = 0x0015,

         // ENTER_LONG_LIMIT = ENTER | LONG | LIMIT,
         ENTER_LONG_LIMIT = 0x0025,

         // ENTER_LONG_STOP_LIMIT = ENTER | LONG | STOP | LIMIT,
         ENTER_LONG_STOP_LIMIT = 0x0035,

         // EXIT_LONG = EXIT | LONG,
         EXIT_LONG = 0x0006,

         // EXIT_LONG_STOP = EXIT | LONG | STOP,
         EXIT_LONG_STOP = 0x0016,

         // EXIT_LONG_LIMIT = EXIT | LONG | LIMIT,
         EXIT_LONG_LIMIT = 0x0026,

         // EXIT_LONG_STOP_LIMIT = EXIT | LONG | STOP | LIMIT,
         EXIT_LONG_STOP_LIMIT = 0x0036,

         // ENTER_SHORT = ENTER | SHORT,
         ENTER_SHORT = 0x0009,

         // ENTER_SHORT_STOP = ENTER | SHORT | STOP,
         ENTER_SHORT_STOP = 0x0019,

         // ENTER_SHORT_LIMIT = ENTER | SHORT | LIMIT,
         ENTER_SHORT_LIMIT = 0x0029,

         // ENTER_SHORT_STOP_LIMIT = ENTER | SHORT | STOP | LIMIT,
         ENTER_SHORT_STOP_LIMIT = 0x0039,

         // EXIT_SHORT = EXIT | SHORT,
         EXIT_SHORT = 0x000a,

         // EXIT_SHORT_STOP = EXIT | SHORT | STOP,
         EXIT_SHORT_STOP = 0x001a,

         // EXIT_SHORT_LIMIT = EXIT | SHORT | LIMIT,
         EXIT_SHORT_LIMIT = 0x002a,

         // EXIT_SHORT_STOP_LIMIT = EXIT | SHORT | STOP | LIMIT
         EXIT_SHORT_STOP_LIMIT = 0x003a
      };

      enum class State : uint
      {
         ACTIVE,
         CANCELLED,
         FILLED
      };

      enum Flags : uint
      {
         // Stop limit which is already a limit order
         STOP_WAS_TRIGGERED = 0x0001
      };

      Order(const std::string & s, long q, numeric sp, numeric lp, Order::Type t)
         : symbol(s), quantity(q), stopPrice(sp), limitPrice(lp), fillPrice(NAN), type_(t), state_(State::ACTIVE), flags_(0), barsValidFor_(-1)
      {}

      long computeFilledQuantity(long position) const
      {
         long result;
         poco_assert_dbg(this->quantity > 0 || this->quantity == POSITION_QUANTITY);
         if (this->quantity > 0)
         {
            result = std::min(this->quantity, std::abs(position));
         }
         else if (this->quantity == POSITION_QUANTITY)
         {
            result = std::abs(position);
         }
         return result;
      }

      uint type_;
      State state_;
      uint flags_;

      sint barsValidFor_;
      Timestamp lastBar_;
   };

   // The object used to notify for order executions and other events
   class OrderNotification
   {
   public:
      const Order * order;
      const Execution * execution;

      OrderNotification(const Order * o, const Execution * e)
         : order(o), execution(e)
      {}

      OrderNotification()
         : order(nullptr), execution(nullptr)
      {}
   };
}

#endif // ORDER_H