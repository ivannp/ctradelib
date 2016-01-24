#ifndef INDICATORS_H
#define INDICATORS_H

// std headers
#include <math.h>

// tradelib headers
#include "tradelib/Types.h"

namespace tradelib
{
   template<uint U, bool Accumulative = true>
   class SMA
   {
   public:
      static const uint length = U;

      NumericRVector values;

      void onValue(const void * sender, const numeric & value)
      {
         if (Accumulative)
         {
            // Add the new value to the running sum
            sum_ += value;

            // The size of the vector after a push_back
            uint newSize = static_cast<uint>(values.size()) + 1;
            if (newSize > U)
            {
               // Convert to the real sender type
               const NumericRVector * data = reinterpret_cast<const NumericRVector *>(sender);
               // Subtract from the sum the element gone out of the window
               sum_ -= (*data)[U];
               // Compute the SMA
               values.push_back(sum_ / U);
            }
            else if (newSize == U)
            {
               // Prety much same as above, except, there is no element gone out of the window
               values.push_back(sum_ / U);
            }
            else
            {
               values.push_back(NAN);
            }
         }
         else
         {
            // The size of the vector after a push_back
            uint newSize = static_cast<uint>(values.size()) + 1;
            if (newSize >= U)
            {
               // Convert to the real sender type
               const NumericRVector * data = reinterpret_cast<const NumericRVector *>(sender);

               numeric sum = 0.0;
               NumericRVector::const_iterator endIt = data->end();
               NumericRVector::const_iterator it = endIt;
               std::advance(it, -(sint)U);
               while (it != endIt)
               {
                  sum += *it++;
               }

               values.push_back(sum / U);
            }
            else
            {
               values.push_back(NAN);
            }
         }
      }

      numeric operator[](sint ii) { return values[ii]; }

      static numeric value(const NumericRVector & data, sint ii = 0)
      {
         poco_assert(ii >= 0);
         if ((ii + U)> data.size()) return NAN;

         numeric sum = 0.0;
         NumericRVector::const_iterator endIt = data.end();
         NumericRVector::const_iterator it = endIt;
         std::advance(it, -(sint)U);
         while (it != endIt)
         {
            sum += *it++;
         }

         return sum/U;
      }

   protected:
      numeric sum_ = 0.0;
   };

   template<uint U, bool Accumulative = false>
   class SmaAndStdDev
   {
   public:
      static const uint length = U;

      NumericRVector sma;
      NumericRVector stdDev;

      void onValue(const void * sender, const numeric & value)
      {
         if (Accumulative)
         {
            // The size of the vector after a push_back
            uint newSize = static_cast<uint>(sma.size()) + 1;
            if (newSize > U)
            {
               numeric oldMean = mean_;
               const NumericRVector * data = reinterpret_cast<const NumericRVector *>(sender);
               mean_ += (value - (*data)[U])/U;
               var_ += (value - mean_ + (*data)[U] - oldMean)*(value - (*data)[U])/(U - 1);

               sma.push_back(mean_);
               stdDev.push_back(sqrt(var_));
            }
            else if (newSize == U)
            {
               numeric newMean = mean_ + (value - mean_)/U;
               numeric newVar = var_ + (value - mean_)*(value - newMean);

               mean_ = newMean;
               var_ = newVar/(U - 1);

               sma.push_back(mean_);
               stdDev.push_back(sqrt(var_));
            }
            else if (newSize > 1)
            {
               numeric newMean = mean_ + (value - mean_)/newSize;
               numeric newVar = var_ + (value - mean_)*(value - newMean);

               mean_ = newMean;
               var_ = newVar;

               sma.push_back(NAN);
               stdDev.push_back(NAN);
            }
            else // newSize == 1
            {
               mean_ = value;
               var_ = 0.0;

               sma.push_back(NAN);
               stdDev.push_back(NAN);
            }

            /*
            // Add the new value to the running sum
            sum_ += value;
            sumSquares_ += value*value;

            // The size of the vector after a push_back
            uint newSize = static_cast<uint>(sma.size()) + 1;
            if (newSize > U)
            {
               // Convert to the real sender type
               const NumericRVector * data = reinterpret_cast<const NumericRVector *>(sender);
               // Subtract from the sum the element gone out of the window
               sum_ -= (*data)[U];
               sumSquares_ -= (*data)[U] * (*data)[U];
               // Compute SMA and StdDev
               sma.push_back(sum_ / U);
               stdDev.push_back(sqrt((sumSquares_ - sum_*sum_/U)/(U - 1)));
            }
            else if (newSize == U)
            {
               // Prety much same as above, except, there is no element gone out of the window
               sma.push_back(sum_ / U);
               stdDev.push_back(sqrt((sumSquares_ - sum_*sum_/U)/(U - 1)));
            }
            else
            {
               sma.push_back(NAN);
               stdDev.push_back(NAN);
            }
            */
         }
         else
         {
            // The size of the vector after a push_back
            uint newSize = static_cast<uint>(sma.size()) + 1;
            if (newSize >= U)
            {
               // Convert to the real sender type
               const NumericRVector * data = reinterpret_cast<const NumericRVector *>(sender);

               numeric sum = 0.0;
               NumericRVector::const_iterator endIt = data->end();
               NumericRVector::const_iterator startIt = endIt;
               std::advance(startIt, -(sint)U);
               for (NumericRVector::const_iterator it = startIt; it != endIt; ++it)
               {
                  sum += *it;
               }

               numeric ss = sum / U;
               sma.push_back(ss);

               sum = 0.0;
               for (NumericRVector::const_iterator it = startIt; it != endIt; ++it)
               {
                  numeric diff = *it - ss;
                  sum += diff*diff;
               }

               stdDev.push_back(std::sqrt(sum / (U - 1)));
            }
            else
            {
               sma.push_back(NAN);
               stdDev.push_back(NAN);
            }
         }
      }

   protected:
      numeric sum_ = 0.0;
      numeric sumSquares_ = 0.0;
      numeric mean_;
      numeric var_;
   };

   class Average
   {
   public:
      Average()
         : size_(0)
      {}

      void add(numeric v)
      {
         ++size_;
         if (size_ > 1) mean_ += (v - mean_) / size_;
         else mean_ = v;
      }

      numeric get() const { return mean_; }
      ulong size() const { return size_; }

   protected:
      numeric mean_;
      ulong size_;
   };

   class AverageAndVariance
   {
   public:
      AverageAndVariance()
         : size_(0), mean_(0.0)
      {}

      void add(numeric v)
      {
         ++size_;
         if (size_ > 1)
         {
            numeric newMean = mean_ + (v - mean_)/size_;
            numeric newVariance = variance_ + (v - mean_)*(v - newMean);

            mean_ = newMean;
            variance_ = newVariance;
         }
         else
         {
            mean_ = v;
            variance_ = 0.0;
         }
      }

      numeric getAverage() const { return mean_; }
      numeric getVariance() const
      {
         switch (size_)
         {
         case 0: return 0.0;
         case 1: return variance_;
         default: return variance_ / (size_ - 1);
         }
      }
      numeric getStdDev() const { return std::sqrt(getVariance()); }
      ulong size() const { return size_; }

   protected:
      numeric mean_;
      numeric variance_;
      ulong size_;
   };
}

#endif // INDICATORS_H