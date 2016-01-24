#include "tradelib/Order.h"

namespace tradelib
{
   bool Order::isBuy() const
   {
      switch (type_)
      {
      case ENTER_LONG:
      case ENTER_LONG_STOP:
      case ENTER_LONG_LIMIT:
      case ENTER_LONG_STOP_LIMIT:
      case EXIT_SHORT:
      case EXIT_SHORT_STOP:
      case EXIT_SHORT_LIMIT:
      case EXIT_SHORT_STOP_LIMIT:
         return true;

      case EXIT_LONG:
      case EXIT_LONG_STOP:
      case EXIT_LONG_LIMIT:
      case EXIT_LONG_STOP_LIMIT:
      case ENTER_SHORT:
      case ENTER_SHORT_STOP:
      case ENTER_SHORT_LIMIT:
      case ENTER_SHORT_STOP_LIMIT:
         return false;

      default:
         throw Poco::AssertionViolationException("unhandled order type");
         return false;
      }
   }

   bool Order::isLongEntry() const
   {
      switch (type_)
      {
      case ENTER_LONG:
      case ENTER_LONG_STOP:
      case ENTER_LONG_LIMIT:
      case ENTER_LONG_STOP_LIMIT:
         return true;

      default:
         return false;
      }
   }

   bool Order::isLongExit() const
   {
      switch (type_)
      {
      case EXIT_LONG:
      case EXIT_LONG_STOP:
      case EXIT_LONG_LIMIT:
      case EXIT_LONG_STOP_LIMIT:
         return true;

      default:
         return false;
      }
   }

   bool Order::isShortEntry() const
   {
      switch (type_)
      {
      case ENTER_SHORT:
      case ENTER_SHORT_STOP:
      case ENTER_SHORT_LIMIT:
      case ENTER_SHORT_STOP_LIMIT:
         return true;

      default:
         return false;
      }
   }

   bool Order::isShortExit() const
   {
      switch (type_)
      {
      case EXIT_SHORT:
      case EXIT_SHORT_STOP:
      case EXIT_SHORT_LIMIT:
      case EXIT_SHORT_STOP_LIMIT:
         return true;

      default:
         return false;
      }
   }

   bool Order::tryFill(
            const Tick & tick,
            sint position,
            bool executeOnLimitOrStop,
            numeric & fillPrice,
            long & filledQuantity,
            long & transactionQuantity,
            long & newPosition)
   {
      bool filled = false;

      if (isActive())
      {
         switch (this->type_)
         {
            // market orders
         case Type::ENTER_LONG:
            if (position == 0)
            {
               filled = true;
               fillPrice = tick.price;
               poco_assert_dbg(this->quantity > 0);
               transactionQuantity = filledQuantity = this->quantity;
               newPosition = filledQuantity;
            }
            break;

         case Type::ENTER_SHORT:
            if (position == 0)
            {
               filled = true;
               fillPrice = tick.price;
               poco_assert_dbg(this->quantity > 0);
               filledQuantity = this->quantity;
               transactionQuantity = -filledQuantity;
               newPosition = -filledQuantity;
            }
            break;

         case Type::EXIT_LONG:
            if (position > 0)
            {
               filled = true;
               fillPrice = tick.price;
               filledQuantity = computeFilledQuantity(position);
               transactionQuantity = -filledQuantity;
               newPosition = 0;
            }
            break;

         case Type::EXIT_SHORT:
            filled = true;
            fillPrice = tick.price;
            filledQuantity = computeFilledQuantity(position);
            transactionQuantity = filledQuantity;
            newPosition = 0;
            break;

            // limit orders
         case Type::ENTER_LONG_LIMIT:
            if (position == 0)
            {
               if (tick.price <= this->limitPrice)
               {
                  filled = true;
                  fillPrice = executeOnLimitOrStop ? this->limitPrice : tick.price;
                  poco_assert_dbg(this->quantity > 0);
                  filledQuantity = this->quantity;
                  transactionQuantity = filledQuantity;
                  newPosition = this->quantity;
               }
            }
            break;

         case Type::EXIT_SHORT_LIMIT:
            if (position < 0)
            {
               if (tick.price <= this->limitPrice)
               {
                  filled = true;
                  fillPrice = executeOnLimitOrStop ? this->limitPrice : tick.price;
                  filledQuantity = computeFilledQuantity(position);
                  transactionQuantity = filledQuantity;
                  newPosition = 0;
               }
            }
            break;

            // limit orders
         case Type::ENTER_SHORT_LIMIT:
            if (position == 0)
            {
               if (this->limitPrice <= tick.price)
               {
                  filled = true;
                  fillPrice = executeOnLimitOrStop ? this->limitPrice : tick.price;
                  poco_assert_dbg(this->quantity > 0);
                  filledQuantity = this->quantity;
                  transactionQuantity = -filledQuantity;
                  newPosition = -this->quantity;
               }
            }
            break;

         case Type::EXIT_LONG_LIMIT:
            if (position > 0)
            {
               if (this->limitPrice <= tick.price)
               {
                  filled = true;
                  fillPrice = executeOnLimitOrStop ? this->limitPrice : tick.price;
                  filledQuantity = computeFilledQuantity(position);
                  transactionQuantity = -filledQuantity;
                  newPosition = 0;
               }
            }
            break;

            // stop orders
         case Type::ENTER_LONG_STOP:
            if (position == 0)
            {
               if (this->stopPrice <= tick.price)
               {
                  filled = true;
                  fillPrice = executeOnLimitOrStop ? this->stopPrice : tick.price;
                  poco_assert_dbg(this->quantity > 0);
                  filledQuantity = this->quantity;
                  transactionQuantity = -filledQuantity;
                  newPosition = this->quantity;
               }
            }
            break;

         case Type::EXIT_SHORT_STOP:
            if (position < 0)
            {
               if (this->stopPrice <= tick.price)
               {
                  filled = true;
                  fillPrice = executeOnLimitOrStop ? this->stopPrice : tick.price;
                  filledQuantity = computeFilledQuantity(position);
                  transactionQuantity = -filledQuantity;
                  newPosition = 0;
               }
            }
            break;

         case Type::EXIT_LONG_STOP:
            if (position > 0)
            {
               if (this->stopPrice >= tick.price)
               {
                  filled = true;
                  fillPrice = executeOnLimitOrStop ? this->stopPrice : tick.price;
                  filledQuantity = computeFilledQuantity(position);
                  transactionQuantity = -filledQuantity;
                  newPosition = 0;
               }
            }
            break;

         case Type::ENTER_SHORT_STOP:
            if (position == 0)
            {
               if (this->stopPrice >= tick.price)
               {
                  filled = true;
                  fillPrice = executeOnLimitOrStop ? this->stopPrice : tick.price;
                  poco_assert_dbg(this->quantity > 0);
                  filledQuantity = this->quantity;
                  transactionQuantity = -filledQuantity;
                  newPosition = -this->quantity;
               }
            }
            break;

         case Type::ENTER_LONG_STOP_LIMIT:
            if (position == 0)
            {
               if (isStopped())
               {
                  if (this->limitPrice >= tick.price)
                  {
                     filled = true;
                     fillPrice = executeOnLimitOrStop ? this->limitPrice : tick.price;
                     poco_assert_dbg(this->quantity > 0);
                     filledQuantity = this->quantity;
                     transactionQuantity = filledQuantity;
                     newPosition = this->quantity;
                  }
               }
               else if (this->stopPrice <= tick.price)
               {
                  if ((this->limitPrice >= tick.price) || (executeOnLimitOrStop && this->stopPrice <= this->limitPrice))
                  {
                     filled = true;
                     fillPrice = executeOnLimitOrStop ? this->stopPrice : tick.price;
                     poco_assert_dbg(this->quantity > 0);
                     filledQuantity = this->quantity;
                     transactionQuantity = filledQuantity;
                     newPosition = this->quantity;
                  }
                  else
                  {
                     makeStopped();
                  }
               }
            }
            break;

         case Type::EXIT_LONG_STOP_LIMIT:
            if (isStopped())
            {
               if (this->limitPrice <= tick.price)
               {
                  filled = true;
                  fillPrice = executeOnLimitOrStop ? this->limitPrice : tick.price;
                  filledQuantity = computeFilledQuantity(position);
                  transactionQuantity = -filledQuantity;
                  newPosition = 0;
               }
            }
            else if (this->stopPrice >= tick.price)
            {
               if ((this->limitPrice <= tick.price) || (executeOnLimitOrStop && this->limitPrice <= this->stopPrice))
               {
                  filled = true;
                  fillPrice = executeOnLimitOrStop ? this->stopPrice : tick.price;
                  filledQuantity = computeFilledQuantity(position);
                  transactionQuantity = -filledQuantity;
                  newPosition = 0;
               }
               else
               {
                  makeStopped();
               }
            }
            break;

         case Type::ENTER_SHORT_STOP_LIMIT:
            if (position == 0)
            {
               if (isStopped())
               {
                  if (this->limitPrice <= tick.price)
                  {
                     filled = true;
                     fillPrice = executeOnLimitOrStop ? this->limitPrice : tick.price;
                     poco_assert_dbg(this->quantity > 0);
                     filledQuantity = this->quantity;
                     transactionQuantity = -filledQuantity;
                     newPosition = -this->quantity;
                  }
               }
               else if (this->stopPrice >= tick.price)
               {
                  if ((this->limitPrice <= tick.price) || (executeOnLimitOrStop && this->stopPrice >= this->limitPrice))
                  {
                     filled = true;
                     fillPrice = executeOnLimitOrStop ? this->stopPrice : tick.price;
                     poco_assert_dbg(this->quantity > 0);
                     filledQuantity = this->quantity;
                     transactionQuantity = -filledQuantity;
                     newPosition = -this->quantity;
                  }
                  else
                  {
                     makeStopped();
                  }
               }
            }
            break;

         case Type::EXIT_SHORT_STOP_LIMIT:
            if (position < 0)
            {
               if (isStopped())
               {
                  if (this->limitPrice >= tick.price)
                  {
                     filled = true;
                     fillPrice = executeOnLimitOrStop ? this->limitPrice : tick.price;
                     filledQuantity = computeFilledQuantity(position);
                     transactionQuantity = -filledQuantity;
                     newPosition = 0;
                  }
               }
               else if (this->stopPrice <= tick.price)
               {
                  if ((this->limitPrice >= tick.price) || (executeOnLimitOrStop && this->stopPrice <= this->limitPrice))
                  {
                     filled = true;
                     fillPrice = executeOnLimitOrStop ? this->stopPrice : tick.price;
                     filledQuantity = computeFilledQuantity(position);
                     transactionQuantity = -filledQuantity;
                     newPosition = 0;
                  }
                  else
                  {
                     makeStopped();
                  }
               }
            }
            break;
         }
      }

      return filled;
   }

   void Order::updateState(const Bar & bar)
   {
      // Check whether the order requires processing (bar expiration is set and is active)
      if (!isActive() || barsValidFor_ < 0) return;

      poco_assert_dbg(barsValidFor_ > 0 || isCancelled());
      if (bar.timestamp != lastBar_)
      {
         --barsValidFor_;
         if (barsValidFor_ == 0)
         {
            cancel();
         }
         else
         {
            lastBar_ = bar.timestamp;
         }
      }
   }

   /** 
    * @brief Make the order valid for numBars including the bar on which it is submitted.
    *
    * To account for the bar on which the order is submitted (and to be independent of the
    * current bar), "lastBar_" is initialized to TIMESTAMP_MIN. Thus, at the end of that
    * bar, the code below will perform the first decrement and consider for cancelling.
    *  
    * @param numBars The number of bars this order is valid for
    */
   void Order::setExpiration(uint numBars)
   {
      poco_assert_dbg(numBars <= std::numeric_limits<sint>::max());
      barsValidFor_ = (sint)numBars;
      lastBar_ = TIMESTAMP_MIN;
   }
}