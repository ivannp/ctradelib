INCLUDE_DIRECTORIES(include)

ADD_LIBRARY(
   tradelib
   STATIC
   src/CsvReader.cpp
   src/HistoricalReplay.cpp 
   src/Order.cpp
   src/PinnacleDataFeed.cpp
   src/Portfolio.cpp
   src/Strategy.cpp)
