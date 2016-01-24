#include "Poco/Exception.h"
#include "Poco/NumberFormatter.h"

#include "tradelib/CsvReader.h"

namespace tradelib
{
   bool CsvReader::next(std::vector<std::string> & columns)
   {
      std::string line;
      if (!std::getline(*stream_, line)) return false;

      Poco::StringTokenizer tokenizer(line, separators_, Poco::StringTokenizer::TOK_TRIM);
      Poco::StringTokenizer::Iterator it = tokenizer.begin();

      if (numColumns_ != -1)
      {
         if (numColumns_ != tokenizer.count())
         {
            throw CsvException("Line " + Poco::NumberFormatter::format(numLines_) + " has " + Poco::NumberFormatter::format(tokenizer.count()) + " columns, previous lines had " + Poco::NumberFormatter::format(numColumns_) + " columns");
         }
      }
      else
      {
         numColumns_ = static_cast<sint>(tokenizer.count());
      }

      columns.resize(numColumns_);
      for (sint ii = 0; ii < numColumns_; ++ii)
      {
         columns[ii] = *it++;
      }

      return true;
   }

   POCO_IMPLEMENT_EXCEPTION(CsvException, Poco::Exception, "Bad CSV Format")
}