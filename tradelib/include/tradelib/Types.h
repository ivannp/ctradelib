#ifndef TYPES_H
#define TYPES_H

// std headers
#include <algorithm>
#include <cstdint>
#include <ctime>
#include <deque>
#include <limits>
#include <stack>
#include <vector>

// libraries headers
#include "Poco/BasicEvent.h"
#include "Poco/Delegate.h"
#include "Poco/Timespan.h"
#include "Poco/Timestamp.h"

// convenience types
typedef int_fast32_t    sint;
typedef uint_fast32_t   uint;
typedef int8_t          sint8;
typedef uint8_t         uint8;
typedef int16_t         sint16;
typedef uint16_t        uint16;
typedef int32_t         sint32;
typedef uint32_t        uint32;
typedef int64_t         sint64;
typedef uint64_t        uint64;

typedef unsigned long   ulong;

// The main type for prices
typedef double numeric;

namespace tradelib
{  
   // The main type for timestamps and timespans
   typedef Poco::Timestamp Timestamp;
   typedef Poco::Timespan Timespan;

   typedef std::vector<numeric> NumericVector;
   typedef std::vector<Timestamp> TimestampVector;

   static const Timestamp TIMESTAMP_MIN(std::numeric_limits<Poco::Int64>::min());
   static const Timestamp TIMESTAMP_MAX(std::numeric_limits<Poco::Int64>::max());

   static const numeric NUMERIC_NAN = std::nan("");
   static const numeric NUMERIC_MIN = std::numeric_limits<numeric>::min();
   static const numeric NUMERIC_MAX = std::numeric_limits<numeric>::max();

   template<typename T>
   class Indexer
   {
   public:
      typedef std::vector<T> container_type;
      typedef typename container_type::value_type value_type;
      typedef typename container_type::size_type size_type;
      typedef typename container_type::reference reference;
      typedef typename container_type::const_reference const_reference;
      typedef typename container_type::pointer pointer;
      typedef typename container_type::const_pointer const_pointer;

      typedef std::vector<Timestamp> index_type;

      index_type index;
      container_type container;

      Indexer() {}

      Indexer(const index_type & i, const container_type & v)
      {
         poco_assert(i.size() == v.size());
         index.resize(0);
         container.resize(0);
         this->index.insert(std::end(this->index), std::begin(i), std::end(i));
         this->container.insert(std::end(this->container), std::begin(v), std::end(v));
      }

      Indexer(index_type && i, container_type && v)
      {
         poco_assert(index.size() == values.size());
         index.resize(0);
         container.resize(0);
         std::move(std::begin(i), std::end(i), std::begin(index));
         std::move(std::begin(v), std::end(v), std::begin(container));
      }

      void push_back(const Timestamp & t, const value_type & v)
      {
         index.push_back(t);
         container.push_back(v);
      }

      void push_back(const Timestamp & t)
      {
         index.push_back(t);
         container.resize(container.size() + 1);
      }

      void push_back(Timestamp && t)
      {
         index.push_back(std::move(t));
         container.resize(container.size() + 1);
      }

      void resize(size_type new_size)
      {
         index.resize(new_size);
         container.resize(new_size);
      }

      size_type size() const
      {
         return index.size();
      }

      void reserve(size_type n)
      {
         index.reserve(n);
         container.reserve(n)
      }

      const_pointer at(Timestamp t)
      {
         index_type::iterator it = std::lower_bound(std::begin(index), std::end(index), t);
         if (it != index.end() && *it == t) return &container[std::distance(std::begin(index), it)];
         else return nullptr;
      }

      // Append a range to the index, resize the container
      template<typename InputIterator>
      void append(InputIterator first, InputIterator last)
      {
         index.insert(index.end(), first, last);
         container.resize(index.size());
      }

      // Append a range to the index, and a range to the container
      template<typename IndexInputIterator, typename InputIterator>
      void append(IndexInputIterator indexFirst, IndexInputIterator indexLast, InputIterator first, InputIterator last)
      {
         index.insert(index.end(), indexFirst, indexLast);
         container.insert(container.end(), first, last);
         poco_assert_dbg(index.size() == container.size());
      }
   };

   typedef Indexer<numeric> NumericIndexer;

   /**
    * @class RVector
    *
    * @brief A vector but "[]" and "at" index in reverse order. Also notifies on append.
    *
    * In trading we want close[0] to be the last close and close[1] - yesterday's. RVector
    * accomplishes that by mapping close[0] to the last element in the vector. No other 
    * changes to std:vector, for instance iterators work the same way.
    *
    * It also provides a subscription mechanism - observers are informed when a new value
    * is appended (push_back only supported).
    *
    */
   template<class T, class A = std::allocator<T>>
   class RVector : public std::vector<T, A>
   {
   public:
      // The observers are not meant to modify the value, that's why it's "const"
      Poco::BasicEvent<const T> valueEvent;

      void push_back(value_type && val)
      {
         vector_type::push_back(std::move(val));
         valueEvent(this, val);
      }

      void push_back(const value_type & val)
      {
         vector_type::push_back(val);
         valueEvent(this, val);
      }

      template<class... V>
      void emplace_back(V&&... val)
      {
         vector_type::emplace_back(std::forward<V>(val)...);
         valueEvent(this, *(end() - 1));
      }

      const_reference operator[](size_type pos) const
      {
         return vector_type::operator[](vector_position(pos));
      }

      reference operator[](size_type pos)
      {
         return vector_type::operator[](vector_position(pos));
      }

      const_reference at(size_type pos) const
      {
         return vector_type::operator[](vector_position(pos));
      }

      reference at(size_type pos)
      {
         return vector_type::operator[](vector_position(pos));
      }

      size_type vector_position(size_type pos) const { return size() - pos - 1; }
      size_type rvector_position(size_type pos) const { return size() - pos - 1; }

   protected:
      typedef std::vector<T, A> vector_type;
   };
 
   typedef RVector<numeric> NumericRVector;
   typedef RVector<Timestamp> TimestampRVector;
   typedef RVector<long> LongRVector;
}

namespace tl = tradelib;

#endif // TYPES_H