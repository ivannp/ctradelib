#ifndef MATH_H
#define MATH_H

#include <cmath>

// tradelib headers
#include "tradelib/Types.h"

namespace tradelib
{
   template <typename T>
   inline int sign(T val)
   {
      return (T(0) < val) - (val < T(0));
   }

   template<typename T>
   inline T roundAny(T x, T accuracy = (T)1, T(*func)(T) = std::trunc)
   {
      return func(x / accuracy)*accuracy;
   }

   template<typename T>
   inline T roundCeil(T x, T accuracy = (T)1)
   {
      return roundAny<T>(x, accuracy, &std::ceil);
   }

   template<typename T>
   inline T roundFloor(T x, T accuracy = (T)1)
   {
      return roundAny<T>(x, accuracy, &std::floor);
   }
}

#endif // MATH_H
