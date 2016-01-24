#ifndef PINNACLE_DATA_FEED_H
#define PINNACLE_DATA_FEED_H

// std headers
#include <map>
#include <string>
#include <vector>

// libraries headers
#include "Poco/Path.h"
#include "Poco/JSON/Object.h"

// tradelib headers
#include "tradelib/DataFeed.h"

namespace tradelib
{
   class PinnacleDataFeed : public DataFeed
   {
   public:
      virtual void configure(const std::string & config);
      virtual void reset();

      virtual void subscribe(const std::string & symbol);
      virtual void unsubscribe(const std::string & symbol);
      virtual void start();

   protected:
      typedef std::vector<BarFileReader> ReaderVector;
      ReaderVector readers_;

      Poco::Dynamic::Var parsedJson_;
      Poco::JSON::Object::Ptr jsonRoot_;

      Poco::Path path_;
      std::string suffix_;
      std::string format_;
   };
}

#endif // PINNACLE_DATA_FEED_H