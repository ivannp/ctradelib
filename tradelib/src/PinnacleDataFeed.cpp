#include <string>

#include "Poco/Data/RecordSet.h"
#include "Poco/Data/Session.h"
#include "Poco/Data/Statement.h"
#include "Poco/FileStream.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Path.h"

#include "tradelib/PinnacleDataFeed.h"

namespace tradelib
{
   void PinnacleDataFeed::configure(const std::string & config)
   {
      // Load the settings
      Poco::Data::Session session("SQLite", config);
      Poco::Data::Statement kvStmt(session);
      kvStmt << "select key, value from key_value";
      kvStmt.execute();
      Poco::Data::RecordSet kvrs(kvStmt);
      for (bool more = kvrs.moveFirst(); more; more = kvrs.moveNext())
      {
         std::string key = Poco::toLower<std::string>(kvrs[0].convert<std::string>());
         if (key == "directory") path_ = Poco::Path::forDirectory(kvrs[1].convert<std::string>());
         else if (key == "suffix") suffix_ = kvrs[1].convert<std::string>();
         else if (key == "date_format") format_ = kvrs[1].convert<std::string>();
      }

      // Load the instruments
      Poco::Data::Statement instrumentStmt(session);
      instrumentStmt << "select symbol, bpv, tick, comment, exchange from instrument";
      instrumentStmt.execute();
      Poco::Data::RecordSet instrumentRS(instrumentStmt);
      for (bool more = instrumentRS.moveFirst(); more; more = instrumentRS.moveNext())
      {
         std::string symbol = instrumentRS[0].convert<std::string>();
         // The future symbol must be unique
         poco_assert(instruments_.find(symbol) == instruments_.end());
         numeric bpv = instrumentRS[1].convert<numeric>();
         numeric tick = instrumentRS[2].convert<numeric>();
         std::string comment = instrumentRS[3].convert<std::string>();
         std::string exchange = instrumentRS[4].convert<std::string>();
         instruments_.insert(InstrumentMap::value_type(symbol, Instrument::newFuture(symbol, tick, bpv, comment)));
      }

      // Load the instrument variations
      Poco::Data::Statement ivStmt(session);
      ivStmt << "select provider, original_symbol, symbol, factor, tick from instrument_variation";
      ivStmt.execute();
      Poco::Data::RecordSet ivrs(ivStmt);
      for (bool more = ivrs.moveFirst(); more; more = ivrs.moveNext())
      {
         // Provider is case insensitive
         std::string provider = Poco::toLower(ivrs[0].convert<std::string>());
         std::string originalSymbol = ivrs[1].convert<std::string>();
         InstrumentVariation iv(ivrs[2].convert<std::string>(), ivrs[3].convert<numeric>(), ivrs[4].convert<numeric>());

         InstrumentVariationProviders::iterator providerIt = instrumentVariationProviders_.find(provider);
         if (providerIt == instrumentVariationProviders_.end())
         {
            // First time we are seeing this provider - add a map for it
            providerIt = instrumentVariationProviders_.insert(InstrumentVariationProviders::value_type(provider, InstrumentVariationMap())).first;
         }

         // Single symbol mapping per provider - insert the symbol and check whether it was added anew
         poco_assert(providerIt->second.insert(InstrumentVariationMap::value_type(originalSymbol, iv)).second);
      }
      
      /*
      Poco::FileInputStream fis(config);

      Poco::JSON::Parser parser;
      parsedJson_ = parser.parse(fis);
      jsonRoot_ = parsedJson_.extract<Poco::JSON::Object::Ptr>();

      if (jsonRoot_->has("directory")) path_.assign(jsonRoot_->getValue<std::string>("directory"));
      if (jsonRoot_->has("suffix")) suffix_ = jsonRoot_->getValue<std::string>("suffix");
      if (jsonRoot_->has("date_format")) format_ = jsonRoot_->getValue<std::string>("date_format");

      Poco::JSON::Object::Ptr futures = jsonRoot_->getObject("futures");
      std::vector<std::string> symbols;
      futures->getNames(symbols);

      // Load all instruments
      for (auto ss : symbols)
      {
         // The future symbol must be unique
         poco_assert(instruments_.find(ss) == instruments_.end());
         Poco::JSON::Object::Ptr future = futures->getObject(ss);

         numeric tick = (numeric)future->getValue<double>("tick");
         numeric bpv = (numeric)future->getValue<double>("bpv");
         numeric minMove = (numeric)future->getValue<double>("min_move");
         std::string comment = future->getValue<std::string>("comment");
         std::string exchange = future->getValue<std::string>("exchange");
         instruments_.insert(InstrumentMap::value_type(ss, Instrument::newFuture(ss, tick, bpv, comment)));
      }
      */
   }

   void PinnacleDataFeed::subscribe(const std::string & symbol)
   {
      // Check for duplicates
      for (auto & aa : readers_)
      {
         if (symbol == aa.symbol()) return;
      }

      path_.setFileName(symbol + suffix_);
      readers_.emplace_back(symbol, path_.toString(), format_);
   }

   void PinnacleDataFeed::unsubscribe(const std::string & symbol)
   {
      for (auto it = std::begin(readers_); it != std::end(readers_); ++it)
      {
         if (it->symbol() == symbol)
         {
            readers_.erase(it);
            break;
         }
      }
   }

   void PinnacleDataFeed::start()
   {
      Bar bar;

      while (true)
      {
         Timestamp timestamp = TIMESTAMP_MAX;
         sint minIndex = -1;

         for (sint ii = 0; ii < readers_.size(); ++ii)
         {
            if (readers_[ii].peek(bar) && bar.timestamp < timestamp)
            {
               timestamp = bar.timestamp;
               minIndex = ii;
            }
         }

         if (minIndex != -1)
         {
            readers_[minIndex].next(bar);
            // fire the event
            barEvent(bar);
         }
         else
         {
            // The feed is exhausted
            break;
         }
      }
   }

   void PinnacleDataFeed::reset()
   {
      readers_.resize(0);
   }
}