#ifndef TICK_H
#define TICK_H

// std headers
#include <string>

// libraries headers

// tradelib headers
#include "Types.h"

namespace tradelib
{
   class Tick
   {
   public:
      std::string symbol;
      Timestamp timestamp;
      numeric price;
      ulong volume;

      Tick(const std::string & s, Timestamp t, numeric p, ulong v)
         : symbol(s), timestamp(t), price(p), volume(v)
      {}

      Tick(const std::string & s, Timestamp t, numeric p)
         : Tick(s, t, p, 0)
      {}
   };
}

#endif // TICK_H
