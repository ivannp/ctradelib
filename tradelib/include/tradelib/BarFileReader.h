#ifndef BAR_FILE_READER_H
#define BAR_FILE_READER_H

// std headers
#include <fstream>
#include <memory>
#include <queue>
#include <string>

// libraries headers
#include "Poco/String.h"
#include "Poco/DateTimeParser.h"

// tradelib headers
#include "tradelib/Bar.h"
#include "tradelib/CsvReader.h"

namespace tradelib
{
   class BarFileReader
   {
   public:
      explicit BarFileReader(const std::string & symbol, const std::string & path, const std::string & format)
         : symbol_(symbol), stream_(new std::ifstream(path)), csvReader_(stream_.get()), format_(format)
      {}

      explicit BarFileReader(const std::string & symbol, const std::string & path)
         : symbol_(symbol), stream_(new std::ifstream(path)), csvReader_(stream_.get())
      {}

      explicit BarFileReader(const BarFileReader & other)
         : stream_(other.stream_.release()), csvReader_(other.csvReader_), symbol_(other.symbol_), buffer_(other.buffer_), format_(other.format_)
      {}

      BarFileReader()
      {}

      BarFileReader & operator=(const BarFileReader & other)
      {
         stream_.reset(other.stream_.release());
         csvReader_ = other.csvReader_;
         symbol_ = other.symbol_;
         buffer_ = other.buffer_;
         format_ = other.format_;
         return *this;
      }

      bool next(Bar & bar) { return getBar(bar, true); }
      bool peek(Bar & bar) { return getBar(bar, false); }

      bool eof() const { return buffer_.empty() && csvReader_.eof(); }

      const std::string & symbol() const { return symbol_; }

   protected:
      void readBars()
      {
         std::vector<std::string> columns;
         while (!csvReader_.eof() && buffer_.size() < CACHE_SIZE && csvReader_.next(columns))
         {
            int tzd;
            // date
            tradelib::Timestamp timestamp = format_.length() > 0 ? Poco::DateTimeParser::parse(format_, columns[0], tzd).timestamp() : Poco::DateTimeParser::parse(columns[0], tzd).timestamp();
            numeric op = std::stod(columns[1]);
            numeric hi = std::stod(columns[2]);
            numeric lo = std::stod(columns[3]);
            numeric cl = std::stod(columns[4]);
            ulong vol = (columns.size() > 5) ? std::stol(columns[5]) : 0L;
            ulong interest = (columns.size() > 6) ? std::stol(columns[6]) : 0L;

            buffer_.emplace(symbol_, timestamp, op, hi, lo, cl, vol);
         }
      }

      bool getBar(Bar & bar, bool popIt)
      {
         // Cache some bars
         if (buffer_.size() < 2) readBars();

         // Done when the buffer is empty
         if (buffer_.empty()) return false;

         // Pop a bar
         bar = buffer_.front();
         if (popIt) buffer_.pop();

         // Mark the last bar of the file
         if (buffer_.size() == 0) bar.setLast(true);

         return true;
      }

      static const sint CACHE_SIZE = 16;
      poco_static_assert(CACHE_SIZE > 1);

      mutable std::unique_ptr<std::ifstream> stream_;
      CsvReader csvReader_;
      
      std::string symbol_;
      std::queue<Bar> buffer_;
      std::string format_;
   };
}

#endif // CSV_BAR_READER_H