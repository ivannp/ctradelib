#ifndef CSV_READER_H
#define CSV_READER_H

// std headers
#include <istream>
#include <string>
#include <vector>

// libraries headers
#include "Poco/Exception.h"
#include "Poco/String.h"
#include "Poco/StringTokenizer.h"

// tradelib headers
#include "tradelib/Types.h"

namespace tradelib
{
   class CsvReader
   {
   public:
      CsvReader(std::istream * stream)
         : stream_(stream), numColumns_(-1), separators_(","), numLines_(0)
      {}

      CsvReader(std::istream * stream, std::string separators)
         : stream_(stream), numColumns_(-1), separators_(separators), numLines_(0)
      {}

      CsvReader()
         : stream_(nullptr)
      {}

      bool eof() const { return stream_->eof(); }

      bool next(std::vector<std::string> & columns);

   private:
      std::istream * stream_;
      sint numColumns_;
      sint numLines_;
      std::string separators_;
   };

   POCO_DECLARE_EXCEPTION(, CsvException, Poco::Exception)
}

#endif // CSV_READER_H