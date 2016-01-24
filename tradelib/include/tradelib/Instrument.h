#ifndef INSTRUMENT_H
#define INSTRUMENT_H

// std headers
#include <string>

// tradelib headers
#include "tradelib/Math.h"
#include "tradelib/Types.h"

namespace tradelib
{
   /**
    * @class Instrument
    *
    * @brief The instrument object in tradelib
    *
    * Some tough choices here - we can use inheritance for instance. The main advantage
    * of the current design is that we can store different types of objects within the
    * same container.
    *
    */
   class Instrument
   {
   public:
      static Instrument newStock(const std::string & symbol) { return Instrument(Type::STOCK, symbol, 0.01, 1); }
      static Instrument newStock(const std::string & symbol, const std::string & name) { return Instrument(Type::STOCK, symbol, 0.01, 1, name); }

      static Instrument newFuture(const std::string & symbol, numeric tick, numeric bpv) { return Instrument(Type::FUTURE, symbol, tick, bpv); }
      static Instrument newFuture(const std::string & symbol, numeric tick, numeric bpv, const std::string & name) { return Instrument(Type::FUTURE, symbol, tick, bpv, name); }

      // Stupid to have a default constructor, but std::map won't work otherwise (operator [] needs it)
      Instrument()
         : type_(Type::NONE)
      {}

      Instrument(const Instrument & other)
         : type_(other.type_), symbol_(other.symbol_), tick_(other.tick_), bpv_(other.bpv_), name_(other.name_)
      {}

      /*
      instrument & operator=(const instrument & other)
      {
         type_ = other.type_;
         symbol_ = other.symbol_;
         tick_size_ = other.tick_size_;
         bpv_ = other.bpv_;
      }
      */

      numeric tick() const { return tick_; }
      numeric bpv() const { return bpv_; }
      const std::string & symbol() const { return symbol_; }
      const std::string & name() const { return name_; }

      bool isFuture() const { return type_ == Type::FUTURE; }
      bool isStock() const { return type_ == Type::STOCK; }

   private:
      enum class Type { NONE, STOCK, FUTURE };

      Instrument(Instrument::Type type, const std::string & symbol, numeric tick, numeric bpv)
         : type_(type), symbol_(symbol), tick_(tick), bpv_(bpv)
      {
      }

      Instrument(Instrument::Type type, const std::string & symbol, numeric tick, numeric bpv, const std::string & name)
         : type_(type), symbol_(symbol), tick_(tick), bpv_(bpv), name_(name)
      {
      }

      Type type_;
      std::string symbol_;
      numeric tick_;
      numeric bpv_;
      std::string name_;
   };

   class InstrumentVariation
   {
   public:
      std::string symbol;
      numeric factor;
      numeric tick;

      InstrumentVariation(const std::string & s, numeric f, numeric t)
         : symbol(s), factor(f), tick(t)
      {}

      numeric price(numeric originalPrice) const { return originalPrice/factor; }
      numeric tickCeil(numeric p) const { return roundAny(price(p), tick, &std::ceil); }
      numeric tickFloor(numeric p) const { return roundAny(price(p), tick, &std::floor); }
   };
}

#endif // INSTRUMENT_H
