#ifndef EXECUTION_H
#define EXECUTION_H

// std headers
#include <string>

// tradelib headers
#include "Types.h"

namespace tradelib
{
   class Execution
   {
   public:
      Timestamp timestamp;
      numeric price;
      long quantity;

      Execution(Timestamp t, numeric p, long q)
         : timestamp(t), price(p), quantity(q)
      {}
   };
}

#endif // EXECUTION_H
